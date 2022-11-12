#include "services/script_patcher/script_patcher_service.hpp"
#include <script/scrProgramTable.hpp>

namespace big
{
	void register_script_patches()
	{
		script_patch uncloak_players(RAGE_JOAAT("freemode"), "2D 01 08 00 ? 38 00 5D ? ? ? 2A 06", 5, {0x6E, 0x2E, 0x01, 0x01}, &g->session.decloak_players);
		script_patch script_host_kick_prot(RAGE_JOAAT("freemode"), "2D 01 04 00 ? 2C ? ? ? 5D ? ? ? 6E 57 ? ? 2C", 5, { 0x2E, 0x01, 0x00 }, &g->protections.script_host_kick);
		script_patch disable_death_barrier(RAGE_JOAAT("freemode"), "2D 01 09 00 00 5D ? ? ? 56 ? ? 2E", 5, { 0x2E, 0x01, 0x00 }, nullptr);

		for (auto& entry : *g_pointers->m_script_program_table)
		{
			if (entry.m_program)
				g_script_patcher_service.on_script_load(entry.m_program);
		}
	}
}