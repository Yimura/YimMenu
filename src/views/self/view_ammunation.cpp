#include "gta/weapons.hpp"
#include "services/gta_data/gta_data_service.hpp"
#include "services/persist_weapons/persist_weapons.hpp"
#include "views/view.hpp"
#include "script_mgr.hpp"

namespace big
{
	bool is_weapon_mk2(Hash weapon)
	{
		switch (weapon)
		{
		case "WEAPON_PISTOL_MK2"_J:
		case "WEAPON_SMG_MK2"_J:
		case "WEAPON_ASSAULTRIFLE_MK2"_J:
		case "WEAPON_CARBINERIFLE_MK2"_J:
		case "WEAPON_COMBATMG_MK2"_J:
		case "WEAPON_HEAVYSNIPER_MK2"_J:
		case "WEAPON_PUMPSHOTGUN_MK2"_J:
		case "WEAPON_SPECIALCARBINE_MK2"_J:
		case "WEAPON_SNSPISTOL_MK2"_J:
		case "WEAPON_MARKSMANRIFLE_MK2"_J:
		case "WEAPON_REVOLVER_MK2"_J:
		case "WEAPON_BULLPUPRIFLE_MK2"_J:
			return true;
		}
		return false;
	}

	void view::ammunation()
	{
		static std::string selected_weapon     = "SELECT"_T.data();
		static std::string selected_attachment = "SELECT"_T.data();
		static Hash selected_weapon_hash       = 0;
		static Hash selected_attachment_hash   = 0;
		static int selected_tint               = 0;
		static char search_weapon[64];

		static std::vector<std::string> default_tints;
		static std::vector<std::string> mk2_tints;
		if (default_tints.empty() && g_script_mgr.can_tick())
		{
			g_fiber_pool->queue_job([] {
				for (int i = 0; i <= 7; i++)
				{
					std::string gxt  = std::string("WM_TINT") + std::to_string(i);
					std::string tint = HUD::GET_FILENAME_FOR_AUDIO_CONVERSATION(gxt.c_str());
					default_tints.push_back(tint);
				}
			});
		}
		if (mk2_tints.empty() && g_script_mgr.can_tick())
		{
			g_fiber_pool->queue_job([] {
				for (int i = 0; i <= 31; i++)
				{
					std::string gxt  = std::string("WCT_TINT_") + std::to_string(i);
					std::string tint = HUD::GET_FILENAME_FOR_AUDIO_CONVERSATION(gxt.c_str());
					mk2_tints.push_back(tint);
				}
			});
		}

		ImGui::BeginCombo("GUI_TAB_WEAPONS"_T.data(), selected_weapon.c_str());
		if (ImGui::IsItemActive() && !ImGui::IsPopupOpen("##weapons_popup"))
		{
			ImGui::OpenPopup("##weapons_popup");
			memset(search_weapon, 0, sizeof(search_weapon));
		}
		if (ImGui::BeginPopup("##weapons_popup", ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			std::map<std::string, weapon_item> sorted_map;
			for (const auto& [_, weapon] : g_gta_data_service.weapons())
			{
				sorted_map.emplace(weapon.m_display_name, weapon);
			}

			ImGui::Text("SEARCH"_T.data());
			ImGui::SameLine();
			ImGui::SetNextItemWidth(250.f);
			components::input_text("##search_weapon", search_weapon, sizeof(search_weapon));

			std::string search_lower = search_weapon;
			std::transform(search_lower.begin(), search_lower.end(), search_lower.begin(), ::tolower);

			bool found_match = false;
			for (const auto& weapon : sorted_map)
			{
				auto weapon_name = weapon.second.m_display_name;
				auto weapon_hash = weapon.second.m_hash;

				if (weapon_name == "NULL" || weapon_hash == NULL)
					continue;

				std::string weapon_lower = weapon_name;
				std::transform(weapon_lower.begin(), weapon_lower.end(), weapon_lower.begin(), ::tolower);

				if (weapon_lower.find(search_lower) != std::string::npos)
				{
					found_match = true;
					if (ImGui::Selectable(weapon_name.c_str()))
					{
						selected_weapon          = weapon_name;
						selected_weapon_hash     = weapon_hash;
						selected_attachment      = "SELECT"_T.data();
						selected_attachment_hash = 0;
						selected_tint            = 0;
					}
					if (ImGui::IsItemHovered() && !weapon.second.m_display_desc.empty())
					{
						ImGui::BeginTooltip();
						ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35);
						ImGui::TextUnformatted(weapon.second.m_display_desc.c_str());
						ImGui::PopTextWrapPos();
						ImGui::EndTooltip();
					}
				}
			}
			if (!found_match)
				ImGui::Text("NO_WEAPON_IN_REGISTRY"_T.data());
			ImGui::EndPopup();
		}
		components::button("VIEW_WEAPON_GIVE_WEAPON"_T, [] {
			WEAPON::GIVE_WEAPON_TO_PED(self::ped, selected_weapon_hash, 9999, false, true);
		});
		ImGui::SameLine();
		components::button("VIEW_WEAPON_REMOVE_WEAPON"_T, [] {
			WEAPON::REMOVE_WEAPON_FROM_PED(self::ped, selected_weapon_hash);
		});

		if (ImGui::BeginCombo("VIEW_WEAPON_ATTACHMENTS"_T.data(), selected_attachment.c_str()))
		{
			weapon_item weapon = g_gta_data_service.weapon_by_hash(selected_weapon_hash);
			if (!weapon.m_attachments.empty())
			{
				for (std::string attachment : weapon.m_attachments)
				{
					weapon_component attachment_component = g_gta_data_service.weapon_component_by_name(attachment);
					std::string attachment_name           = attachment_component.m_display_name;
					Hash attachment_hash                  = attachment_component.m_hash;

					if (attachment_name == "NULL" || attachment_hash == NULL)
						continue;

					bool is_selected = attachment_hash == selected_attachment_hash;
					ImGui::PushID(attachment_hash);
					if (ImGui::Selectable(attachment_name.c_str(), is_selected, ImGuiSelectableFlags_None))
					{
						selected_attachment      = attachment_name;
						selected_attachment_hash = attachment_hash;
					}
					if (ImGui::IsItemHovered() && !attachment_component.m_display_desc.empty())
					{
						ImGui::SetTooltip(attachment_component.m_display_desc.c_str());
					}
					if (is_selected)
					{
						ImGui::SetItemDefaultFocus();
					}
					ImGui::PopID();
				}
			}
			else
			{
				ImGui::Text("NO_WEAPON_ATTACHMENT_IN_REGISTRY"_T.data());
			}
			ImGui::EndCombo();
		}
		components::button("VIEW_WEAPON_ADD_TO_WEAPON"_T, [] {
			WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(self::ped, selected_weapon_hash, selected_attachment_hash);
		});
		ImGui::SameLine();
		components::button("VIEW_WEAPON_REMOVE_TO_WEAPON"_T, [] {
			WEAPON::REMOVE_WEAPON_COMPONENT_FROM_PED(self::ped, selected_weapon_hash, selected_attachment_hash);
		});

		if ((!default_tints.empty() && !mk2_tints.empty()) && selected_weapon_hash != NULL)
		{
			if (is_weapon_mk2(selected_weapon_hash))
			{
				if (ImGui::BeginCombo("VIEW_WEAPON_TINTS"_T.data(), mk2_tints[selected_tint].data()))
				{
					for (int index = 0; index < mk2_tints.size(); index++)
					{
						if (ImGui::Selectable(mk2_tints[index].c_str()))
						{
							selected_tint = index;
						}
					}
					ImGui::EndCombo();
				}
			}
			else
			{
				if (ImGui::BeginCombo("VIEW_WEAPON_TINTS"_T.data(), default_tints[selected_tint].data()))
				{
					for (int index = 0; index < default_tints.size(); index++)
					{
						if (ImGui::Selectable(default_tints[index].c_str()))
						{
							selected_tint = index;
						}
					}
					ImGui::EndCombo();
				}
			}
			components::button("APPLY"_T, [] {
				WEAPON::SET_PED_WEAPON_TINT_INDEX(self::ped, selected_weapon_hash, selected_tint);
			});
		}

		ImGui::SeparatorText("VIEW_WEAPON_PERSIST_WEAPONS"_T.data());

		static std::string selected_loadout = g.persist_weapons.weapon_loadout_file;
		static std::string input_file_name;

		ImGui::PushID(1);
		ImGui::Checkbox("ENABLED"_T.data(), &g.persist_weapons.enabled);
		ImGui::PopID();

		ImGui::BeginGroup();
		ImGui::Text("VIEW_WEAPON_PERSIST_WEAPONS_SAVED_LOADOUTS"_T.data());
		if (ImGui::BeginListBox("##saved_loadouts", ImVec2(200, 200)))
		{
			for (std::string filename : persist_weapons::list_weapon_loadouts())
			{
				if (components::selectable(filename, filename == selected_loadout))
				{
					selected_loadout = filename;
				}
			}
			ImGui::EndListBox();
		}
		ImGui::EndGroup();
		ImGui::SameLine();
		ImGui::BeginGroup();
		ImGui::Text("VIEW_WEAPON_PERSIST_WEAPONS_WEAPON_LOADOUT_FILENAME"_T.data());
		components::input_text_with_hint("##loadout_filename", "VIEW_WEAPON_PERSIST_WEAPONS_LOADOUT_NAME"_T, input_file_name);
		components::button("VIEW_WEAPON_PERSIST_WEAPONS_SAVE"_T, [] {
			persist_weapons::save_weapons(input_file_name);
			input_file_name.clear();
		});
		ImGui::SameLine();
		components::button("VIEW_WEAPON_PERSIST_WEAPONS_LOAD"_T, [] {
			persist_weapons::give_player_loadout(selected_loadout);
		});
		ImGui::SameLine();
		components::button("VIEW_WEAPON_PERSIST_WEAPONS_SET_LOADOUT"_T, [] {
			persist_weapons::set_weapon_loadout(selected_loadout);
		});
		ImGui::Text(std::format("{}: {}:", "VIEW_WEAPON_PERSIST_WEAPONS_CURRENT_LOADOUT"_T, g.persist_weapons.weapon_loadout_file).data());
		ImGui::EndGroup();

		ImGui::SeparatorText("VIEW_WEAPON_WEAPON_HOTKEYS"_T.data());

		static int selected_key = 0;
		const char* const keys[]{"1", "2", "3", "4", "5", "6"};

		ImGui::PushID(2);
		ImGui::Checkbox("ENABLED"_T.data(), &g.weapons.enable_weapon_hotkeys);
		ImGui::PopID();
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("VIEW_WEAPON_WEAPON_HOTKEYS_TOOLTIP"_T.data());
		}

		ImGui::Combo("VIEW_WEAPON_WEAPON_HOTKEYS_KEY"_T.data(), &selected_key, keys, IM_ARRAYSIZE(keys));
		if (!g.weapons.weapon_hotkeys[selected_key].empty())
		{
			int counter = 0;
			for (auto& weapon_hash : g.weapons.weapon_hotkeys[selected_key])
			{
				weapon_item weapon = g_gta_data_service.weapon_by_hash(weapon_hash);
				ImGui::PushID(counter);
				if (ImGui::BeginCombo("GUI_TAB_WEAPONS"_T.data(), weapon.m_display_name.c_str()))
				{
					std::map<std::string, weapon_item> sorted_map;
					for (const auto& [_, weapon_iter] : g_gta_data_service.weapons())
					{
						sorted_map.emplace(weapon_iter.m_display_name, weapon_iter);
					}

					for (const auto& [_, weapon_iter] : g_gta_data_service.weapons())
					{
						if (weapon_iter.m_display_name == "NULL")
							continue;

						bool is_selected = weapon_iter.m_hash == weapon_hash;
						if (ImGui::Selectable(weapon_iter.m_display_name.c_str(), is_selected, ImGuiSelectableFlags_None))
						{
							weapon_hash = weapon_iter.m_hash;
						}
						if (is_selected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
				ImGui::SameLine();
				components::button("VIEW_WEAPON_WEAPON_HOTKEYS_SET_TO_CURRENT_WEAPON"_T, [&weapon_hash] {
					WEAPON::GET_CURRENT_PED_WEAPON(self::ped, &weapon_hash, NULL);
					if (weapon_hash == NULL)
					{
						WEAPON::GET_CURRENT_PED_VEHICLE_WEAPON(self::ped, &weapon_hash);
					}
				});
				ImGui::SameLine();
				if (ImGui::Button("VIEW_WEAPON_REMOVE_WEAPON"_T.data()))
				{
					g.weapons.weapon_hotkeys[selected_key].erase(g.weapons.weapon_hotkeys[selected_key].begin() + counter);
				}
				ImGui::PopID();
				counter++;
			}
		}
		if (ImGui::Button("VIEW_WEAPON_WEAPON_ADD_WEAPON"_T.data()))
		{
			g.weapons.weapon_hotkeys[selected_key].push_back(WEAPON_UNARMED);
		}
	}
}