#include "gui/components/components.hpp"
#include "services/players/player_service.hpp"
#include "view_debug.hpp"

namespace big
{
	void debug::logs()
	{
		if (ImGui::BeginTabItem("DEBUG_TABS_LOGS"_T.data()))
		{
			static const char* options[]{"OFF"_T.data(), "ALL"_T.data(), "FILTERS"_T.data()};
			ImGui::Combo("DEBUG_LOG_METRICS"_T.data(), (int*)&g.debug.logs.metric_logs, options, IM_ARRAYSIZE(options));
			ImGui::Combo("VIEW_DEBUG_LOGS_LOG_PACKETS"_T.data(), (int*)&g.debug.logs.packet_logs, options, IM_ARRAYSIZE(options));
			ImGui::Checkbox("DEBUG_LOG_HTTP_START_REQUESTS"_T.data(), &g.debug.logs.http_start_request_logs);
			ImGui::Checkbox("DEBUG_LOG_NATIVE_SCRIPT_HOOKS"_T.data(), &g.debug.logs.script_hook_logs);

			if (ImGui::TreeNode("DEBUG_LOG_TREE_SCRIPT_EVENT"_T.data()))
			{
				ImGui::Checkbox("DEBUG_LOG_SCRIPT_EVENT"_T.data(), &g.debug.logs.script_event.logs);
				ImGui::Separator();

				ImGui::Checkbox("DEBUG_LOG_SCRIPT_EVENT_FILTER_BY_PLAYER"_T.data(), &g.debug.logs.script_event.filter_player);

				if (g.debug.logs.script_event.filter_player)
				{
					ImGui::BeginListBox("##filter_player");
					for (const auto& [_, player] : g_player_service->players())
					{
						if (components::selectable(player->get_name(), g.debug.logs.script_event.player_id == player->id()))
						{
							g.debug.logs.script_event.player_id = player->id();
						}
					}
					ImGui::EndListBox();
				}

				ImGui::Checkbox("VIEW_DEBUG_LOGS_BLOCK_ALL"_T.data(), &g.debug.logs.script_event.block_all);

				ImGui::TreePop();
			}

			ImGui::EndTabItem();
		}
	}
}