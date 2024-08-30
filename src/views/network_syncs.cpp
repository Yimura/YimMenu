#include "core/var/sync_lists.hpp"
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
	    {"ped", &g_sync_lists.peds},
	    {"vehicle", &g_sync_lists.vehicles},
	    {"object", &g_sync_lists.objects},
	    {"pickup", &g_sync_lists.pickups},
	    {"explosion", &g_sync_lists.explosions},
	    {"ptfx", &g_sync_lists.ptfxs},
	};

	void view::network_syncs()
	{
		ImGui::BeginTabBar("network_syncs_tabbar");
		for (auto& pair : all_special_views)
			render_special_view(pair.first, pair.second);
		ImGui::EndTabBar();
	}
}