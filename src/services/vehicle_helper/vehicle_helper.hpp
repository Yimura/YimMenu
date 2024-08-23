#pragma once

#include "gta/vehicle_values.hpp"

namespace big
{
	class vehicle_helper
	{
	public:
		static bool check_mod_blacklist(Hash model, int mod_slot, int mod);
		static const char* get_mod_slot_name(Hash model, Vehicle vehicle, int mod_slot);
		static const char* get_mod_name(Hash model, Vehicle vehicle, int mod_slot, int mod, int mod_count);
	};
}
