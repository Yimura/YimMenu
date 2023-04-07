#include "gui/components/components.hpp"
#include "services/locals/locals_service.hpp"
#include "view_debug.hpp"

namespace big
{

	void render_local_creator_popup_content()
	{
		static int base_address            = 0;
		static bool freeze                 = false;
		static char name[32]               = "";
		static char script_thread_name[32] = "";
		static int(*offsets)[2]            = nullptr;
		static int offset_count            = 0;
		static int previous_offset_count   = 0;
		components::input_text_with_hint("##local_name", "Name", name, sizeof(name));
		components::input_text_with_hint("##local_script_thread_name", "Script thread", script_thread_name, sizeof(script_thread_name));
		ImGui::Text("Base address");
		ImGui::InputInt("##local_base_address", &base_address);
		ImGui::Text("Offsetcount");
		ImGui::InputInt("##modal_offset_count", &offset_count);

		if (offset_count < 0)
			offset_count = 0;
		else if (offset_count > 10)
			offset_count = 10;

		if (offset_count != previous_offset_count)
		{
			int(*new_offsets)[2] = new int[offset_count][2]{0};
			memcpy(new_offsets, offsets, sizeof(int) * std::min(offset_count, previous_offset_count) * 2);

			delete[] offsets;
			offsets = new_offsets;

			previous_offset_count = offset_count;
		}

		ImGui::PushItemWidth(320.f);
		for (int i = 0; i < offset_count; i++)
		{
			ImGui::PushID(i);

			ImGui::Separator();

			ImGui::Text("DEBUG_GLOBAL_OFFSET"_T.data(), i + 1);
			ImGui::InputInt("##offset", &offsets[i][0]);

			ImGui::Text("DEBUG_GLOBAL_SIZE"_T.data());
			ImGui::SameLine();
			ImGui::InputInt("##size", &offsets[i][1]);

			ImGui::PopID();
		}
		ImGui::PopItemWidth();

		if (components::button("CANCEL"_T))
		{
			strcpy(name, "");
			freeze = false;
			delete[] offsets;
			offsets               = nullptr;
			offset_count          = 0;
			previous_offset_count = 0;

			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (components::button("SAVE"_T))
		{
			if (locals_service::does_script_exist(script_thread_name))
			{
				auto new_local = local(script_thread_name, name, base_address, freeze, offsets, offset_count);
				g_locals_service.m_locals.push_back(new_local);
				strcpy(name, "");
				freeze = false;
				delete[] offsets;
				offsets               = nullptr;
				offset_count          = 0;
				previous_offset_count = 0;

				ImGui::CloseCurrentPopup();
			}
			else
			{
				g_notification_service->push_error("Locals editor", "Script does not exist");
			}
		}
	}

	void debug::locals()
	{
		if (ImGui::BeginTabItem("DEBUG_TAB_LOCALS"_T.data()))
		{
			if (components::button("LOAD"_T))
				g_locals_service.load();
			ImGui::SameLine();
			if (components::button("SAVE"_T))
				g_locals_service.save();

			if (components::button("Add local"))
			{
				ImGui::OpenPopup("Addlocal");
			}

			if (ImGui::BeginPopupModal("Addlocal"))
			{
				render_local_creator_popup_content();

				ImGui::EndPopup();
			}

			for (auto& local : g_locals_service.m_locals)
			{
				ImGui::BeginGroup();
				ImGui::PushID(local.get_id());

				ImGui::Text("%s : %s", local.m_name, local.m_script_thread_name);
				//Find the thread among the script threads
				local.m_script_thread = locals_service::find_script_thread(rage::joaat(local.m_script_thread_name));
				//Check whether the specified script is running
				if (local.m_script_thread && locals_service::is_script_thread_running(local.m_script_thread))
				{
					script_local actual_local = script_local(local.m_script_thread, local.m_base_address);

					//Apply offsets to base address
					for (auto offset : local.m_offsets)
					{
						if (offset.m_size > 0)
							actual_local.at(offset.m_offset, offset.m_size);
						else
							actual_local.at(offset.m_offset);
					}

					ImGui::Text("Value");
					ImGui::SetNextItemWidth(200);
					if (ImGui::InputInt("##local_value", actual_local.as<int*>()))
					{
						local.m_value = *actual_local.as<int*>();
					}
					ImGui::SameLine();
					if (ImGui::Checkbox("Freeze", &local.m_freeze))
						local.m_freeze_value = *actual_local.as<int*>();

					if (local.m_freeze)
						*actual_local.as<int*>() = local.m_freeze_value;
				}
				else
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
					ImGui::Text("%s isn't running", local.m_script_thread_name);
					ImGui::PopStyleColor();
				}

				if (components::button("Delete"))
				{
					for (int i = 0; i < g_locals_service.m_locals.size(); i++)
					{
						if (auto& it = g_locals_service.m_locals.at(i); it.get_id() == local.get_id())
						{
							g_locals_service.m_locals.erase(g_locals_service.m_locals.begin() + i);
							break;
						}
					}
				}

				ImGui::PopID();
				ImGui::Separator();
				ImGui::EndGroup();
			}

			ImGui::EndTabItem();
		}
	}
}