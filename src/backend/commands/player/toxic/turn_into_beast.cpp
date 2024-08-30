#include "backend/player_command.hpp"
#include "core/scr_globals.hpp"
#include "natives.hpp"
#include "util/scripts.hpp"

namespace big
{
	class turn_into_beast : player_command
	{
		using player_command::player_command;

		virtual CommandAccessLevel get_access_level() override
		{
			return CommandAccessLevel::AGGRESSIVE;
		}

		virtual void execute(player_ptr player, const command_arguments& _args, const std::shared_ptr<command_context> ctx) override
		{
			auto id = player->id();

			if (!NETWORK::NETWORK_IS_PLAYER_A_PARTICIPANT_ON_SCRIPT(id, "am_hunt_the_beast", -1))
			{
				if (!NETWORK::NETWORK_IS_PLAYER_A_PARTICIPANT_ON_SCRIPT(id, "am_launcher", -1))
				{
					g_notification_service.push_error("TURN_INTO_BEAST"_T.data(), "BACKEND_TURN_INTO_BEAST_CANNOT_START_AM_LAUNCHER"_T.data());
					return;
				}

				g_notification_service.push("TURN_INTO_BEAST"_T.data(), "BACKEND_TURN_INTO_BEAST_STARTING"_T.data());

				scripts::start_launcher_script("AM_HUNT_THE_BEAST"_J);

				for (int i = 0; !NETWORK::NETWORK_IS_PLAYER_A_PARTICIPANT_ON_SCRIPT(id, "am_hunt_the_beast", -1); i++)
				{
					if (i >= 1000)
					{
						g_notification_service.push_error("TURN_INTO_BEAST"_T.data(), "BACKEND_TURN_INTO_BEAST_FAILED"_T.data());
						return;
					}

					script::get_current()->yield(1ms);
				}
			}

			if (!NETWORK::NETWORK_IS_PLAYER_CONNECTED(id))
				return;

			if (!scripts::force_host("am_hunt_the_beast"_J))
			{
				g_notification_service.push_error("TURN_INTO_BEAST"_T.data(), "BACKEND_TURN_INTO_BEAST_FAILED_CONTROL"_T.data());
				return;
			}

			auto thread        = gta_util::find_script_thread("am_hunt_the_beast"_J);
			auto stack         = thread->m_stack;
			auto net_component = (CGameScriptHandlerNetComponent*)thread->m_net_component;
			auto idx           = scr_locals::am_hunt_the_beast::broadcast_idx;

			if (!stack || !net_component || !player->is_valid())
				return;

			*script_local(stack, idx).as<int*>()       = 1;
			*script_local(stack, idx).at(1).as<int*>() = 2; // stage
			*script_local(stack, idx).at(1).at(6).as<int*>() = net_component->get_participant_index(player->get_net_game_player()); // beast participant idx
			*script_local(stack, idx).at(1).at(7).as<Player*>()     = id;      // beast player idx
			*script_local(stack, idx).at(1).at(2).as<int*>()        = INT_MAX; // stopwatch time
			*script_local(stack, idx).at(1).at(2).at(1).as<bool*>() = true;    // stopwatch initialized
			*script_local(stack, idx).at(1).at(4).at(1).as<bool*>() = false;   // destroy old stage 1 stopwatch
			*script_local(stack, idx).at(1).at(9).as<int*>()        = 2;       // some distance check
			*script_local(stack, idx).at(83).as<int*>()             = 0;       // transformed bitset
		}
	};

	class turn_into_beast_all : command
	{
		using command::command;

		virtual CommandAccessLevel get_access_level()
		{
			return CommandAccessLevel::AGGRESSIVE;
		}

		virtual void execute(const command_arguments& _args, const std::shared_ptr<command_context> ctx)
		{
			scripts::start_launcher_script("AM_HUNT_THE_BEAST"_J);

			for (int i = 0; !scripts::is_running("am_launcher"_J); i++)
			{
				if (i >= 7000)
				{
					g_notification_service.push_error("TURN_INTO_BEAST"_T.data(), "BACKEND_TURN_INTO_BEAST_FAILED"_T.data());
					return;
				}

				script::get_current()->yield(1ms);
			}

			script::get_current()->yield(500ms);

			if (!scripts::force_host("am_hunt_the_beast"_J))
			{
				g_notification_service.push_error("TURN_INTO_BEAST"_T.data(), "BACKEND_TURN_INTO_BEAST_FAILED_CONTROL"_T.data());
				return;
			}

			script::get_current()->yield(3s);

			auto thread = gta_util::find_script_thread("am_hunt_the_beast"_J);

			if (!thread)
				return;

			auto stack         = thread->m_stack;
			auto net_component = (CGameScriptHandlerNetComponent*)thread->m_net_component;
			auto idx           = scr_locals::am_hunt_the_beast::broadcast_idx;

			if (!stack || !net_component)
				return;

			thread->m_context.m_state = rage::eThreadState::paused;
			g.m_hunt_the_beast_thread = thread;

			for (int i = 0; i < 15; i++)
			{
				*script_local(stack, idx).as<int*>()                    = 1;
				*script_local(stack, idx).at(1).as<int*>()              = 2;         // stage
				*script_local(stack, idx).at(1).at(6).as<int*>()        = __rdtsc(); // participant idx
				*script_local(stack, idx).at(1).at(7).as<Player*>()     = __rdtsc(); // beast player idx
				*script_local(stack, idx).at(1).at(2).as<int*>()        = INT_MAX;   // stopwatch time
				*script_local(stack, idx).at(1).at(2).at(1).as<bool*>() = true;      // stopwatch initialized
				*script_local(stack, idx).at(1).at(4).at(1).as<bool*>() = false;     // destroy old stage 1 stopwatch
				*script_local(stack, idx).at(1).at(9).as<int*>()        = 2;         // some distance check
				*script_local(stack, idx).at(83).as<int*>()             = 0;         // transformed bitset
				script::get_current()->yield(350ms);
			}

			// unfortunately we must also turn ourselves into the beast to prevent the script from exiting due to a "missing player"

			*script_local(stack, idx).at(1).at(6).as<int*>() = net_component->m_local_participant_index; // participant idx
			*script_local(stack, idx).at(1).at(7).as<Player*>() = self::id; // beast player idx
			*script_local(stack, idx).at(1).at(2).as<int*>()    = INT_MAX;  // stopwatch time
			*script_local(stack, idx).at(83).as<int*>()         = 0;        // transformed bitset

			thread->m_context.m_state = rage::eThreadState::running;
		}
	};

	turn_into_beast g_turn_into_beast("beast", "TURN_INTO_BEAST", "TURN_INTO_BEAST_DESC", 0, false);
	turn_into_beast_all g_turn_into_beast_all("beastall", "TURN_INTO_BEAST_ALL", "TURN_INTO_BEAST_ALL_DESC", 0);
}
