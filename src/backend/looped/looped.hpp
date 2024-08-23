#pragma once
#include "common.hpp"
#include "services/players/player_service.hpp"

namespace big
{
	struct spawned_ped
	{
		Ped ped_handle;
		Player spawned_for_player;
		bool is_bodyguard;
		bool is_attacker;
	};

	inline std::vector<spawned_ped> spawned_peds;

	class looped
	{
	public:
		static void hud_transition_state();
		static void hud_disable_input();

		static void player_good_options();
		static void player_toxic_options();
		static void player_spectate();
		static void player_remote_control_vehicle();

		static void self_wanted();
		static void self_hud();
		static void self_persist_outfit();

		static void session_pop_multiplier_areas();
		static void session_force_thunder();
		static void session_block_jobs();
		static void session_randomize_ceo_colors();
		static void session_auto_kick_host();
		static void session_force_script_host();
		static void session_chat_translator();
		static void session_modder_detection();

		static void system_self_globals();
		static void system_update_pointers();
		static void system_update_desync_kick();
		static void system_spoofing();
		static void system_mission_creator();

		static void vehicle_auto_drive();
		static void vehicle_allow_all_weapons();
		static void vehicle_boost_behavior();
		static void vehicle_rainbow_paint();

		static void weapons_tp_gun();
		static void weapons_ammo_special_type();
		static void weapons_cage_gun();
		static void custom_gun_disable_control_action();
		static void weapons_delete_gun();
		static void weapons_gravity_gun();
		static void weapons_repair_gun();
		static void weapons_steal_vehicle_gun();
		static void weapons_vehicle_gun();
		static void weapons_c4_limit();
		static void weapons_do_persist_weapons();
		static void weapons_do_weapon_hotkeys();
		static void weapons_paint_gun();

		static void drive_train();
		static void derail_train();

		static void world_spawn_ped();
	};
}
