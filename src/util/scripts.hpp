#pragma once
#include "core/data/all_script_names.hpp"
#include "core/scr_globals.hpp"
#include "fiber_pool.hpp"
#include "gta/script_handler.hpp"
#include "gta_util.hpp"
#include "misc.hpp"
#include "natives.hpp"
#include "script.hpp"
#include "script_local.hpp"
#include "services/players/player_service.hpp"

#include <memory/pattern.hpp>
#include <script/globals/GPBD_FM_3.hpp>

namespace big::scripts
{
	inline bool is_loaded(int hash)
	{
		return SCRIPT::HAS_SCRIPT_WITH_NAME_HASH_LOADED(hash);
	}

	inline bool is_running(int hash)
	{
		return SCRIPT::GET_NUMBER_OF_THREADS_RUNNING_THE_SCRIPT_WITH_THIS_HASH(hash) > 0;
	}

	inline void request_script(int hash)
	{
		SCRIPT::REQUEST_SCRIPT_WITH_NAME_HASH(hash);
	}

	inline int start_script_with_args(int hash, int* args, int arg_size, int stack_size)
	{
		int thread_id = SYSTEM::START_NEW_SCRIPT_WITH_NAME_HASH_AND_ARGS(hash, args, arg_size, stack_size);
		SCRIPT::SET_SCRIPT_WITH_NAME_HASH_AS_NO_LONGER_NEEDED(hash);
		return thread_id;
	}

	inline bool wait_till_loaded(int hash)
	{
		if (is_loaded(hash))
			return true;
		for (int i = 0; i < 150 && !is_loaded(hash); i++)
			script::get_current()->yield(10ms);
		if (is_loaded(hash))
			return true;
		return false;
	}

	inline bool wait_till_running(int hash)
	{
		if (is_running(hash))
			return true;
		for (int i = 0; i < 150 && !is_running(hash); i++)
			script::get_current()->yield(10ms);
		if (is_running(hash))
			return true;
		return false;
	}

	inline bool force_host(rage::joaat_t hash)
	{
		if (auto launcher = gta_util::find_script_thread(hash); launcher && launcher->m_net_component)
		{
			for (int i = 0; !((CGameScriptHandlerNetComponent*)launcher->m_net_component)->is_local_player_host(); i++)
			{
				if (i > 200)
					return false;

				((CGameScriptHandlerNetComponent*)launcher->m_net_component)
				    ->send_host_migration_event(g_player_service->get_self()->get_net_game_player());
				script::get_current()->yield(10ms);

				if (!launcher->m_stack || !launcher->m_net_component)
					return false;
			}
		}

		return true;
	}

	inline int launcher_index_from_hash(rage::joaat_t script_hash)
	{
		for (int i = 0; i < launcher_scripts.size(); i++)
			if (launcher_scripts[i] == script_hash)
				return i;

		return -1;
	}

	// force launcher script over the lobby, take two
	// try to get am_launcher in a consistent state before trying to start the script taking account of all participants
	inline void start_launcher_script(rage::joaat_t script_hash)
	{
		auto script_id = launcher_index_from_hash(script_hash);

		static auto check_players_in_state = [](GtaThread* launcher, int state) -> bool {
			bool set = false;

			if (!launcher->m_net_component)
				return false;

			for (auto& [_, plyr] : g_player_service->players())
			{
				if (((CGameScriptHandlerNetComponent*)launcher->m_net_component)->is_player_a_participant(plyr->get_net_game_player()))
				{
					if (*script_local(launcher->m_stack, 238).at(plyr->id(), 3).at(2).as<int*>() == state)
					{
						set = true;
						break;
					}
				}
			}

			return set;
		};

		// 1) Get launcher
		if (auto launcher = gta_util::find_script_thread("am_launcher"_J))
		{
			// 2) Force host of launcher
			if (!force_host("am_launcher"_J))
			{
				// 2F) Failed to force host of launcher
				g_notification_service.push_error("Script", "Cannot force script host of am_launcher");
				return;
			}

			launcher->m_context.m_state = rage::eThreadState::unk_3; // prevent bad things from happening to the thread in the meantime

			// 3) Remove players from that annoying waiting stage
			if (check_players_in_state(launcher, 5))
			{
				for (int i = 0; check_players_in_state(launcher, 5); i++)
				{
					if (i > 200)
						break; // 3F) Timeout

					*scr_globals::launcher_global.at(3).at(1).as<int*>() = 0;
					*scr_globals::launcher_global.at(2).as<int*>()       = 6;
					script::get_current()->yield(10ms);
				}
			} // State should now be 6 or 0

			// 4) Check if a script is already being executed, and unstuck from that state if so
			if (check_players_in_state(launcher, 6))
			{
				for (int i = 0; check_players_in_state(launcher, 6); i++)
				{
					if (i > 200)
						break; // 4F) Timeout

					*scr_globals::launcher_global.at(3).at(1).as<int*>() = 0;
					*scr_globals::launcher_global.at(2).as<int*>()       = 7;
					script::get_current()->yield(10ms);
				}
			} // State should now be 7 or 0

			// 5) Get everyone out of state 7
			if (check_players_in_state(launcher, 7))
			{
				for (int i = 0; check_players_in_state(launcher, 7); i++)
				{
					if (i > 200)
						break; // 5F) Timeout

					*scr_globals::launcher_global.at(2).as<int*>() = 0;
					script::get_current()->yield(10ms);
				}
			} // State should now be 0

			// 6) Actually get the script to start
			misc::set_bit(scr_globals::launcher_global.at(1).as<int*>(), 1); // run immediately
			*scr_globals::launcher_global.at(2).as<int*>() = 6; // will change to 7 shortly but that's fine as players are guaranteed not to be in the waiting stage
			*script_local(launcher->m_stack, 238).at(self::id, 3).at(2).as<int*>() = 6;
			*scr_globals::launcher_global.at(3).at(1).as<int*>()                   = script_id;

			launcher->m_context.m_state = rage::eThreadState::running;
		}
		else
		{
			// 1F) Cannot find launcher
			g_notification_service.push_error("Script", "Cannot start script, am_launcher not running locally");
		}
	}

	inline void force_script_on_player(player_ptr player, rage::joaat_t script_hash, int instance = -1)
	{
		const size_t arg_count  = 27;
		int64_t args[arg_count] = {(int64_t)eRemoteEvent::StartScriptBegin, (int64_t)self::id, 1 << player->id()};

		args[3] = scripts::launcher_index_from_hash(script_hash);
		strcpy((char*)&args[3 + 3], "0");
		args[3 + 16] = instance;
		args[3 + 17] = 1337;
		args[3 + 19] = 0;
		args[25] = scr_globals::gpbd_fm_3.as<GPBD_FM_3*>()->Entries[player->id()].ScriptEventReplayProtectionCounter;

		g_pointers->m_gta.m_trigger_script_event(1, args, arg_count, 1 << player->id(), (int)eRemoteEvent::StartScriptBegin);

		for (int i = 0; i < 2; i++)
		{
			const size_t arg_count_2 = 27;
			int64_t args_2[arg_count_2] = {(int64_t)eRemoteEvent::StartScriptProceed, (int64_t)self::id, 1 << player->id()};
			args_2[3 + 17] = 1337;
			g_pointers->m_gta.m_trigger_script_event(1, args_2, arg_count_2, 1 << player->id(), (int)eRemoteEvent::StartScriptProceed);

			script::get_current()->yield(20ms);
		}
	}

	inline const std::optional<uint32_t> get_code_location_by_pattern(rage::scrProgram* program, const memory::pattern& pattern)
	{
		uint32_t code_size = program->m_code_size;
		for (uint32_t i = 0; i < (code_size - pattern.m_bytes.size()); i++)
		{
			for (uint32_t j = 0; j < pattern.m_bytes.size(); j++)
				if (pattern.m_bytes[j].has_value())
					if (pattern.m_bytes[j].value() != *program->get_code_address(i + j))
						goto incorrect;

			return i;
		incorrect:
			continue;
		}

		return std::nullopt;
	}

	// we can't use the script patch service for this
	inline void patch_script(rage::scrProgram* program, std::optional<uint32_t> location, std::vector<uint8_t> patch, int offset)
	{
		uint8_t* bytearray = patch.data();
		if (location)
			memcpy(program->get_code_address(location.value() + offset), bytearray, patch.size());
	}

	inline void start_creator_script(rage::joaat_t hash)
	{
		static auto read_uint24_t = [](uint8_t* arr) {
			return arr[0] + (arr[1] << 8) + (arr[2] << 16);
		};

		if (g.m_mission_creator_thread || SCRIPT::GET_NUMBER_OF_THREADS_RUNNING_THE_SCRIPT_WITH_THIS_HASH("creator"_J) != 0 || SCRIPT::GET_NUMBER_OF_THREADS_RUNNING_THE_SCRIPT_WITH_THIS_HASH("maintransition"_J) != 0 || STREAMING::IS_PLAYER_SWITCH_IN_PROGRESS() || CUTSCENE::IS_CUTSCENE_ACTIVE())
		{
			g_notification_service.push_warning("GUI_TAB_CREATOR"_T.data(), "CREATOR_CANNOT_START"_T.data());
			return;
		}

		if (MISC::GET_NUMBER_OF_FREE_STACKS_OF_THIS_SIZE(62500) == 0)
		{
			g_notification_service.push_warning("GUI_TAB_CREATOR"_T.data(), "CREATOR_NO_FREE_STACKS"_T.data());
		}

		while (!SCRIPT::HAS_SCRIPT_WITH_NAME_HASH_LOADED(hash))
		{
			SCRIPT::REQUEST_SCRIPT_WITH_NAME_HASH(hash);
			script::get_current()->yield();
		}

		*scr_globals::terminate_creator.as<bool*>()                    = false;
		*scr_globals::mission_creator_exited.as<bool*>()               = false;
		*scr_globals::mission_creator_radar_follows_camera.as<bool*>() = true;

		if (SYSTEM::START_NEW_SCRIPT_WITH_NAME_HASH(hash, 62500))
		{
			g.m_mission_creator_thread = gta_util::find_script_thread(hash);
		}

		if (auto program = gta_util::find_script_program(hash))
		{
			patch_script(program,
			    get_code_location_by_pattern(program, "2D 02 04 00 ? 38 01 38 00 42 13"),
			    {
			        0x72, // PUSH_CONST_1
			        0x00  // NOP
			    },
			    5); // place anywhere

			patch_script(program, get_code_location_by_pattern(program, "71 08 2A 56 ? ? 2C ? ? ? 1F 56 ? ? 72"), {0x00, 0x00, 0x00, 0x00, 0x00}, 0xE); // don't bail on network mode

			if (auto loc = get_code_location_by_pattern(program, "39 04 5D ? ? ? 71"))
			{
				patch_script(program,
				    read_uint24_t(program->get_code_address(loc.value() + 3)),
				    {
				        0x73, // PUSH_CONST_2 0 = mp, 2 = creator, 999 = singleplayer
				        0x2E,
				        0x00,
				        0x01 // LEAVE 0 1
				    },
				    5); // allow fast zoom in mp
			}
		}

		SCRIPT::SET_SCRIPT_WITH_NAME_HASH_AS_NO_LONGER_NEEDED(hash);
	}
}
