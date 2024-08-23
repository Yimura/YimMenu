#include "backend.hpp"

#include "looped/looped.hpp"
#include "looped_command.hpp"
#include "script.hpp"
#include "script_patches.hpp"
#include "services/context_menu/context_menu_service.hpp"
#include "services/custom_teleport/custom_teleport_service.hpp"
#include "services/orbital_drone/orbital_drone.hpp"
#include "services/ped_animations/ped_animations_service.hpp"
#include "services/script_connection/script_connection_service.hpp"
#include "services/squad_spawner/squad_spawner.hpp"
#include "services/tunables/tunables_service.hpp"
#include "services/vehicle/vehicle_control_service.hpp"
#include "services/vehicle/xml_vehicles_service.hpp"
#include "services/xml_maps/xml_map_service.hpp"

namespace big
{
	void backend::loop()
	{
		for (auto& command : g_bool_commands)
			command->refresh();

		register_script_patches();

		g_squad_spawner_service.fetch_squads();
		g_xml_vehicles_service->fetch_xml_files();
		g_xml_map_service->fetch_xml_files();
		g_custom_teleport_service.fetch_saved_locations();
		g_ped_animation_service.fetch_saved_animations();

		while (g_running)
		{
			looped::system_self_globals();
			looped::system_update_pointers();
			looped::system_update_desync_kick();
			looped::system_spoofing();
			looped::system_mission_creator();

			for (auto command : g_looped_commands)
				if (command->is_enabled())
					command->on_tick();

			script::get_current()->yield();
		}
	}

	void backend::self_loop()
	{
		LOG(INFO) << "Starting script: Self";

		while (g_running)
		{
			looped::self_wanted();
			looped::self_hud();
			looped::self_persist_outfit();

			script::get_current()->yield();
		}
	}

	void backend::ambient_animations_loop()
	{
		while (g_running)
		{
			g_ped_animation_service.ambient_animations_prompt_tick();

			script::get_current()->yield();
		}
	}

	void backend::weapons_loop()
	{
		LOG(INFO) << "Starting script: Weapons";

		while (g_running)
		{
			looped::weapons_tp_gun();
			looped::weapons_paint_gun();
			looped::weapons_ammo_special_type();
			looped::weapons_cage_gun();
			looped::weapons_delete_gun();
			looped::weapons_gravity_gun();
			looped::weapons_repair_gun();
			looped::weapons_steal_vehicle_gun();
			looped::weapons_vehicle_gun();
			looped::weapons_c4_limit();
			looped::weapons_do_persist_weapons();
			looped::weapons_do_weapon_hotkeys();

			script::get_current()->yield();
		}
	}

	void backend::vehicles_loop()
	{
		LOG(INFO) << "Starting script: Vehicles";

		while (g_running)
		{
			looped::vehicle_auto_drive();
			looped::vehicle_allow_all_weapons();
			looped::vehicle_boost_behavior();
			looped::derail_train();
			looped::drive_train();

			script::get_current()->yield();
		}
	}

	void backend::misc_loop()
	{
		LOG(INFO) << "Starting script: Miscellaneous";

		while (g_running)
		{
			looped::hud_transition_state();
			looped::hud_disable_input();
			looped::session_pop_multiplier_areas();
			looped::session_force_thunder();
			looped::session_randomize_ceo_colors();
			looped::session_auto_kick_host();
			looped::session_force_script_host();
			looped::session_block_jobs();
			looped::session_chat_translator();
			looped::session_modder_detection();

			if (g_script_connection_service)
				g_script_connection_service->on_tick();

			script::get_current()->yield();
		}
	}

	void backend::remote_loop()
	{
		LOG(INFO) << "Starting script: Remote";

		while (g_running)
		{
			looped::player_good_options();
			looped::player_toxic_options();
			looped::player_spectate();
			looped::player_remote_control_vehicle();

			script::get_current()->yield();
		}
	}

	void backend::rainbowpaint_loop()
	{
		LOG(INFO) << "Starting script: Rainbow Paint";

		while (g_running)
		{
			looped::vehicle_rainbow_paint();

			script::get_current()->yield();
		}
	}

	void backend::disable_control_action_loop()
	{
		LOG(INFO) << "Starting script: Disable Control Action";

		while (g_running)
		{
			looped::custom_gun_disable_control_action();
			context_menu_service::disable_control_action_loop();

			script::get_current()->yield();
		}
	}

	void backend::world_loop()
	{
		LOG(INFO) << "Starting script: World";

		while (g_running)
		{
			looped::world_spawn_ped();
			script::get_current()->yield();
		}
	}

	void backend::orbital_drone()
	{
		while (true)
		{
			if (g.world.orbital_drone.enabled && PAD::IS_CONTROL_JUST_PRESSED(2, (int)ControllerInputs::INPUT_VEH_LOOK_BEHIND))
			{
				if (!g_orbital_drone_service.initialized())
					g_orbital_drone_service.init();
				else
					g_orbital_drone_service.destroy();
			}

			g_orbital_drone_service.tick();

			script::get_current()->yield();
		}
	}

	void backend::vehicle_control()
	{
		while (true)
		{
			g_vehicle_control_service.tick();

			script::get_current()->yield();
		}
	}

	void backend::tunables_script()
	{
		g_tunables_service->run_script();
	}

	void backend::squad_spawner()
	{
		while (true)
		{
			g_squad_spawner_service.tick();

			script::get_current()->yield();
		}
	}
}
