#include "backend/looped_command.hpp"
#include "gta/enums.hpp"
#include "natives.hpp"
#include "util/entity.hpp"
#include "services/friends/friends_service.hpp"
#include "services/player_database/player_database_service.hpp"
#include "util/ped.hpp"

namespace big
{
	class triggerbot : looped_command
	{
		using looped_command::looped_command;

		virtual void on_tick() override
		{
			Entity crosshair_catch;
			if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)ControllerInputs::INPUT_AIM))
			{
				if (g.weapons.aimbot.enable && g.weapons.aimbot.has_target)
				{
					PED::SET_PED_RESET_FLAG(self::ped, 65, TRUE);
				}
				else if (entity::raycast(&crosshair_catch))
				{
					if (ENTITY::IS_ENTITY_A_PED(crosshair_catch) && !ENTITY::IS_ENTITY_DEAD(crosshair_catch, 0))
					{
						Ped ped = (Ped)crosshair_catch;
						CPed* ped_ptr = reinterpret_cast<CPed*>(g_pointers->m_gta.m_handle_to_ptr(ped));

						if (ped_ptr == nullptr)
							return;

						const bool trace_hit_non_player = g.weapons.aimbot.only_on_player && !ped_ptr->m_player_info;
						const bool are_we_in_the_same_vehicle = self::veh != 0 && self::veh == PED::GET_VEHICLE_PED_IS_IN(ped, TRUE);
						if (trace_hit_non_player || are_we_in_the_same_vehicle)
						{
							return;
						}

						auto weapon_info = g_local_player->m_weapon_manager->m_weapon_info;
						if (PED::GET_PED_CONFIG_FLAG(ped, 9, TRUE) || !g_pointers->m_gta.m_can_do_damage_to_ped(g_local_player, weapon_info, ped_ptr)) //Can't do damage to them, skip.
							return;

						if (g.weapons.aimbot.exclude_friends && ped_ptr->m_player_info)
						{
							auto rockstar_id = ped_ptr->m_player_info->m_net_player_data.m_gamer_handle.m_rockstar_id;
							auto is_friend   = friends_service::is_friend(rockstar_id);
							auto db_player   = g_player_database_service->get_player_by_rockstar_id(rockstar_id);
							auto is_trusted  = db_player && db_player->is_trusted;

							if (is_friend || is_trusted)
								return;
						}

						if (g.weapons.aimbot.only_on_enemy && ped::is_ped_a_friend(ped, ped_ptr))
						{
							return;
						}

						if (g.weapons.aimbot.only_on_threats && WEAPON::HAS_PED_GOT_WEAPON(ped, 1, 1) == FALSE)
						{
							return;
						}

						bool is_a_ped_type_we_dont_care_about;
						const auto ped_type = PED::GET_PED_TYPE(ped);

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
								is_a_ped_type_we_dont_care_about = (g.weapons.aimbot.only_on_ped_type & (1LL << ped_type)) == 0;
							}
							default: is_a_ped_type_we_dont_care_about = false;
						}

						if (is_a_ped_type_we_dont_care_about)
						{
							return;
						}

						//Vector3 coords = ENTITY::GET_ENTITY_BONE_POSTION(crosshair_catch, 0x796E); //SKEL_Head (This will fix the edge case of peds in cars)
						//PED::SET_PED_SHOOTS_AT_COORD(self::ped, coords.x, coords.y, coords.z, true);
						PED::SET_PED_RESET_FLAG(self::ped, 65, TRUE);
					}
				}
			}
		}
	};

	triggerbot g_triggerbot("triggerbot", "VIEW_OVERLAY_TRIGGERBOT", "BACKEND_LOOPED_WEAPONS_TRIGGERBOT_DESC", g.weapons.triggerbot);
}
