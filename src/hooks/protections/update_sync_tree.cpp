#include "hooking/hooking.hpp"
#include "pointers.hpp"

namespace big
{
	void hooks::update_sync_tree(rage::netSyncTree* _this, rage::netObject* object, uint32_t flags, uint32_t timestamp, bool a5)
	{
		if (!object->GetSyncData()) [[unlikely]]
			return;

		g_hooking->get_original<hooks::update_sync_tree>()(_this, object, flags, timestamp, a5);
	}
}