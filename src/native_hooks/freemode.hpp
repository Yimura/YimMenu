#pragma once
#include "script_function.hpp"
#include "services/vehicle/vehicle_control_service.hpp"

namespace big
{
	namespace freemode
	{
		void GET_NUMBER_OF_THREADS_RUNNING_THE_SCRIPT_WITH_THIS_HASH(rage::scrNativeCallContext* src)
		{
			const auto hash = src->get_arg<rage::joaat_t>(0);

			if (hash == "director_mode"_J || hash == "main"_J)
			{
				src->set_return_value(0);
				return;
			}

			src->set_return_value(SCRIPT::GET_NUMBER_OF_THREADS_RUNNING_THE_SCRIPT_WITH_THIS_HASH(hash));
		}

		void STAT_GET_INT(rage::scrNativeCallContext* src)
		{
			if (g_vehicle_control_service.m_driver_performing_task && (src->get_arg<Hash>(0) == "MP0_PERSONAL_VEHICLE_ACCESS"_J || src->get_arg<Hash>(0) == "MP1_PERSONAL_VEHICLE_ACCESS"_J))
			{
				src->set_return_value<int>(0);
				return;
			}

			src->set_return_value(STATS::STAT_GET_INT(src->get_arg<Hash>(0), src->get_arg<int*>(1), src->get_arg<int>(2)));
		}

		inline void IS_PLAYER_PLAYING(rage::scrNativeCallContext* src)
		{
			// block undead OTR
			if (g.session.decloak_players && src->get_arg<Player>(0) != self::id && !NETWORK::NETWORK_IS_ACTIVITY_SESSION())
				src->set_return_value<BOOL>(TRUE);
			else
				src->set_return_value<BOOL>(PLAYER::IS_PLAYER_PLAYING(src->get_arg<Player>(0)));
		};

		inline void SET_ENTITY_VISIBLE(rage::scrNativeCallContext* src)
		{
			auto entity = src->get_arg<Entity>(0);
			auto toggle = src->get_arg<bool>(1);
			auto outfit = src->get_arg<bool>(2);

			if (g.self.invisibility && entity == self::ped && toggle)
				return;
			else
				ENTITY::SET_ENTITY_VISIBLE(entity, toggle, outfit);
		}

		void SET_BIGMAP_ACTIVE(rage::scrNativeCallContext* src)
		{
			if (!g.m_mission_creator_thread)
				HUD::SET_BIGMAP_ACTIVE(src->get_arg<bool>(0), src->get_arg<bool>(1));
		};

		void SET_BLIP_DISPLAY(rage::scrNativeCallContext* src)
		{
			if ((!g.m_mission_creator_thread) || src->get_arg<Blip>(0) != HUD::GET_MAIN_PLAYER_BLIP_ID())
				HUD::SET_BLIP_DISPLAY(src->get_arg<Blip>(0), src->get_arg<bool>(1));
		};

		void NETWORK_HAS_RECEIVED_HOST_BROADCAST_DATA(rage::scrNativeCallContext* src)
		{
			if (NETWORK::NETWORK_IS_ACTIVITY_SESSION() || NETWORK::NETWORK_IS_IN_TRANSITION() || NETWORK::NETWORK_IS_TRANSITION_TO_GAME() || NETWORK::NETWORK_IS_HOST_OF_THIS_SCRIPT())
			{
				src->set_return_value<BOOL>(NETWORK::NETWORK_HAS_RECEIVED_HOST_BROADCAST_DATA());
			}
			else
			{
				if (SCRIPT::GET_HASH_OF_THIS_SCRIPT_NAME() == "freemode"_J && g.session.fast_join)
				{
					scr_functions::set_freemode_session_active.call<void>();
					src->set_return_value<BOOL>(TRUE);
				}
				else
				{
					src->set_return_value<BOOL>(NETWORK::NETWORK_HAS_RECEIVED_HOST_BROADCAST_DATA());
				}
			}
		}

		void REMOVE_WEAPON_FROM_PED(rage::scrNativeCallContext* src)
		{
			auto ped         = src->get_arg<Ped>(0);
			auto weapon_hash = src->get_arg<Hash>(1);
			if (g.weapons.enable_mk1_variants)
			{
				static const std::unordered_set<Hash> weapon_variants = {
					"WEAPON_PISTOL"_J, "WEAPON_SMG"_J, "WEAPON_ASSAULTRIFLE"_J, "WEAPON_CARBINERIFLE"_J,
					"WEAPON_COMBATMG"_J, "WEAPON_HEAVYSNIPER"_J, "WEAPON_BULLPUPRIFLE"_J, "WEAPON_MARKSMANRIFLE"_J,
					"WEAPON_PUMPSHOTGUN"_J, "WEAPON_REVOLVER"_J, "WEAPON_SNSPISTOL"_J, "WEAPON_SPECIALCARBINE"_J,
					//MK2 variants
					"WEAPON_PISTOL_MK2"_J, "WEAPON_SMG_MK2"_J, "WEAPON_ASSAULTRIFLE_MK2"_J, "WEAPON_CARBINERIFLE_MK2"_J,
					"WEAPON_COMBATMG_MK2"_J, "WEAPON_HEAVYSNIPER_MK2"_J, "WEAPON_BULLPUPRIFLE_MK2"_J, "WEAPON_MARKSMANRIFLE_MK2"_J,
					"WEAPON_PUMPSHOTGUN_MK2"_J, "WEAPON_REVOLVER_MK2"_J, "WEAPON_SNSPISTOL_MK2"_J, "WEAPON_SPECIALCARBINE_MK2"_J };
				if (ped == self::ped && weapon_variants.contains(weapon_hash))
				{
					return;
				}
			}
			WEAPON::REMOVE_WEAPON_FROM_PED(ped, weapon_hash);
		}
	}
}
