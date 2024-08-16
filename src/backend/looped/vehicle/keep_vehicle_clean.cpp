#include "backend/looped_command.hpp"
#include "natives.hpp"
#include "gta/vehicle_values.hpp"

namespace big
{
	class keep_vehicle_clean : looped_command
	{
		using looped_command::looped_command;

		virtual void on_tick() override
		{
			if (self::veh != 0 && VEHICLE::GET_PED_IN_VEHICLE_SEAT(self::veh, SEAT_DRIVER, FALSE) == self::ped)
			{
				VEHICLE::SET_VEHICLE_DIRT_LEVEL(self::veh, 0.0f);
			}
		}
	};

	keep_vehicle_clean
	    g_keep_vehicle_clean("keepvehicleclean", "KEEP_VEHICLE_CLEAN_CMD", "KEEP_VEHICLE_CLEAN_CMD_DESC", g.vehicle.keep_vehicle_clean);
}