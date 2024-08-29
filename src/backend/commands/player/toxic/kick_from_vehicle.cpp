#include "backend/player_command.hpp"
#include "gta/net_object_mgr.hpp"
#include "pointers.hpp"

namespace big
{
	class kick_from_vehicle : player_command
	{
		using player_command::player_command;

		virtual CommandAccessLevel get_access_level() override
		{
			return CommandAccessLevel::AGGRESSIVE;
		}

		virtual void execute(player_ptr player, const command_arguments& _args, const std::shared_ptr<command_context> ctx) override
		{
			int vehicle = PED::GET_VEHICLE_PED_IS_IN(PLAYER::GET_PLAYER_PED(player->id()), false);

			if (!vehicle)
			{
				int playerMask = 0;
				for (int i = 0; i < 32; ++i) {
					int playerIndex = PLAYER::INT_TO_PLAYERINDEX(i);
					if (PED::IS_PED_IN_ANY_VEHICLE(PLAYER::GET_PLAYER_PED(playerIndex), false)) {
						int currentVehicle = PED::GET_VEHICLE_PED_IS_IN(PLAYER::GET_PLAYER_PED(playerIndex), false);
						if (currentVehicle == vehicle) {
							MISC::SET_BIT(&playerMask, i);
						}
					}
				}
				
				const size_t arg_count  = 10;
				int64_t args[arg_count] = {
					(int64_t)eRemoteEvent::VehicleKick, 
					self::id, 
					0,
					0,
					0,
					0,
					-1,
					MISC::GET_FRAME_COUNT(), //0xFC8202EFC642E6F2
					0,
					0
				};

				g_pointers->m_gta.m_trigger_script_event(1, args, arg_count, playerMask, (int)eRemoteEvent::VehicleKick);
			}
			else
			{
				// use a private method to kick player from vehicle
				(*g_pointers->m_gta.m_network_object_mgr)
				    ->ChangeOwner(vehicle->m_net_object, g_player_service->get_self()->get_net_game_player(), 0);
			}
		}
	};

	kick_from_vehicle g_kick_from_vehicle("vehkick", "VEHICLE_KICK", "VEHICLE_KICK_DESC", 0);
}
