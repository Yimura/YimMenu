#include "entity.hpp"
#include "gta/joaat.hpp"
#include "gta_util.hpp"
#include "math.hpp"
#include "natives.hpp"
#include "pools.hpp"
#include "script.hpp"
#include "services/players/player_service.hpp"
#include "packet.hpp"
#include "gta/net_object_mgr.hpp"

#include <entities/CDynamicEntity.hpp>

namespace
{
	int get_next_token_value(int prev_token)
	{
		for (int i = 0; i < 0x1F; i++)
		{
			if ((i << 27) - (prev_token << 27) > 0)
				return i;
		}

		return 0;
	}
}

namespace big::entity
{
	void cage_ped(Ped ped)
	{
		Hash hash = "prop_gold_cont_01"_J;

		Vector3 location = ENTITY::GET_ENTITY_COORDS(ped, true);
		OBJECT::CREATE_OBJECT(hash, location.x, location.y, location.z - 1.f, true, false, false);
	}

	void clean_ped(Ped ped)
	{
		PED::CLEAR_PED_BLOOD_DAMAGE(ped);
		PED::CLEAR_PED_WETNESS(ped);
		PED::CLEAR_PED_ENV_DIRT(ped);
		PED::RESET_PED_VISIBLE_DAMAGE(ped);

		// https://forum.cfx.re/t/information-needed-to-clear-visible-player-damage-scars-etc/283216
		// https://docs.fivem.net/natives/?_0x397C38AA7B4A5F83
		for (int i = 0; i <= 5; ++i)
			PED::CLEAR_PED_DAMAGE_DECAL_BY_ZONE(ped, i, "ALL");
	}

	void delete_entity(Entity& ent, bool force)
	{
		if (!ENTITY::DOES_ENTITY_EXIST(ent))
		{
			ent = NULL;
			return;
		}

		if (auto ptr = g_pointers->m_gta.m_handle_to_ptr(ent))
		{
			if (ptr->m_net_object)
			{
				force_remove_network_entity(ptr, true);
				return;
			}
		}

		if (ENTITY::IS_ENTITY_A_VEHICLE(ent))
		{
			for (auto obj : pools::get_all_props())
			{
				auto object = g_pointers->m_gta.m_ptr_to_handle(obj);
				if (!object)
					break;

				if (!ENTITY::IS_ENTITY_ATTACHED_TO_ENTITY(ent, object))
					continue;

				ENTITY::DELETE_ENTITY(&object);
			}

			for (auto veh : pools::get_all_vehicles())
			{
				auto vehicle = g_pointers->m_gta.m_ptr_to_handle(veh);
				if (!vehicle)
					break;

				if (ent == vehicle || !ENTITY::IS_ENTITY_ATTACHED_TO_ENTITY(ent, vehicle))
					continue;

				ENTITY::DELETE_ENTITY(&vehicle);
			}
		}

		ENTITY::DETACH_ENTITY(ent, 1, 1);
		ENTITY::SET_ENTITY_COORDS_NO_OFFSET(ent, 7000.f, 7000.f, 15.f, 0, 0, 0);
		if (!ENTITY::IS_ENTITY_A_MISSION_ENTITY(ent))
		{
			ENTITY::SET_ENTITY_AS_MISSION_ENTITY(ent, true, true);
		}
		ENTITY::DELETE_ENTITY(&ent);
	}

	bool raycast(Entity* ent)
	{
		BOOL hit;
		Vector3 dontCare;

		Vector3 camCoords = CAM::GET_GAMEPLAY_CAM_COORD();
		Vector3 rot       = CAM::GET_GAMEPLAY_CAM_ROT(2);
		Vector3 dir       = math::rotation_to_direction(rot);
		Vector3 farCoords;

		farCoords.x = camCoords.x + dir.x * 1000;
		farCoords.y = camCoords.y + dir.y * 1000;
		farCoords.z = camCoords.z + dir.z * 1000;

		auto shape_test  = SHAPETEST::START_EXPENSIVE_SYNCHRONOUS_SHAPE_TEST_LOS_PROBE(camCoords.x,
            camCoords.y,
            camCoords.z,
            farCoords.x,
            farCoords.y,
            farCoords.z,
            -1,
            0,
            7);
		auto test_result = SHAPETEST::GET_SHAPE_TEST_RESULT(shape_test, &hit, &dontCare, &dontCare, ent);

		return (test_result == 2 && hit == TRUE);
	}

	bool raycast(Vector3* endcoor)
	{
		Entity ent;
		BOOL hit;
		Vector3 surfaceNormal;

		Vector3 camCoords = CAM::GET_GAMEPLAY_CAM_COORD();
		Vector3 dir       = math::rotation_to_direction(CAM::GET_GAMEPLAY_CAM_ROT(2));
		Vector3 farCoords;

		farCoords.x = camCoords.x + dir.x * 1000;
		farCoords.y = camCoords.y + dir.y * 1000;
		farCoords.z = camCoords.z + dir.z * 1000;

		int ray = SHAPETEST::START_EXPENSIVE_SYNCHRONOUS_SHAPE_TEST_LOS_PROBE(camCoords.x,
		    camCoords.y,
		    camCoords.z,
		    farCoords.x,
		    farCoords.y,
		    farCoords.z,
		    -1,
		    0,
		    7);
		SHAPETEST::GET_SHAPE_TEST_RESULT(ray, &hit, endcoor, &surfaceNormal, &ent);

		return (bool)hit;
	}

	bool network_has_control_of_entity(rage::netObject* net_object)
	{
		return !net_object || !net_object->m_is_remote && (net_object->m_wants_to_be_owner == -1);
	}

	bool take_control_of(Entity ent, int timeout)
	{
		if (!*g_pointers->m_gta.m_is_session_started)
			return true;

		auto hnd = g_pointers->m_gta.m_handle_to_ptr(ent);

		if (!hnd || !hnd->m_net_object)
			return false;

		if (hnd->m_entity_type != 3 && hnd->m_entity_type != 4 && hnd->m_entity_type != 5)
			return false;

		for (int i = 0; i <= timeout; ++i)
		{
			if (network_has_control_of_entity(hnd->m_net_object))
				return true;

			g_pointers->m_gta.m_request_control(hnd->m_net_object);

			if (timeout != 0)
				script::get_current()->yield();
		}

		return false;
	}

	std::vector<Entity> get_entities(bool vehicles, bool peds, bool props, bool include_self_veh)
	{
		std::vector<Entity> target_entities;

		if (vehicles)
		{
			for (auto vehicle : pools::get_all_vehicles())
			{
				if (!vehicle || (!include_self_veh && vehicle == gta_util::get_local_vehicle()))
					continue;

				target_entities.push_back(g_pointers->m_gta.m_ptr_to_handle(vehicle));
			}
		}

		if (peds)
		{
			for (auto ped : pools::get_all_peds())
			{
				if (!ped || ped == g_local_player)
					continue;

				target_entities.push_back(g_pointers->m_gta.m_ptr_to_handle(ped));
			}
		}

		if (props)
		{
			for (auto prop : pools::get_all_props())
			{
				if (!prop)
					continue;

				target_entities.push_back(g_pointers->m_gta.m_ptr_to_handle(prop));
			}
		}
		return target_entities;
	}

	bool load_ground_at_3dcoord(Vector3& location)
	{
		constexpr float max_ground_check = 1000.f;
		constexpr int max_attempts       = 300;
		float ground_z                   = location.z;
		int current_attempts             = 0;
		bool found_ground;
		float height;

		do
		{
			found_ground = MISC::GET_GROUND_Z_FOR_3D_COORD(location.x, location.y, max_ground_check, &ground_z, FALSE, FALSE);
			STREAMING::REQUEST_COLLISION_AT_COORD(location.x, location.y, location.z);

			if (current_attempts % 10 == 0)
			{
				location.z += 25.f;
			}

			++current_attempts;

			script::get_current()->yield();
		} while (!found_ground && current_attempts < max_attempts);

		if (!found_ground)
		{
			return false;
		}

		if (WATER::GET_WATER_HEIGHT(location.x, location.y, location.z, &height))
		{
			location.z = height;
		}
		else
		{
			location.z = ground_z + 1.f;
		}

		return true;
	}

	bool request_model(rage::joaat_t hash)
	{
		if (STREAMING::HAS_MODEL_LOADED(hash))
		{
			return true;
		}

		bool has_loaded;

		if (STREAMING::IS_MODEL_VALID(hash) && STREAMING::IS_MODEL_IN_CDIMAGE(hash))
		{
			do
			{
				has_loaded = STREAMING::HAS_MODEL_LOADED(hash);
				if (has_loaded)
					break;

				STREAMING::REQUEST_MODEL(hash);

				script::get_current()->yield();
			} while (!has_loaded);

			return true;
		}

		return false;
	}

	double distance_to_middle_of_screen(const rage::fvector2& screen_pos)
	{
		double cumulative_distance{};

		if (screen_pos.x > 0.5)
			cumulative_distance += screen_pos.x - 0.5;
		else
			cumulative_distance += 0.5 - screen_pos.x;

		if (screen_pos.y > 0.5)
			cumulative_distance += screen_pos.y - 0.5;
		else
			cumulative_distance += 0.5 - screen_pos.y;

		return cumulative_distance;
	}

	Entity get_entity_closest_to_middle_of_screen(rage::fwEntity** pointer, std::vector<Entity> ignore_entities, bool include_veh, bool include_ped, bool include_prop, bool include_players)
	{
		Entity closest_entity{};
		rage::fwEntity* closest_entity_ptr = nullptr;
		float distance                     = 1;

		auto ignored_entity = [=](Entity handle) -> bool {
			if (handle == self::ped)
				return true;

			for (auto ent : ignore_entities)
			{
				if (handle == ent)
					return true;
			}

			return false;
		};

		auto update_closest_entity = [&](Entity handle, rage::fwEntity* entity_ptr) {
			Vector3 pos = ENTITY::GET_ENTITY_COORDS(handle, 1);
			rage::fvector2 screenpos;
			HUD::GET_HUD_SCREEN_POSITION_FROM_WORLD_POSITION(pos.x, pos.y, pos.z, &screenpos.x, &screenpos.y);

			if (distance_to_middle_of_screen(screenpos) < distance && ENTITY::HAS_ENTITY_CLEAR_LOS_TO_ENTITY(self::ped, handle, 17) && !ignored_entity(handle))
			{
				closest_entity     = handle;
				closest_entity_ptr = entity_ptr;
				distance           = distance_to_middle_of_screen(screenpos);
			}
		};

		auto include_pool = [&](auto& pool) {
			for (const auto ptr : pool())
				if (ptr)
					update_closest_entity(g_pointers->m_gta.m_ptr_to_handle(ptr), ptr);
		};

		if (include_veh)
			include_pool(pools::get_all_vehicles);

		if (include_ped)
			include_pool(pools::get_all_peds);

		if (include_prop)
			include_pool(pools::get_all_props);

		if (include_players)
		{
			for (auto player : g_player_service->players() | std::ranges::views::values)
			{
				if (player->get_ped())
				{
					Ped handle = g_pointers->m_gta.m_ptr_to_handle(player->get_ped());
					update_closest_entity(handle, player->get_ped());
				}
			}
		}

		if (pointer)
			*pointer = closest_entity_ptr;

		return closest_entity;
	}

	void force_remove_network_entity(rage::CDynamicEntity* entity, bool delete_locally)
	{
		if (!entity->m_net_object)
			return;

		force_remove_network_entity(entity->m_net_object->m_object_id, entity->m_net_object->m_ownership_token, delete_locally);
	}

	void force_remove_network_entity(std::uint16_t net_id, int ownership_token, bool delete_locally)
	{
		char buf[0x200]{};
		rage::datBitBuffer remove_buf(buf, sizeof(buf));
		int msgs_written = 0;

		if (ownership_token != -1)
		{
			remove_buf.Write<std::uint16_t>(net_id, 13);
			remove_buf.Write<int>(get_next_token_value(ownership_token), 5);
			msgs_written++;
		}
		else
		{
			// try all tokens if we don't know it
			for (int i = 0; i < 0x1F; i++)
			{
				remove_buf.Write<std::uint16_t>(net_id, 13);
				remove_buf.Write<int>(i, 5);
				msgs_written++;
			}
		}

		packet pack;
		pack.write_message(rage::eNetMessage::MsgPackedReliables);
		pack.write<int>(4, 4); // remove
		pack.write<int>(msgs_written, 5);
		pack.write<int>(remove_buf.GetPosition(), 13);
		pack.m_buffer.WriteArray(&buf, remove_buf.GetPosition());

		for (auto& player : g_player_service->players())
		{
			if (player.second->get_net_game_player())
			{
				if (!player.second->get_ped() || player.second->get_ped()->m_net_object->m_object_id != net_id) // would crash the player otherwise
				{
					pack.send(player.second->get_net_game_player()->m_msg_id);
				}
			}
		}

		if (delete_locally)
			if (auto object = (*g_pointers->m_gta.m_network_object_mgr)->find_object_by_id(net_id, true))
				(*g_pointers->m_gta.m_network_object_mgr)->UnregisterNetworkObject(object, 8, true, true);
	}
}
