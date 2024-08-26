#pragma once
#include "debug/view_debug.hpp"
#include "esp/view_esp.hpp"
#include "gui/components/components.hpp"

//Percentage of window space
constexpr auto listbox_width  = 0.5f;
constexpr auto listbox_height = 0.2f;

namespace big
{

	inline ImVec2 get_listbox_dimensions()
	{
		return {750 * listbox_width, 1000 * listbox_height};
	}

	class view
	{
		inline static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav;

	public:
		static void active_view();
		static void esp_settings();
		static void outfit_editor();
		static void outfit_slots();
		static void stat_editor();
		static void gui_settings();
		static void hotkey_settings();
		static void handling_current_profile();
		static void handling_saved_profiles();
		static void reaction_settings();
		static void protection_settings();
		static void heading();
		static void mobile();
		static void navigation();
		static void notifications();
		static void overlay();
		static void root();
		static void self();
		static void animations();
		static void network();
		static void network_controls();
		static void chat();
		static void missions();
		static void player_database();
		static void session_browser();
		static void settings();
		static void vehicle();
		static void lsc();
		static void spawn_vehicle();
		static void pv();
		static void persist_car();
		static void proxy_settings();
		static void xml_vehicles();
		static void fun_vehicle();
		static void vehicle_control();
		static void spawn_ped();
		static void squad_spawner();
		static void time_and_weather();
		static void gravity();
		static void ocean();
		static void waypoint_and_objective();
		static void spoofing();
		static void teleport();
		static void custom_teleport();
		static void view_player();
		static void players();
		static void weapons();
		static void ammunation();
		static void context_menu();
		static void gta_data();
		static void creator();
		static void train();
		static void blackhole();
		static void model_swapper();
		static void world();
		static void xml_maps();
		static void gta_cache();
		static void lua_scripts();
		static void vfx();

		static void onboarding();

		static void cmd_executor();

		static void player_info();
		static void player_teleport();
		static void player_kick();
		static void player_toxic();
		static void player_misc();
		static void player_vehicle();
	};
}
