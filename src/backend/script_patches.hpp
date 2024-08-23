#pragma once
#include "pointers.hpp"
#include "services/script_patcher/script_patcher_service.hpp"

#include <script/scrProgramTable.hpp>

namespace big
{
	void register_script_patches()
	{
		g_script_patcher_service->add_patch({"freemode"_J, "freemode1", "2D 01 08 00 ? 38 00 5D ? ? ? 2A 06", 5, {0x71, 0x2E, 0x01, 0x01}, &g.session.decloak_players});
		g_script_patcher_service->add_patch({"freemode"_J, "script host kick", "2D 01 04 00 ? 2C ? ? ? 5D ? ? ? 71 57 ? ? 2C", 5, {0x2E, 0x01, 0x00}, nullptr}); // script host kick
		g_script_patcher_service->add_patch({"freemode"_J, "end session kick protection", "5D ? ? ? 76 57 ? ? 5D ? ? ? 76", 0, {0x2E, 0x00, 0x00}, nullptr}); // end session kick protection
		g_script_patcher_service->add_patch({"freemode"_J, "disable death when undermap/spectating", "2D 01 09 00 00 5D ? ? ? 56 ? ? 3A", 5, {0x2E, 0x01, 0x00}, nullptr}); // disable death when undermap/spectating
		g_script_patcher_service->add_patch({"freemode"_J, "load island even if stranded animal IPL choice is not set", "71 2E ? ? 55 ? ? 61 ? ? ? 47 ? ? 63", 0, {0x72}, nullptr}); // load island even if stranded animal IPL choice is not set
		g_script_patcher_service->add_patch({"freemode"_J, "disable population load balancing", "2D 00 07 00 00 7B", 5, {0x2E, 0x00, 0x00}, nullptr}); // disable population load balancing
		g_script_patcher_service->add_patch({"freemode"_J, "freemode7", "2D 02 08 00 00 38 01 56", 5, {0x2E, 0x02, 0x00}, &g.session.block_muggers});
		g_script_patcher_service->add_patch({"freemode"_J, "freemode8", "2D 00 CF 00 00", 5, {0x2E, 0x00, 0x00}, &g.session.block_ceo_raids});
		g_script_patcher_service->add_patch({"freemode"_J, "prevent normal blip update", "06 56 ? ? 38 02 2C ? ? ? 71 71", 0, {0x2B, 0x55}, &g.spoofing.spoof_blip}); // prevent normal blip update
		g_script_patcher_service->add_patch({"freemode"_J, "prevent normal blip update 2", "2C ? ? ? 55 ? ? 71 2C ? ? ? 61", 7, std::vector<uint8_t>(16, 0x0), &g.spoofing.spoof_blip}); // prevent normal blip update 2
		g_script_patcher_service->add_patch({"freemode"_J, "stop relinquishing invalid CEO slots", "2D 01 05 00 00 38 00 2C ? ? ? 39 03 38 03 2C ? ? ? 56", 5, {0x2E, 0x01, 0x00}, &g.session.block_ceo_creation}); // stop relinquishing invalid CEO slots
		g_script_patcher_service->add_patch({"freemode"_J, "prevent NPC bounties", "2D 04 06 00 00 25 1D", 5, {0x2E, 0x04, 0x00}, &g.protections.script_events.bounty}); // disable NPCs putting bounties on us.
		g_script_patcher_service->add_patch({"freemode"_J, "freemode9", "5D ? ? ? 56 ? ? 72 39 05 38 04 2C ? ? ? 58", 0, {0x2B, 0x2B, 0x2B, 0x00, 0x55}, &g.self.invisibility});
		g_script_patcher_service->add_patch({"freemode"_J, "freemode10", "2D 01 03 00 00 38 00 71 72 5D ? ? ? 06 56 ? ? 71 2E ? ? 2C ? ? ? 71", 5, {0x72, 0x2E, 0x01, 0x01}, &g.session.unhide_players_from_player_list});
		g_script_patcher_service->add_patch({"shop_controller"_J, "despawn bypass", "2D 01 04 00 00 2C ? ? ? 56 ? ? 71", 5, {0x71, 0x2E, 0x01, 0x01}, nullptr}); // despawn bypass
		g_script_patcher_service->add_patch({"shop_controller"_J, "godmode/invisibility detection bypass", "2D 01 03 00 00 5D ? ? ? 06 56 ? ? 2E ? ? 2C", 5, {0x2E, 0x01, 0x00}, nullptr}); // godmode/invisibility detection bypass
		g_script_patcher_service->add_patch({"carmod_shop"_J, "allow all vehicles", "2D 03 16 00 00 38 00", 5, {0x72, 0x2E, 0x03, 0x01}, nullptr}); // allow all vehicles
		g_script_patcher_service->add_patch({"carmod_shop"_J, "allow all vehicles 2", "2D 03 07 00 00 71 38 02", 5, {0x72, 0x2E, 0x03, 0x01}, nullptr}); // allow all vehicles 2
		g_script_patcher_service->add_patch({"main_persistent"_J, "vehicle clan logo SP bypass", "56 04 00 72 2E 01 01 2C 01 04 1F 5D ? ? ? 74", 0, {0x55}, nullptr}); // vehicle clan logo SP bypass

		for (auto& entry : *g_pointers->m_gta.m_script_program_table)
		{
			if (entry.m_program)
				g_script_patcher_service->on_script_load(entry.m_program);
		}
	}
}
