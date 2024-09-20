#include "views/view.hpp"

namespace big
{
	void view::player_kick()
	{
		ImGui::SeparatorText("KICKS"_T.data());

		if (!g_player_service->get_self()->is_host())
			ImGui::Text("VIEW_PLAYER_KICK_HOST_AND_BREAKUP_KICK_REQUIRE_SESSION_HOST"_T.data());

		ImGui::BeginDisabled(!g_player_service->get_self()->is_host());
		components::player_command_button<"hostkick">(g_player_service->get_selected());
		ImGui::SameLine();
		components::player_command_button<"breakup">(g_player_service->get_selected());
		ImGui::SameLine();
		components::player_command_button<"battlekick">(g_player_service->get_selected());
		ImGui::SameLine();
		components::player_command_button<"battleban">(g_player_service->get_selected());
		ImGui::EndDisabled();
			
		components::player_command_button<"smartkick">(g_player_service->get_selected());
        ImGui::SameLine();
		components::player_command_button<"oomkick">(g_player_service->get_selected());
		ImGui::SameLine();	
		components::player_command_button<"shkick">(g_player_service->get_selected());
        ImGui::SameLine();
		components::player_command_button<"endkick">(g_player_service->get_selected());
		ImGui::SameLine();	
		components::player_command_button<"desync">(g_player_service->get_selected());
	}
}
