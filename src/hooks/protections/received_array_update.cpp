#include "core/scr_globals.hpp"
#include "gta/net_array.hpp"
#include "gta/script_handler.hpp"
#include "gta_util.hpp"
#include "hooking/hooking.hpp"
#include "script_local.hpp"
#include "services/players/player_service.hpp"
#include "util/misc.hpp"

#include <script/globals/GlobalPlayerBD.hpp>

namespace big
{
	bool hooks::received_array_update(rage::netArrayHandlerBase* array, CNetGamePlayer* sender, rage::datBitBuffer* buffer, int size, std::int16_t cycle)
	{
		int old_beast_index = -1;
		int participant_id  = 0;
		auto beast          = gta_util::find_script_thread("am_hunt_the_beast"_J);

		if (beast)
		{
			old_beast_index = *script_local(beast->m_stack, scr_locals::am_hunt_the_beast::broadcast_idx).at(1).at(6).as<int*>();
			if (beast->m_net_component)
				participant_id = ((CGameScriptHandlerNetComponent*)beast->m_net_component)->m_local_participant_index;
		}

		bool result = g_hooking->get_original<hooks::received_array_update>()(array, sender, buffer, size, cycle);

		if (beast && array->m_array == script_local(beast->m_stack, scr_locals::am_hunt_the_beast::broadcast_idx).as<void*>()
		    && old_beast_index
		        != *script_local(beast->m_stack, scr_locals::am_hunt_the_beast::broadcast_idx).at(1).at(6).as<int*>()
		    && *script_local(beast->m_stack, scr_locals::am_hunt_the_beast::broadcast_idx).at(1).at(6).as<int*>() == participant_id
		    && !misc::has_bit_set(script_local(beast->m_stack, scr_locals::am_hunt_the_beast::player_broadcast_idx)
		                              .at(participant_id, 4)
		                              .at(3)
		                              .as<int*>(),
		        1))
		{
			*script_local(beast->m_stack, scr_locals::am_hunt_the_beast::broadcast_idx).at(1).at(6).as<int*>()    = -1;
			*script_local(beast->m_stack, scr_locals::am_hunt_the_beast::broadcast_idx).at(1).at(7).as<Player*>() = -1;

			if (auto plyr = g_player_service->get_by_id(sender->m_player_id))
				g.reactions.turn_into_beast.process(plyr);
		}

		if (g.protections.remote_wanted_level && (array->m_array >= scr_globals::globalplayer_bd.as<uint8_t*>()
		        && array->m_array <= scr_globals::globalplayer_bd.at(31, sizeof(GlobalPlayerBDEntry) / 8).as<uint8_t*>())
		    && scr_globals::globalplayer_bd.as<GlobalPlayerBD*>()->Entries[sender->m_player_id].RemoteWantedLevelPlayer != -1)
		{
			if (auto plyr = g_player_service->get_by_id(sender->m_player_id))
			{
				if (scr_globals::globalplayer_bd.as<GlobalPlayerBD*>()->Entries[sender->m_player_id].RemoteWantedLevelPlayer == self::id)
				{
					g.reactions.remote_wanted_level.process(plyr);
				}
				else if (auto victim = g_player_service->get_by_id(
				             scr_globals::globalplayer_bd.as<GlobalPlayerBD*>()->Entries[sender->m_player_id].RemoteWantedLevelPlayer))
				{
					g.reactions.remote_wanted_level_others.process(plyr, victim);
				}
			}

			scr_globals::globalplayer_bd.as<GlobalPlayerBD*>()->Entries[sender->m_player_id].RemoteWantedLevelPlayer = -1; // reset locally
		}

		if (array->m_array == scr_globals::gsbd.as<void*>() && *scr_globals::gsbd.as<eFreemodeState*>() == eFreemodeState::CLOSING)
		{
			*scr_globals::gsbd.as<eFreemodeState*>() = eFreemodeState::RUNNING;

			if (auto plyr = g_player_service->get_by_id(sender->m_player_id))
				g.reactions.kick.process(plyr);
		}

		return result;
	}
}
