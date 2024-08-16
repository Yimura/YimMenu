#include "ped.hpp"

namespace big::ped
{
	bool change_player_model(const Hash hash)
	{
		if (entity::request_model(hash))
		{
			self::ped = PLAYER::PLAYER_PED_ID();
			PLAYER::SET_PLAYER_MODEL(self::id, hash);
			script::get_current()->yield();
			STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(hash);
			for (int i = 0; i < 12; i++)
			{
				PED::SET_PED_COMPONENT_VARIATION(self::ped, i, PED::GET_PED_DRAWABLE_VARIATION(self::ped, i), PED::GET_PED_TEXTURE_VARIATION(self::ped, i), PED::GET_PED_PALETTE_VARIATION(self::ped, i));
			}
			return true;
		}
		return false;
	}

	bool steal_outfit(const Ped target)
	{
		Ped ped = self::ped;

		if (ENTITY::GET_ENTITY_MODEL(ped) != ENTITY::GET_ENTITY_MODEL(target))
		{
			return false;
		}
		for (int i = 0; i < 12; i++)
		{
			PED::SET_PED_COMPONENT_VARIATION(ped, i, PED::GET_PED_DRAWABLE_VARIATION(target, i), PED::GET_PED_TEXTURE_VARIATION(target, i), PED::GET_PED_PALETTE_VARIATION(target, i));
		}

		return true;
	}

	void clone_ped(const Ped src, const Ped target)
	{
		PED::CLONE_PED_TO_TARGET(src, target);
		auto src_ptr = g_pointers->m_gta.m_handle_to_ptr(src);
		auto dst_ptr = g_pointers->m_gta.m_handle_to_ptr(target);

		if (src_ptr && dst_ptr)
		{
			for (auto container = src_ptr->m_extension_container; container; container = container->m_next)
			{
				if (container->m_entry && container->m_entry->get_id() == 0xB)
				{
					g_pointers->m_gta.m_set_head_blend_data(reinterpret_cast<CPed*>(dst_ptr),
					    reinterpret_cast<CHeadBlendData*>(container->m_entry));
					break;
				}
			}
		}
	}

	void steal_identity(const Ped target)
	{
		const int max_health     = ENTITY::GET_ENTITY_MAX_HEALTH(self::ped);
		const int current_health = ENTITY::GET_ENTITY_HEALTH(self::ped);
		const int current_armor  = PED::GET_PED_ARMOUR(self::ped);

		if (ENTITY::GET_ENTITY_MODEL(target) != ENTITY::GET_ENTITY_MODEL(self::id))
		{
			PLAYER::SET_PLAYER_MODEL(self::id, ENTITY::GET_ENTITY_MODEL(target));
			script::get_current()->yield();
		}
		clone_ped(target, self::ped);
		ENTITY::SET_ENTITY_MAX_HEALTH(self::ped, max_health);
		ENTITY::SET_ENTITY_HEALTH(self::ped, current_health, 0, 0);
		PED::SET_PED_ARMOUR(self::ped, current_armor);
	}

	void kill_ped(const Ped ped)
	{
		if (entity::take_control_of(ped, 0))
			ENTITY::SET_ENTITY_HEALTH(ped, 0, self::ped, 0);
		else
		{
			auto ptr = g_pointers->m_gta.m_handle_to_ptr(ped);
			if (!ptr)
				return;

			g_pointers->m_gta.m_send_network_damage(g_player_service->get_self()->get_ped(), ptr, ptr->get_position(), 0, true, "weapon_explosion"_J, 10000.0f, 2, 0, (1 << 4), 0, 0, 0, false, false, true, true, nullptr);
		}
	}

	Ped spawn(ePedType pedType, Hash hash, Ped clone, Vector3 location, float heading, bool is_networked)
	{
		if (entity::request_model(hash))
		{
			Ped ped = PED::CREATE_PED(pedType, hash, location.x, location.y, location.z, heading, is_networked, false);

			script::get_current()->yield();

			if (clone)
			{
				clone_ped(clone, ped);
			}

			STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(hash);

			return ped;
		}
		return 0;
	}

	void set_ped_random_component_variation(Ped ped)
	{
		constexpr auto range = [](int lower_bound, int upper_bound) -> int {
			return std::rand() % (upper_bound - lower_bound + 1) + lower_bound;
		};

		outfit::components_t components;

		for (auto& item : components.items)
		{
			int drawable_id_max = PED::GET_NUMBER_OF_PED_DRAWABLE_VARIATIONS(ped, item.id) - 1;
			if (drawable_id_max == -1)
				continue;
			int drawable_id    = range(0, drawable_id_max);
			int texture_id_max = PED::GET_NUMBER_OF_PED_TEXTURE_VARIATIONS(ped, item.id, drawable_id) - 1;
			if (texture_id_max == -1)
				continue;
			int texture_id = range(0, texture_id_max);
			PED::SET_PED_COMPONENT_VARIATION(ped, item.id, drawable_id, texture_id, PED::GET_PED_PALETTE_VARIATION(ped, item.id));
		}
	}

	player_ptr get_player_from_ped(Ped ped)
	{
		for (auto& p : g_player_service->players())
		{
			if (p.second->get_ped())
			{
				if (p.second->get_ped() == g_pointers->m_gta.m_handle_to_ptr(ped))
					return p.second;
			}
		}
		return nullptr;
	}

	bool load_animation_dict(const char* dict)
	{
		if (STREAMING::HAS_ANIM_DICT_LOADED(dict))
			return true;

		for (uint8_t i = 0; !STREAMING::HAS_ANIM_DICT_LOADED(dict) && i < 35; i++)
		{
			STREAMING::REQUEST_ANIM_DICT(dict);
			script::get_current()->yield();
		}

		return STREAMING::HAS_ANIM_DICT_LOADED(dict);
	}

	void ped_play_animation(Ped ped, const std::string_view& animDict, const std::string_view& animName, float speed, float speedMultiplier, int duration, int flag, float playbackRate, bool lockPos, Vector3 pos, Vector3 rot, int ik_flags)
	{
		if (load_animation_dict(animDict.data()))
			if (pos.x == 0 && pos.y == 0 && pos.z == 0)
				TASK::TASK_PLAY_ANIM(ped, animDict.data(), animName.data(), speed, speedMultiplier, duration, flag, playbackRate, lockPos, lockPos, lockPos);
			else
				TASK::TASK_PLAY_ANIM_ADVANCED(ped, animDict.data(), animName.data(), pos.x, pos.y, pos.z, rot.x, rot.y, rot.z, speed, speedMultiplier, duration, flag, playbackRate, lockPos, ik_flags);
	}

	/*
	* Will make the ped enter the vehicle with animation if vehicle is in vicinity
	* Param movespeed: 1 = walk, 2 = run, 3 = sprint
	*/
	void ped_enter_vehicle_animated(Ped ped, Vehicle veh, eVehicleSeats seat, int movespeed)
	{
		if (entity::take_control_of(ped))
		{
			if (ENTITY::DOES_ENTITY_EXIST(veh))
			{
				if (math::distance_between_vectors(ENTITY::GET_ENTITY_COORDS(ped, 0), ENTITY::GET_ENTITY_COORDS(veh, 0)) < 15.f)
					TASK::TASK_ENTER_VEHICLE(ped, veh, 10000, (int)seat, movespeed, 8, NULL, 0);
				else
					PED::SET_PED_INTO_VEHICLE(ped, veh, (int)seat);
			}
		}
	}

	bool is_ped_a_friend(Ped ped, CPed* ped_ptr)
	{
		if (PED::GET_PED_CONFIG_FLAG(ped, 38, TRUE) == TRUE)
			return false;

		if (PED::IS_PED_IN_COMBAT(ped, self::ped))
			return false;

		return !g_pointers->m_gta.m_is_ped_enemies_with(ped_ptr->m_ped_intelligence, g_local_player, true, false, false);
	}
}