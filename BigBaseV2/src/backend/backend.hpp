#pragma once
#include "common.hpp"
#include "looped/looped.hpp"
#include "script.hpp"
#include "services/context_menu_service.hpp"

namespace big
{
	class backend
	{
	public:
		static void loop();
		static void self_loop();
		static void weapons_loop();
		static void vehicles_loop();
		static void turnsignal_loop();
		static void rgbrandomizer_loop();
		static void misc_loop();
		static void remote_loop();
		static void noclip_loop();
		static void lscustoms_loop();
		static void vehiclefly_loop();
		static void hotkeys_input();
		static void hotkeys_loop();
		static void disable_control_action_loop();
	};
}
