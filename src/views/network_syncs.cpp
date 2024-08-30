#include "core/var/misc.hpp"
#include "services/players/player_service.hpp"
#include "view.hpp"

#include <imgui.h>

namespace big
{
	static inline void render_special_view(std::string_view name, sync_list* list)
	{
		if (ImGui::BeginTabItem(name.data()))
		{
			ImGui::BeginChild("ScrollingRegion", {600, 450});
			for (int i = 0; i < list->size; ++i)
				ImGui::Text("%s", list->list[i].c_str());
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
	}

	static const std::unordered_map<std::string_view, sync_list*> all_special_views = {
	    {"ped", &g_misc_data.ped_sync_list},
	    {"vehicle", &g_misc_data.vehicle_sync_list},
	    {"object", &g_misc_data.object_sync_list},
	    {"pickup", &g_misc_data.pickup_sync_list},
	    {"explosion", &g_misc_data.explosion_sync_list},
	    {"ptfx", &g_misc_data.ptfx_sync_list},
	};

	void view::network_syncs()
	{
		ImGui::BeginTabBar("network_syncs_tabbar");
		for (auto& pair : all_special_views)
			render_special_view(pair.first, pair.second);
		ImGui::EndTabBar();
	}
}