#pragma once
#include "common.hpp"

namespace big
{
	class backend
	{
	public:
		static void loop();
		static void self_loop();
		static void ambient_animations_loop();
		static void weapons_loop();
		static void vehicles_loop();
		static void misc_loop();
		static void remote_loop();
		static void rainbowpaint_loop();
		static void disable_control_action_loop();
		static void world_loop();
		static void orbital_drone();
		static void vehicle_control();
		static void tunables_script();
		static void squad_spawner();
	};
}
