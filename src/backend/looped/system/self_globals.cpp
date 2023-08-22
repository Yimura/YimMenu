#include "backend/looped/looped.hpp"
#include "natives.hpp"
#include "pointers.hpp"

#include <network/CNetworkPlayerMgr.hpp>

namespace big
{
	void looped::system_self_globals()
	{
		if (!*g_pointers->m_gta.m_network_player_mgr || !(*g_pointers->m_gta.m_network_player_mgr)->m_local_net_player
		    || (*g_pointers->m_gta.m_network_player_mgr)->m_local_net_player->m_player_id == -1)
			self::id = 0;
		else
			self::id = (*g_pointers->m_gta.m_network_player_mgr)->m_local_net_player->m_player_id;

		self::ped = PLAYER::PLAYER_PED_ID();

		self::pos = ENTITY::GET_ENTITY_COORDS(self::ped, false /*Unused*/);

		self::rot = ENTITY::GET_ENTITY_ROTATION(self::ped, 2);

		int original_heap_size = *g_pointers->m_gta.m_heap_size / 1024 / 1024; //Compiler will optimize this out to right shift by 20 instead of 2 divides.
		if (original_heap_size != 650)
		{
			LOG(INFO) << "Original Memory Heap Size: " << original_heap_size << "MB";
			DWORD old_heap_protect;
			VirtualProtect(g_pointers->m_gta.m_heap_size, 4, PAGE_READWRITE, &old_heap_protect);
			*g_pointers->m_gta.m_heap_size = 650 * 1024 * 1024;
			VirtualProtect(g_pointers->m_gta.m_heap_size, 4, old_heap_protect, &old_heap_protect);
			LOG(INFO) << "Modified Memory Heap Size To: 650MB";
		}

		if (PED::IS_PED_IN_ANY_VEHICLE(self::ped, 0))
		{
			self::veh = PED::GET_VEHICLE_PED_IS_IN(self::ped, false);
		}
		else
		{
			self::veh = 0;
		}
	}
}
