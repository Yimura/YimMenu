#pragma once
#include "core/enums.hpp"
#include "core/scr_globals.hpp"
#include "globals.hpp"
#include "gta_util.hpp"
#include "misc.hpp"
#include "natives.hpp"
#include "notify.hpp"
#include "script.hpp"
#include "core/scr_globals.hpp"
#include "script_local.hpp"
#include "vehicle.hpp"

namespace big::mobile
{
	namespace util
	{
		int get_current_personal_vehicle(); // forward declare
		inline void despawn_current_personal_vehicle()
		{
			misc::clear_bits(scr_globals::vehicle_global.at(get_current_personal_vehicle(), 142).at(103).as<int*>(), eVehicleFlags::TRIGGER_SPAWN_TOGGLE);
		}

		inline int get_current_personal_vehicle()
		{
			return *scr_globals::stats.at(0, 5568).at(681).at(2).as<int*>();
		}
	}

	namespace merry_weather
	{
		inline void request_ammo_drop()
		{
			*scr_globals::freemode_global.at(906).as<int*>() = 1;
		}

		inline void request_boat_pickup()
		{
			*scr_globals::freemode_global.at(907).as<int*>() = 1;
		}

		inline void request_helicopter_pickup()
		{
			*scr_globals::freemode_global.at(908).as<int*>() = 1;
		}

		inline void request_backup_helicopter()
		{
			*scr_globals::freemode_global.at(4506).as<int*>() = 1;
		}

		inline void request_airstrike()
		{
			*scr_globals::freemode_global.at(4507).as<int*>() = 1;
		}
	}

	namespace mors_mutual
	{
		inline bool fix_index(int veh_idx, bool spawn_veh = false)
		{
			bool can_be_fixed = misc::has_bits_set(scr_globals::vehicle_global.at(veh_idx, 142).at(103).as<int*>(), eVehicleFlags::DESTROYED | eVehicleFlags::HAS_INSURANCE);

			if (can_be_fixed)
			{
				misc::clear_bits(scr_globals::vehicle_global.at(veh_idx, 142).at(103).as<int*>(), eVehicleFlags::DESTROYED | eVehicleFlags::IMPOUNDED | eVehicleFlags::UNK2);

				if (spawn_veh)
				{
					misc::set_bits(scr_globals::vehicle_global.at(veh_idx, 142).at(103).as<int*>(), eVehicleFlags::TRIGGER_SPAWN_TOGGLE | eVehicleFlags::SPAWN_AT_MORS_MUTUAL);
				}
			}
			return can_be_fixed;
		}

		inline int fix_all()
		{
			int fixed_count = 0;

			const int arr_size = *scr_globals::vehicle_global.as<int*>();
			for (int i = 0; i < arr_size; i++)
				if (fix_index(i, true))
					fixed_count++;

			return fixed_count;
		}
	}

	namespace ceo_abilities
	{
		inline void request_bullshark_testosterone()
		{
			*scr_globals::freemode_properties.at(3733).as<int*>() = 1;
		}

		inline void request_ballistic_armor() //i think this is a ceo ability atleast?
		{
			*scr_globals::freemode_global.at(906).as<int*>() = 1;
		}
	}

	namespace services
	{
		inline void request_avenger()
		{
			*scr_globals::freemode_global.at(953).as<int*>() = 1;
		}

		inline void request_kosatka()
		{
			*scr_globals::freemode_global.at(975).as<int*>() = 1;
		}

		inline void request_mobile_operations_center()
		{
			*scr_globals::freemode_global.at(945).as<int*>() = 1;
		}

		inline void request_terrorbyte()
		{
			*scr_globals::freemode_global.at(958).as<int*>() = 1;
		}

		inline void request_acidlab()
		{
			*scr_globals::freemode_global.at(959).as<int*>() = 1;
		}

		inline void request_acidlab_bike()
		{
			*scr_globals::freemode_global.at(1009).as<int*>() = 1;
		}
	}

	namespace mechanic
	{
		inline Vehicle get_personal_vehicle()
		{
			return *scr_globals::freemode_global.at(301).as<Vehicle*>();
		}

		inline CVehicle* get_personal_cvehicle()
		{
			Vehicle personal_vehicle = get_personal_vehicle();
			if (personal_vehicle != -1 && ENTITY::DOES_ENTITY_EXIST(personal_vehicle))
			{
				return reinterpret_cast<CVehicle*>(g_pointers->m_gta.m_handle_to_ptr(personal_vehicle));
			}

			return nullptr;
		}

		inline void summon_vehicle_by_index(int veh_idx)
		{
			if (*scr_globals::freemode_global.at(1000).as<int*>() != -1)
				return g_notification_service.push_warning("VEHICLE"_T.data(), "VEHICLE_MECHANIC_BUSY"_T.data());

			if (g.clone_pv.spawn_inside && self::veh)
				TASK::CLEAR_PED_TASKS_IMMEDIATELY(PLAYER::PLAYER_PED_ID());

			// despawn current veh
			util::despawn_current_personal_vehicle();
			mors_mutual::fix_index(veh_idx);

			script::get_current()->yield(100ms);

			// only do this when spawn inside is enabled otherwise the vehicle will spawn relatively far away from players
			if (g.clone_pv.spawn_inside)
			{
				*scr_globals::freemode_global.at(957).as<int*>() = 1; // disable vehicle node distance check
			}
			*scr_globals::freemode_global.at(943).as<int*>() = 1; // tell freemode to spawn our vehicle
			*scr_globals::freemode_global.at(1003).as<int*>() = 0; // required
			*scr_globals::freemode_global.at(1000).as<int*>() = veh_idx;

			script::get_current()->yield(100ms);

			GtaThread* freemode_thread = gta_util::find_script_thread("freemode"_J);
			if (freemode_thread)
			{
				// regex to find this shit easily
				// \(func_\d{3}\(&\(uParam0->f_\d{3}\), \d+000, 0\) \|\| func
				// or if you prefer a string "VD_FAIL4"
				// or if you really prefer an image https://i.imgur.com/K8vMILe.png
				*scr_locals::freemode::mobile.set(freemode_thread).at(176).as<int*>() = 0; // spawn vehicle instantly
			}

			// blocking call till vehicle is delivered
			notify::busy_spinner("Delivering vehicle...", scr_globals::freemode_global.at(1000).as<int*>(), -1);

			if (g.clone_pv.spawn_inside)
			{
				vehicle::bring(get_personal_vehicle(), self::pos, true);
			}
		}
	}

	namespace mobile_misc
	{
		inline void request_taxi()
		{
			*scr_globals::freemode_global.at(868).as<int*>() = 1;
		}

		inline void request_gun_van()
		{
			auto local_pos      = self::pos;
			auto forward_vector = ENTITY::GET_ENTITY_FORWARD_VECTOR(self::ped);
			Vector3 spawn_point;

			if (MISC::FIND_SPAWN_POINT_IN_DIRECTION(local_pos.x,
			        local_pos.y,
			        local_pos.z,
			        forward_vector.x,
			        forward_vector.y,
			        forward_vector.z,
			        25.f,
			        &spawn_point))
			{
				*scr_globals::gun_van.as<Vector3*>() = spawn_point;

				return g_notification_service.push_success("GUI_TAB_MOBILE"_T.data(), "REQUEST_GUN_VAN_NOTIFY_SUCCESS"_T.data());
			}

			g_notification_service.push_warning("GUI_TAB_MOBILE"_T.data(), "REQUEST_GUN_VAN_NOTIFY_FAILED"_T.data());
		}
	}
}
