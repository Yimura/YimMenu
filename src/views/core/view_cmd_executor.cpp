#include "views/view.hpp"
#include "pointers.hpp"
#include "backend/context/default_command_context.hpp"

namespace big
{
	void view::cmd_executor()
	{
		if (!g.cmd_executor.enabled)
			return;

		float screen_x = (float)*g_pointers->m_resolution_x;
		float screen_y = (float)*g_pointers->m_resolution_y;

		ImGui::SetNextWindowPos(ImVec2(screen_x * 0.25f, screen_y * 0.2f), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.65f);
		ImGui::SetNextWindowSize({screen_x * 0.5f, -1});

		if (ImGui::Begin("cmd_executor", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
		{
			static char command_buffer[255];
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {10.f, 15.f});
			ImGui::Text("YimMenu Command Executor");

			// set focus by default on input box
			ImGui::SetKeyboardFocusHere(0);

			ImGui::SetNextItemWidth(screen_x * 0.5f);
			components::input_text_with_hint("", "Type your command", command_buffer, sizeof(command_buffer), ImGuiInputTextFlags_EnterReturnsTrue, []
			{
				std::string final_cmd = g_commands.contains(rage::joaat(command_buffer)) ? command_buffer : command::get_suggestions(command_buffer)[0]->get_name();
				if(command::process(command::get_suggestions(command_buffer)[0]->get_name(), std::make_shared<default_command_context>()))
				{
					command_buffer[0] = 0;
					g.cmd_executor.enabled = false;
				}
			});

			auto possible_commands = command::get_suggestions(command_buffer);
			for (auto cmd : possible_commands)
			{
				ImGui::Text(std::format("{} - {}", cmd->get_label(), cmd->get_description()).data());

				// check if we aren't on the last iteration
				if (cmd != possible_commands.back())
					ImGui::Separator();
			}

			ImGui::PopStyleVar();
		}
		ImGui::End();
	}

	bool_command g_cmd_executor("cmdexecutor", "Toggle Command Executor", "Toggles the command executor on or off", g.cmd_executor.enabled);
}