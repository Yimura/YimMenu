#include "backend/looped_command.hpp"
#include "gta/enums.hpp"
#include "hooking/hooking.hpp"
#include "natives.hpp"
#include "util/math.hpp"
#include "util/pools.hpp"
#include "services/friends/friends_service.hpp"
#include "services/player_database/player_database_service.hpp"
#include "util/ped.hpp"

namespace big
{
	bool_command g_aimbot_only_on_player("aimonlyatplayer", "BACKEND_LOOPED_WEAPONS_AIM_ONLY_AT_PLAYER", "BACKEND_LOOPED_WEAPONS_AIM_ONLY_AT_PLAYER_DESC",
	    g.weapons.aimbot.only_on_player);

	bool_command g_aimbot_only_on_enemy("aimonlyatenemy", "BACKEND_LOOPED_WEAPONS_AIM_ONLY_AT_ENEMY", "BACKEND_LOOPED_WEAPONS_AIM_ONLY_AT_ENEMY_DESC",
	    g.weapons.aimbot.only_on_enemy);

	bool_command g_aimbot_only_on_threat("aimonlyatthreats", "BACKEND_LOOPED_WEAPONS_AIM_ONLY_AT_THREATS", "BACKEND_LOOPED_WEAPONS_AIM_ONLY_AT_THREATS_DESC",
	    g.weapons.aimbot.only_on_threats);

	class aimbot : looped_command
	{
		using looped_command::looped_command;

	public:
		static inline CPed* m_target{};

		static inline bool should_aimbot = false;

		static bool is_a_ped_type_we_dont_care_about(const Ped ped_handle)
		{
			const auto ped_type = PED::GET_PED_TYPE(ped_handle);

			auto config_value = g.weapons.aimbot.only_on_ped_type;

			switch (ped_type)
			{
				case ePedType::PED_TYPE_PLAYER_0:
				case ePedType::PED_TYPE_PLAYER_1:
				case ePedType::PED_TYPE_NETWORK_PLAYER:
				case ePedType::PED_TYPE_PLAYER_2:
				case ePedType::PED_TYPE_CIVMALE:
				case ePedType::PED_TYPE_CIVFEMALE:
				case ePedType::PED_TYPE_COP:
				case ePedType::PED_TYPE_GANG_ALBANIAN:
				case ePedType::PED_TYPE_GANG_BIKER_1:
				case ePedType::PED_TYPE_GANG_BIKER_2:
				case ePedType::PED_TYPE_GANG_ITALIAN:
				case ePedType::PED_TYPE_GANG_RUSSIAN:
				case ePedType::PED_TYPE_GANG_RUSSIAN_2:
				case ePedType::PED_TYPE_GANG_IRISH:
				case ePedType::PED_TYPE_GANG_JAMAICAN:
				case ePedType::PED_TYPE_GANG_AFRICAN_AMERICAN:
				case ePedType::PED_TYPE_GANG_KOREAN:
				case ePedType::PED_TYPE_GANG_CHINESE_JAPANESE:
				case ePedType::PED_TYPE_GANG_PUERTO_RICAN:
				case ePedType::PED_TYPE_DEALER:
				case ePedType::PED_TYPE_MEDIC:
				case ePedType::PED_TYPE_FIREMAN:
				case ePedType::PED_TYPE_CRIMINAL:
				case ePedType::PED_TYPE_BUM:
				case ePedType::PED_TYPE_PROSTITUTE:
				case ePedType::PED_TYPE_SPECIAL:
				case ePedType::PED_TYPE_MISSION:
				case ePedType::PED_TYPE_SWAT:
				case ePedType::PED_TYPE_ANIMAL:
				case ePedType::PED_TYPE_ARMY:
				{
					return (config_value & (1LL << ped_type)) == 0;
				}
			}
			return false;
		}

		static uintptr_t get_cam_follow_ped_camera()
		{
			uintptr_t cam_gameplay_director = *g_pointers->m_gta.m_cam_gameplay_director;
			return *reinterpret_cast<uintptr_t*>(cam_gameplay_director + 0x3C0);
		}

		static rage::fvector3 get_camera_position()
		{
			return *reinterpret_cast<rage::fvector3*>(get_cam_follow_ped_camera() + 0x60);
		}

		static rage::fvector3 get_camera_aim_direction()
		{
			uintptr_t cam_gameplay_director = *g_pointers->m_gta.m_cam_gameplay_director;
			uintptr_t cam_follow_ped_camera = get_cam_follow_ped_camera();

			uintptr_t cam_follow_ped_camera_metadata = *reinterpret_cast<uintptr_t*>(cam_follow_ped_camera + 0x10);
			bool is_first_person = *reinterpret_cast<float*>(cam_follow_ped_camera_metadata + 0x30) == 0.0f;
			if (is_first_person)
			{
				return reinterpret_cast<rage::fvector3*>(cam_follow_ped_camera + 0x40)->normalize();
			}
			else
			{
				return reinterpret_cast<rage::fvector3*>(cam_follow_ped_camera + 0x3D0)->normalize();
			}
		}

		static float get_fov(const rage::fvector3& object_position)
		{
			const auto camera_position = get_camera_position();
			auto camera_aim_direction  = get_camera_aim_direction();

			auto direction_to_object = object_position - camera_position;
			direction_to_object = direction_to_object.normalize();

			float dot_product = camera_aim_direction.dot_product(direction_to_object);
			dot_product       = std::max(-1.0f, std::min(1.0f, dot_product));
			float fov         = std::acos(dot_product);

			return fov * 2;
		}

		static void find_best_target(CPed* self_ped, const rage::fvector3& self_pos)
		{
			m_target = nullptr;

			float best_fov      = math::deg_to_rad(g.weapons.aimbot.fov);
			float best_distance = g.weapons.aimbot.distance;

			if (g.weapons.aimbot.use_weapon_range)
			{
				if (auto weapon_manager = g_local_player->m_weapon_manager)
				{
					if (auto weapon_info = weapon_manager->m_weapon_info)
					{
						best_distance = weapon_info->m_weapon_range;
					}
				}
			}

			for (rage::CEntity* ped_ : pools::get_all_peds())
			{
				CPed* ped = (CPed*)ped_;

				if (ped == self_ped)
				{
					continue;
				}

				if (ped->m_health <= 0)
				{
					continue;
				}

				const auto ped_handle = g_pointers->m_gta.m_ptr_to_handle(ped);
				const bool is_not_a_player_and_we_target_only_players = g_aimbot_only_on_player.is_enabled() && !ped->m_player_info;
				const bool are_we_in_the_same_vehicle = self::veh != 0 && self::veh == PED::GET_VEHICLE_PED_IS_IN(ped_handle, TRUE);
				if (is_not_a_player_and_we_target_only_players || are_we_in_the_same_vehicle)
				{
					continue;
				}

				auto weapon_info = g_local_player->m_weapon_manager->m_weapon_info;
				if (PED::GET_PED_CONFIG_FLAG(ped_handle, 9, TRUE) || !g_pointers->m_gta.m_can_do_damage_to_ped(g_local_player, weapon_info, ped)) //Can't do damage to them, skip.
					continue;
				
				if (g.weapons.aimbot.exclude_friends && ped->m_player_info)
				{
					auto rockstar_id = ped->m_player_info->m_net_player_data.m_gamer_handle.m_rockstar_id;
					auto is_friend   = friends_service::is_friend(rockstar_id);
					auto db_player   = g_player_database_service->get_player_by_rockstar_id(rockstar_id);
					auto is_trusted  = db_player && db_player->is_trusted;

					if (is_friend || is_trusted)
						continue;
				}

				if (g_aimbot_only_on_enemy.is_enabled() && ped::is_ped_a_friend(ped_handle, ped))
				{
					continue;
				}

				if (g.weapons.aimbot.only_on_threats && WEAPON::HAS_PED_GOT_WEAPON(ped_handle, 1, 1) == FALSE)
				{
					continue;
				}

				if (is_a_ped_type_we_dont_care_about(ped_handle))
				{
					continue;
				}

				const auto my_head_pos    = self_ped->get_bone_coords(ePedBoneType::HEAD);
				const auto their_head_pos = ped->get_bone_coords((ePedBoneType)g.weapons.aimbot.selected_bone);

				const auto fov             = get_fov(their_head_pos);
				const auto distance_to_ped = self_pos.distance(their_head_pos);
				if (fov < best_fov && distance_to_ped < best_distance)
				{
					constexpr auto los_flags = (ST_OPTION_IGNORE_GLASS | ST_OPTION_IGNORE_NOTHING | ST_OPTION_IGNORE_TRANSPARENT);
					auto shape_test_handle = SHAPETEST::START_EXPENSIVE_SYNCHRONOUS_SHAPE_TEST_LOS_PROBE(my_head_pos.x,
					    my_head_pos.y,
					    my_head_pos.z,
					    their_head_pos.x,
					    their_head_pos.y,
					    their_head_pos.z,
					    ST_INCLUDE_ALL,
					    self::ped,
					    los_flags);
					BOOL did_shapetest_hit{};
					Vector3 dont_care;
					Entity entity_hit{};
					if (SHAPETEST::GET_SHAPE_TEST_RESULT(shape_test_handle, &did_shapetest_hit, &dont_care, &dont_care, &entity_hit))
					{
						if (!((did_shapetest_hit == TRUE && entity_hit == ped_handle) || !did_shapetest_hit))
						{
							continue;
						}
					}
					best_fov      = fov;
					best_distance = distance_to_ped;
					m_target      = ped;
				}
			}
		}

		// Make aimbot works when driving a vehicle.
		static void reset_aim_vectors(uintptr_t camera)
		{
			uintptr_t camera_params = *(uintptr_t*)(camera + 0x10);
			{
				if (g_local_player->m_vehicle)
				{
					if (*(float*)(camera_params + 0x2AC) == -2.0f)
					{
						*(float*)(camera_params + 0x2AC) = 0.0f;
						*(float*)(camera_params + 0x2C0) = 111.0f;
						*(float*)(camera_params + 0x2C4) = 111.0f;
					}
				}
				else
				{
					if (*(float*)(camera_params + 0x130) == 8.0f)
					{
						*(float*)(camera_params + 0x130) = 111.0f; // def 8.0f
						*(float*)(camera_params + 0x134) = 111.0f; // def 10.0f
						*(float*)(camera_params + 0x4CC) = 0.0f;   // def 4.0f

						if (*(float*)(camera_params + 0x49C) == 1.0f)
						{
							*(float*)(camera_params + 0x49C) = 0.0f; // def 1.0f
						}

						*(float*)(camera_params + 0x2AC) = 0.0f; // def -3.0f
						*(float*)(camera_params + 0x2B0) = 0.0f; // def -8.0f
					}
				}
			}
		}

		static void compute_aim_direction_and_set_gameplay_cam(const rage::fvector3& target_bone_position)
		{
			uintptr_t cam_follow_ped_camera = get_cam_follow_ped_camera();

			const auto aim_direction = (target_bone_position - get_camera_position()).normalize();

			reset_aim_vectors(cam_follow_ped_camera);

			*reinterpret_cast<rage::fvector3*>(cam_follow_ped_camera + 0x40)  = aim_direction;
			*reinterpret_cast<rage::fvector3*>(cam_follow_ped_camera + 0x3D0) = aim_direction;
		}

		static inline CPed* last_target_pos_target{};
		static inline rage::fvector3 last_target_pos{};

		static inline rage::fvector3 last_my_pos{};

		static rage::fvector3 get_velocity(CPed* ped)
		{
			if (ped == g_local_player)
			{
				const auto velocity = *ped->get_position() - last_my_pos;
				last_my_pos         = *ped->get_position();

				return velocity;
			}

			if (ped == last_target_pos_target)
			{
				const auto velocity = *ped->get_position() - last_target_pos;
				last_target_pos     = *ped->get_position();

				return velocity;
			}
			else
			{
				last_target_pos_target = ped;
				last_target_pos        = *ped->get_position();

				return {};
			}
		}

		static void adjust_position_for_target_velocity(rage::fvector3& target_position)
		{
			const auto target_velocity = get_velocity(m_target);
			const auto my_velocity     = get_velocity(g_local_player);

			if (target_velocity == rage::fvector3{})
				return;

			target_position += (target_velocity - my_velocity);
		}

		virtual void on_tick() override
		{
			should_aimbot = PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)ControllerInputs::INPUT_AIM);

			if (!should_aimbot)
			{
				return;
			}

			CAM::STOP_SCRIPT_GLOBAL_SHAKING(true);
			CAM::SET_GAMEPLAY_CAM_SHAKE_AMPLITUDE(0);
		}
	};

	aimbot g_aimbot("aimbot", "VIEW_OVERLAY_AIMBOT", "BACKEND_LOOPED_WEAPONS_AIMBOT_DESC", g.weapons.aimbot.enable);

	bool hooks::aimbot_cam_gameplay_director_update(uintptr_t this_)
	{
		const auto res = big::hooking::get_original<hooks::aimbot_cam_gameplay_director_update>()(this_);

		CPed* self_ped;
		rage::fvector3 self_pos;

		rage::fvector3 target_bone_position;

		if (!aimbot::should_aimbot)
		{
			goto exit;
		}

		self_ped = g_local_player;
		if (!self_ped)
		{
			goto exit;
		}

		self_pos = *self_ped->get_position();

		aimbot::find_best_target(self_ped, self_pos);
		g.weapons.aimbot.has_target = aimbot::m_target != nullptr;
		if (!g.weapons.aimbot.has_target)
		{
			goto exit;
		}

		target_bone_position = aimbot::m_target->get_bone_coords((ePedBoneType)g.weapons.aimbot.selected_bone);

		// Take into account the target velocity.
		aimbot::adjust_position_for_target_velocity(target_bone_position);

		aimbot::compute_aim_direction_and_set_gameplay_cam(target_bone_position);

	exit:
		return res;
	}
}
