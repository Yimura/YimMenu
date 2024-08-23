#include "core/data/lsc_types.hpp"
#include "fiber_pool.hpp"
#include "natives.hpp"
#include "script.hpp"
#include "script_function.hpp"
#include "util/vehicle.hpp"
#include "views/view.hpp"

#include <imgui_internal.h>

namespace big
{
	void view::lsc()
	{
		static Vehicle player_vehicle = 0;
		static bool ready             = true;

		static Hash model;
		static std::map<int, int32_t> owned_mods;
		static std::map<int, std::string> slot_display_names;
		static std::map<int, std::map<int, std::string>> mod_display_names;
		static std::map<std::string, std::vector<int>> front_wheel_map;
		static std::map<std::string, std::vector<int>> rear_wheel_map;

		static int selected_slot         = -1;
		static bool is_bennys            = false;
		static bool has_clan_logo        = false; 
		static bool vehicle_cannot_accept_clan_logo = false; 
		static int front_wheel_stock_mod = -1;
		static int rear_wheel_stock_mod  = -1;

		if (self::veh == 0 || player_vehicle != self::veh)
		{
			if (self::veh == 0)
			{
				owned_mods.clear();
				slot_display_names.clear();
				mod_display_names.clear();
				front_wheel_map.clear();
				rear_wheel_map.clear();
				player_vehicle = 0;

				selected_slot = -1;
				ImGui::Text("PLEASE_ENTER_A_VEHICLE"_T.data());
				return;
			}
		}

		if (player_vehicle != self::veh && ready == true)
		{
			ready          = false;
			player_vehicle = self::veh;

			g_fiber_pool->queue_job([] {
				if (!HUD::HAS_THIS_ADDITIONAL_TEXT_LOADED("MOD_MNU", 10))
				{
					HUD::CLEAR_ADDITIONAL_TEXT(10, TRUE);
					HUD::REQUEST_ADDITIONAL_TEXT("MOD_MNU", 10);
					script::get_current()->yield();
				}

				VEHICLE::SET_VEHICLE_MOD_KIT(player_vehicle, 0);

				model = ENTITY::GET_ENTITY_MODEL(player_vehicle);

				owned_mods = vehicle::get_owned_mods_from_vehicle(player_vehicle);
				VEHICLE::SET_VEHICLE_MOD_KIT(player_vehicle, 0);

				std::map<int, std::string> tmp_slot_display_names;
				std::map<int, std::map<int, std::string>> tmp_mod_display_names;
				std::map<std::string, std::vector<int>> tmp_front_wheel_map;
				std::map<std::string, std::vector<int>> tmp_rear_wheel_map;

				tmp_slot_display_names[MOD_PLATE_STYLE] = "PLATE_STYLE"_T.data();
				tmp_slot_display_names[MOD_WINDOW_TINT] = "WINDOW_TINT"_T.data();
				tmp_slot_display_names[MOD_WHEEL_TYPE]  = "WHEEL_TYPE"_T.data();

				tmp_mod_display_names[MOD_PLATE_STYLE].insert(lsc_plate_styles.begin(), lsc_plate_styles.end());
				tmp_mod_display_names[MOD_WINDOW_TINT].insert(lsc_window_tint_types.begin(), lsc_window_tint_types.end());
				tmp_mod_display_names[MOD_WHEEL_TYPE].insert(lsc_wheel_styles.begin(), lsc_wheel_styles.end());

				is_bennys     = owned_mods[MOD_WHEEL_TYPE] == WHEEL_TYPE_BENNYS_ORIGINAL || owned_mods[MOD_WHEEL_TYPE] == WHEEL_TYPE_BENNYS_BESPOKE || owned_mods[MOD_WHEEL_TYPE] == WHEEL_TYPE_OPEN_WHEEL || owned_mods[MOD_WHEEL_TYPE] == WHEEL_TYPE_STREET || owned_mods[MOD_WHEEL_TYPE] == WHEEL_TYPE_TRACK;
				has_clan_logo = GRAPHICS::DOES_VEHICLE_HAVE_CREW_EMBLEM(player_vehicle, 0);
				vehicle_cannot_accept_clan_logo = scr_functions::vehicle_cannot_accept_clan_logo.call<bool>(player_vehicle);

				for (int slot = MOD_SPOILERS; slot <= MOD_LIGHTBAR; slot++)
				{
					int count = VEHICLE::GET_NUM_VEHICLE_MODS(player_vehicle, slot);
					if (count > 0)
					{
						int owner_mod = owned_mods[slot];

						std::string slot_name = vehicle_helper::get_mod_slot_name(model, player_vehicle, slot);
						if (slot_name.empty())
						{
							continue;
						}
						slot_name.append("##");
						slot_name.append(std::to_string(slot));
						tmp_slot_display_names[slot] = slot_name;

						std::map<int, std::string> mod_names;

						for (int mod = -1; mod < count; mod++)
						{
							if (vehicle_helper::check_mod_blacklist(model, slot, mod))
							{
								continue;
							}

							bool is_repeated = false;

							std::string mod_name = vehicle_helper::get_mod_name(model, player_vehicle, slot, mod, count);

							if (mod_name.empty() || mod_name == "NULL")
							{
								continue;
							}

							if (slot == MOD_FRONTWHEEL)
							{
								if (is_bennys)
								{
									if (mod_name.rfind("LSC_CHROME"_T.data(), 0) == 0)
									{
										std::string new_mod_name = mod_name.substr(7);

										if (tmp_front_wheel_map[new_mod_name].size() > 0)
										{
											mod_name = new_mod_name;
										}
									}
								}

								tmp_front_wheel_map[mod_name].push_back(mod);

								if (mod == owner_mod)
								{
									front_wheel_stock_mod = tmp_front_wheel_map[mod_name][0];
								}

								if (tmp_front_wheel_map[mod_name].size() > 1)
								{
									is_repeated = true;
								}
							}
							else if (slot == MOD_REARWHEEL)
							{
								if (is_bennys)
								{
									if (mod_name.rfind("LSC_CHROME"_T.data(), 0) == 0)
									{
										std::string new_mod_name = mod_name.substr(7);

										if (tmp_rear_wheel_map[new_mod_name].size() > 0)
										{
											mod_name = new_mod_name;
										}
									}
								}

								tmp_rear_wheel_map[mod_name].push_back(mod);

								if (mod == owner_mod)
								{
									rear_wheel_stock_mod = tmp_rear_wheel_map[mod_name][0];
								}

								if (tmp_rear_wheel_map[mod_name].size() > 1)
								{
									is_repeated = true;
								}
							}

							if (!is_repeated)
							{
								mod_names[mod] = mod_name;
							}
						}

						tmp_mod_display_names[slot] = mod_names;
					}
				}

				if (tmp_mod_display_names.count(MOD_HORNS) > 0)
				{
					tmp_mod_display_names[MOD_HORNS].insert(lsc_missing_horns.begin(), lsc_missing_horns.end());
				}

				slot_display_names = tmp_slot_display_names;
				mod_display_names  = tmp_mod_display_names;
				front_wheel_map    = tmp_front_wheel_map;
				rear_wheel_map     = tmp_rear_wheel_map;

				ready = true;
			});
		}

		if (components::button("MAX_VEHICLE"_T))
		{
			g_fiber_pool->queue_job([] {
				vehicle::max_vehicle(self::veh);

				// refresh mod names
				player_vehicle = 0;
			});
		}

		ImGui::SameLine();
		if (components::button("MAX_VEHICLE_PERFORMANCE"_T))
		{
			g_fiber_pool->queue_job([] {
				vehicle::max_vehicle_performance(self::veh);

				// refresh mod names
				player_vehicle = 0;
			});
		}

		ImGui::Separator();

		static char plate[9];

		ImGui::SetNextItemWidth(200.f);
		components::input_text_with_hint("##plate", "PLATE_NUMBER"_T, plate, sizeof(plate), ImGuiInputTextFlags_None);
		ImGui::SameLine();
		if (components::button("CHANGE_PLATE_NUMBER"_T))
		{
			g_fiber_pool->queue_job([] {
				vehicle::set_plate(self::veh, plate);
			});
		}

		ImGui::SeparatorText("MOD_OPTIONS"_T.data());

		bool is_bulletproof_tires = !owned_mods[MOD_TIRE_CAN_BURST];
		if (ImGui::Checkbox("BULLETPROOF_TIRES"_T.data(), (bool*)&is_bulletproof_tires))
		{
			g_fiber_pool->queue_job([is_bulletproof_tires] {
				owned_mods[MOD_TIRE_CAN_BURST] = (int32_t)!is_bulletproof_tires;
				VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(player_vehicle, owned_mods[MOD_TIRE_CAN_BURST]);
			});
		}

		ImGui::SameLine();
		if (ImGui::Checkbox("LOW_GRIP_TIRES"_T.data(), (bool*)&owned_mods[MOD_DRIFT_TIRE]))
		{
			g_fiber_pool->queue_job([] {
				VEHICLE::SET_DRIFT_TYRES(player_vehicle, owned_mods[MOD_DRIFT_TIRE]);
			});
		}

		ImGui::SameLine();
		if (ImGui::Checkbox("TURBO"_T.data(), (bool*)&owned_mods[MOD_TURBO]))
		{
			g_fiber_pool->queue_job([] {
				VEHICLE::TOGGLE_VEHICLE_MOD(player_vehicle, MOD_TURBO, owned_mods[MOD_TURBO]);
			});
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("TIRE_SMOKE"_T.data(), (bool*)&owned_mods[MOD_TYRE_SMOKE]))
		{
			g_fiber_pool->queue_job([] {
				VEHICLE::TOGGLE_VEHICLE_MOD(player_vehicle, MOD_TYRE_SMOKE, owned_mods[MOD_TYRE_SMOKE]);
			});
		}
		if (!vehicle_cannot_accept_clan_logo)
		{
			if (ImGui::Checkbox("CLAN_LOGO"_T.data(), &has_clan_logo))
			{
				g_fiber_pool->queue_job([] {
					if (has_clan_logo)
					{
						scr_functions::add_clan_logo_to_vehicle.call<bool>(&player_vehicle, self::id);
					}
					else
					{
						GRAPHICS::REMOVE_VEHICLE_CREW_EMBLEM(player_vehicle, 0);
						GRAPHICS::REMOVE_VEHICLE_CREW_EMBLEM(player_vehicle, 1);
					}
				});
			}
		}

		ImGui::SeparatorText("VIEW_LSC_MOD_SLOTS"_T.data());

		ImGui::BeginGroup();

		components::sub_title("SLOT"_T);
		if (ImGui::BeginListBox("##slot", ImVec2(200, 200)))
		{
			for (const auto& [slot, name] : slot_display_names)
			{
				if (ImGui::Selectable(name.c_str(), slot == selected_slot))
				{
					selected_slot = slot;
				}
			}
			ImGui::EndListBox();
		}
		ImGui::EndGroup();

		if (selected_slot != -1)
		{
			auto wheel_stock_mod = &front_wheel_stock_mod;
			auto wheel_custom    = &owned_mods[MOD_FRONTWHEEL_VAR];
			bool is_wheel_mod    = false;

			if (selected_slot == MOD_FRONTWHEEL)
			{
				is_wheel_mod = true;
			}
			else if (selected_slot == MOD_REARWHEEL)
			{
				wheel_stock_mod = &rear_wheel_stock_mod;
				wheel_custom    = &owned_mods[MOD_REARWHEEL_VAR];
				is_wheel_mod    = true;
			}
			else
			{
				is_wheel_mod = false;
			}

			ImGui::SameLine();
			ImGui::BeginGroup();

			components::sub_title("MOD"_T);
			if (ImGui::BeginListBox("##mod", ImVec2(240, 200)))
			{
				for (const auto& it : mod_display_names[selected_slot])
				{
					const auto& mod  = it.first;
					const auto& name = it.second;

					bool item_selected = mod == owned_mods[selected_slot];

					if (is_wheel_mod)
					{
						item_selected = mod == *wheel_stock_mod;
					}

					if (ImGui::Selectable(name.c_str(), item_selected))
					{
						g_fiber_pool->queue_job([&mod, is_wheel_mod, wheel_stock_mod, wheel_custom] {
							entity::take_control_of(self::veh);

							if (selected_slot >= 0)
							{
								if (!VEHICLE::IS_VEHICLE_MOD_GEN9_EXCLUSIVE(player_vehicle, selected_slot, mod))
								{
									VEHICLE::SET_VEHICLE_MOD(player_vehicle, selected_slot, mod, false);
									owned_mods[selected_slot] = mod;

									if (is_wheel_mod)
									{
										*wheel_stock_mod = mod;
										*wheel_custom    = false;
									}
								}
								else
								{
									g_notification_service.push_error("GUI_TAB_LSC"_T.data(), "VIEW_LSC_SELECTED_MOD_IS_INVALID"_T.data());
								}
							}
							else if (selected_slot == MOD_WINDOW_TINT)
							{
								VEHICLE::SET_VEHICLE_WINDOW_TINT(player_vehicle, mod);
								owned_mods[selected_slot] = mod;
							}
							else if (selected_slot == MOD_WHEEL_TYPE)
							{
								VEHICLE::SET_VEHICLE_WHEEL_TYPE(player_vehicle, mod);
								VEHICLE::SET_VEHICLE_MOD(player_vehicle, MOD_FRONTWHEEL, 0, false);
								VEHICLE::SET_VEHICLE_MOD(player_vehicle, MOD_REARWHEEL, 0, false);
								player_vehicle = 0;
							}
							else if (selected_slot == MOD_PLATE_STYLE)
							{
								VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(player_vehicle, mod);
								owned_mods[selected_slot] = mod;
							}
						});
					}
				}
				ImGui::EndListBox();
			}

			ImGui::EndGroup();

			if (is_wheel_mod && *wheel_stock_mod != -1)
			{
				auto wheel_map = front_wheel_map;

				if (selected_slot == MOD_REARWHEEL)
				{
					wheel_map = rear_wheel_map;
				}

				ImGui::SameLine();
				ImGui::BeginGroup();

				components::sub_title("STYLE"_T);
				if (ImGui::BeginListBox("##style", ImVec2(200, 200)))
				{
					std::string mod_name = mod_display_names[selected_slot][*wheel_stock_mod];
					auto wheel_mods      = wheel_map[mod_name];

					for (int i = 0; i < wheel_mods.size(); i++)
					{
						int& mod = wheel_mods[i];

						int should_custom = false;

						// bennys fix
						if (!is_bennys)
						{
							if (i == 0)
							{
								if (ImGui::Selectable("STOCK"_T.data(), mod == owned_mods[selected_slot] && *wheel_custom == false))
								{
									g_fiber_pool->queue_job([&mod] {
										VEHICLE::SET_VEHICLE_MOD(player_vehicle, selected_slot, mod, false);
										player_vehicle = 0;
									});
								}
							}

							should_custom = true;
						}

						std::string label = "LSC_STYLE"_T.data() + std::to_string(mod);
						if (ImGui::Selectable(label.c_str(), mod == owned_mods[selected_slot] && *wheel_custom == should_custom))
						{
							g_fiber_pool->queue_job([&mod, should_custom] {
								VEHICLE::SET_VEHICLE_MOD(player_vehicle, selected_slot, mod, should_custom);
								player_vehicle = 0;
							});
						}
					}
					ImGui::EndListBox();
				}

				ImGui::EndGroup();
			}
		}

		int item_counter = 0;
		for (int extra = MOD_EXTRA_1; extra >= MOD_EXTRA_14; extra--)
		{
			if (owned_mods.find(extra) != owned_mods.end())
			{
				if (item_counter == 0)
				{
					ImGui::SeparatorText("VIEW_LSC_VEHICLE_EXTRAS"_T.data());
					ImGui::BeginGroup();
				}
				if ((item_counter % 5) != 0)
					ImGui::SameLine();
				int gta_extra_id      = (extra - MOD_EXTRA_1) * -1;
				auto name             = std::format("{}: #{}", "VIEW_LSC_EXTRAS"_T, gta_extra_id);
				bool is_extra_enabled = owned_mods[extra] == 1;
				if (ImGui::Checkbox(name.c_str(), &is_extra_enabled))
				{
					owned_mods[extra] = is_extra_enabled;
					g_fiber_pool->queue_job([gta_extra_id, is_extra_enabled] {
						VEHICLE::SET_VEHICLE_EXTRA(player_vehicle, gta_extra_id, !is_extra_enabled);
					});
				}
				item_counter++;
			}
		}
		if (item_counter != 0)
		{
			ImGui::EndGroup();
		}

		ImGui::SeparatorText("NEON_LIGHT_OPTIONS"_T.data());

		ImGui::PushID("##headlight_en");
		if (ImGui::Checkbox("HEADLIGHT"_T.data(), (bool*)&owned_mods[MOD_XENON_LIGHTS]))
		{
			g_fiber_pool->queue_job([] {
				VEHICLE::TOGGLE_VEHICLE_MOD(player_vehicle, MOD_XENON_LIGHTS, owned_mods[MOD_XENON_LIGHTS]);
			});
		}
		ImGui::PopID();
		ImGui::SameLine();
		if (ImGui::Checkbox("LEFT"_T.data(), (bool*)&owned_mods[MOD_NEON_LEFT_ON]))
		{
			g_fiber_pool->queue_job([] {
				VEHICLE::SET_VEHICLE_NEON_ENABLED(player_vehicle, NEON_LEFT, owned_mods[MOD_NEON_LEFT_ON]);
			});
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("RIGHT"_T.data(), (bool*)&owned_mods[MOD_NEON_RIGHT_ON]))
		{
			g_fiber_pool->queue_job([] {
				VEHICLE::SET_VEHICLE_NEON_ENABLED(player_vehicle, NEON_RIGHT, owned_mods[MOD_NEON_RIGHT_ON]);
			});
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("FRONT"_T.data(), (bool*)&owned_mods[MOD_NEON_FRONT_ON]))
		{
			g_fiber_pool->queue_job([] {
				VEHICLE::SET_VEHICLE_NEON_ENABLED(player_vehicle, NEON_FRONT, owned_mods[MOD_NEON_FRONT_ON]);
			});
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("BACK"_T.data(), (bool*)&owned_mods[MOD_NEON_BACK_ON]))
		{
			g_fiber_pool->queue_job([] {
				VEHICLE::SET_VEHICLE_NEON_ENABLED(player_vehicle, NEON_BACK, owned_mods[MOD_NEON_BACK_ON]);
			});
		}
		ImGui::SameLine();
		ImGui::PushID("##neon_check_all");
		components::button("CHECK_ALL"_T, [] {
			owned_mods[MOD_XENON_LIGHTS]  = true;
			owned_mods[MOD_NEON_LEFT_ON]  = true;
			owned_mods[MOD_NEON_RIGHT_ON] = true;
			owned_mods[MOD_NEON_FRONT_ON] = true;
			owned_mods[MOD_NEON_BACK_ON]  = true;

			VEHICLE::TOGGLE_VEHICLE_MOD(player_vehicle, MOD_XENON_LIGHTS, owned_mods[MOD_XENON_LIGHTS]);
			VEHICLE::SET_VEHICLE_NEON_ENABLED(player_vehicle, NEON_LEFT, owned_mods[MOD_NEON_LEFT_ON]);
			VEHICLE::SET_VEHICLE_NEON_ENABLED(player_vehicle, NEON_RIGHT, owned_mods[MOD_NEON_RIGHT_ON]);
			VEHICLE::SET_VEHICLE_NEON_ENABLED(player_vehicle, NEON_FRONT, owned_mods[MOD_NEON_FRONT_ON]);
			VEHICLE::SET_VEHICLE_NEON_ENABLED(player_vehicle, NEON_BACK, owned_mods[MOD_NEON_BACK_ON]);
		});
		ImGui::PopID();
		ImGui::SameLine();
		ImGui::PushID("##neon_uncheck_all");
		components::button("UNCHECK_ALL"_T, [] {
			owned_mods[MOD_XENON_LIGHTS]  = false;
			owned_mods[MOD_NEON_LEFT_ON]  = false;
			owned_mods[MOD_NEON_RIGHT_ON] = false;
			owned_mods[MOD_NEON_FRONT_ON] = false;
			owned_mods[MOD_NEON_BACK_ON]  = false;

			VEHICLE::TOGGLE_VEHICLE_MOD(player_vehicle, MOD_XENON_LIGHTS, owned_mods[MOD_XENON_LIGHTS]);
			VEHICLE::SET_VEHICLE_NEON_ENABLED(player_vehicle, NEON_LEFT, owned_mods[MOD_NEON_LEFT_ON]);
			VEHICLE::SET_VEHICLE_NEON_ENABLED(player_vehicle, NEON_RIGHT, owned_mods[MOD_NEON_RIGHT_ON]);
			VEHICLE::SET_VEHICLE_NEON_ENABLED(player_vehicle, NEON_FRONT, owned_mods[MOD_NEON_FRONT_ON]);
			VEHICLE::SET_VEHICLE_NEON_ENABLED(player_vehicle, NEON_BACK, owned_mods[MOD_NEON_BACK_ON]);
		});
		ImGui::PopID();

		ImGui::SeparatorText("COLOR_OPTIONS"_T.data());

		static int color_to_change = 0;
		static int color_type      = 8;

		if ((color_to_change == 7 && !owned_mods[MOD_XENON_LIGHTS]) || (color_to_change == 5 && !owned_mods[MOD_TYRE_SMOKE]))
		{
			color_to_change = 0;
			color_type      = 8;
		}

		if (ImGui::BeginListBox("##color_options", ImVec2(120, 254)))
		{
			if (ImGui::Selectable("PRIMARY"_T.data(), color_to_change == 0, ImGuiSelectableFlags_SelectOnClick))
			{
				color_to_change = 0;
			}

			if (ImGui::Selectable("SECONDARY"_T.data(), color_to_change == 1))
			{
				color_to_change = 1;
			}

			if (ImGui::Selectable("PEARLESCENT"_T.data(), color_to_change == 2))
			{
				color_to_change = 2;
				color_type      = 6;
			}

			if (ImGui::Selectable("INTERIOR"_T.data(), color_to_change == 3))
			{
				color_to_change = 3;
				color_type      = 9;
			}

			if (ImGui::Selectable("DASHBOARD"_T.data(), color_to_change == 4))
			{
				color_to_change = 4;
				color_type      = 10;
			}

			if (!owned_mods[MOD_TYRE_SMOKE])
			{
				ImGui::BeginDisabled();
			}
			if (ImGui::Selectable("TIRE_SMOKE"_T.data(), color_to_change == 5))
			{
				color_to_change = 5;
				color_type      = 8;
			}
			if (!owned_mods[MOD_TYRE_SMOKE])
			{
				ImGui::EndDisabled();
			}

			if (ImGui::Selectable("WHEEL_COLOR"_T.data(), color_to_change == 6))
			{
				color_to_change = 6;
				color_type      = 7;
			}

			if (!owned_mods[MOD_XENON_LIGHTS])
			{
				ImGui::BeginDisabled();
			}
			ImGui::PushID("##headlight_col");
			if (ImGui::Selectable("HEADLIGHT"_T.data(), color_to_change == 7))
			{
				color_to_change = 7;
				color_type      = 11;
			}
			ImGui::PopID();
			if (!owned_mods[MOD_XENON_LIGHTS])
			{
				ImGui::EndDisabled();
			}

			if (ImGui::Selectable("NEON"_T.data(), color_to_change == 8))
			{
				color_to_change = 8;
				color_type      = 8;
			}

			ImGui::EndListBox();
		}


		if (color_to_change == 0 || color_to_change == 1)
		{
			if (color_type > 5)
			{
				color_type = 8;
			}

			// primary and secondary color

			ImGui::SameLine();
			if (ImGui::BeginListBox("##colors", ImVec2(140, 254)))
			{
				if (ImGui::Selectable("CUSTOM"_T.data(), color_type == 8, ImGuiSelectableFlags_SelectOnClick))
				{
					color_type = 8;
				}
				if (ImGui::Selectable("REMOVE_CUSTOM"_T.data(), false))
				{
					g_fiber_pool->queue_job([] {
						if (color_to_change == 0)
						{
							VEHICLE::CLEAR_VEHICLE_CUSTOM_PRIMARY_COLOUR(player_vehicle);
						}
						else
						{
							VEHICLE::CLEAR_VEHICLE_CUSTOM_SECONDARY_COLOUR(player_vehicle);
						}
						VEHICLE::SET_VEHICLE_COLOURS(player_vehicle, owned_mods[MOD_PRIMARY_COL], owned_mods[MOD_SECONDARY_COL]);
					});
				}

				if (ImGui::Selectable("CHROME"_T.data(), color_type == 0))
				{
					color_type = 0;
				}
				if (ImGui::Selectable("CLASSIC"_T.data(), color_type == 1))
				{
					color_type = 1;
				}
				if (ImGui::Selectable("MATTE"_T.data(), color_type == 2))
				{
					color_type = 2;
				}
				if (ImGui::Selectable("METALS"_T.data(), color_type == 3))
				{
					color_type = 3;
				}
				if (ImGui::Selectable("UTIL"_T.data(), color_type == 4))
				{
					color_type = 4;
				}
				if (ImGui::Selectable("WORN"_T.data(), color_type == 5))
				{
					color_type = 5;
				}
				ImGui::EndListBox();
			}
		}
		else if (color_to_change == 7)
		{
			ImGui::SameLine();
		}


		if (color_type == 8)
		{
			// custom color

			static float color[3] = {1, 1, 1};
			auto color_r          = &owned_mods[MOD_PRIMARY_COL_R];
			auto color_g          = &owned_mods[MOD_PRIMARY_COL_G];
			auto color_b          = &owned_mods[MOD_PRIMARY_COL_B];

			if (color_to_change == 1)
			{
				color_r = &owned_mods[MOD_SECONDARY_COL_R];
				color_g = &owned_mods[MOD_SECONDARY_COL_G];
				color_b = &owned_mods[MOD_SECONDARY_COL_B];
			}
			else if (color_to_change == 2)
			{
				color_r = &owned_mods[MOD_TIRESMOKE_COL_R];
				color_g = &owned_mods[MOD_TIRESMOKE_COL_G];
				color_b = &owned_mods[MOD_TIRESMOKE_COL_B];
			}
			else if (color_to_change == 3)
			{
				color_r = &owned_mods[MOD_NEON_COL_R];
				color_g = &owned_mods[MOD_NEON_COL_G];
				color_b = &owned_mods[MOD_NEON_COL_B];
			}

			color[0] = (float)*color_r / 255;
			color[1] = (float)*color_g / 255;
			color[2] = (float)*color_b / 255;

			if (color_to_change == 5)
			{
				ImGui::SameLine();
				if (ImGui::BeginListBox("##tire_smoke_rgb", ImVec2(140, 254)))
				{
					for (const auto& it : lsc_tire_smoke_rgb)
					{
						auto& name = it.first;
						auto& rgb  = it.second;

						if (ImGui::Selectable(name.c_str(), false))
						{
							g_fiber_pool->queue_job([&rgb] {
								VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(player_vehicle, rgb[0], rgb[1], rgb[2]);
							});
							*color_r = rgb[0];
							*color_g = rgb[1];
							*color_b = rgb[2];
						}
					}

					ImGui::EndListBox();
				}
			}
			else if (color_to_change == 8)
			{
				ImGui::SameLine();
				if (ImGui::BeginListBox("##neon_rgb", ImVec2(140, 254)))
				{
					for (const auto& it : lsc_neon_rgb)
					{
						auto& name = it.first;
						auto& rgb  = it.second;

						if (ImGui::Selectable(name.c_str(), false))
						{
							g_fiber_pool->queue_job([&rgb] {
								VEHICLE::SET_VEHICLE_NEON_COLOUR(player_vehicle, rgb[0], rgb[1], rgb[2]);
							});
							*color_r = rgb[0];
							*color_g = rgb[1];
							*color_b = rgb[2];
						}
					}

					ImGui::EndListBox();
				}
			}

			ImGui::SameLine();
			ImGui::SetNextItemWidth(214);
			if (ImGui::ColorPicker3("CUSTOM_VEHCOLOR"_T.data(), color, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_DisplayHex))
			{
				*color_r = (int)(color[0] * 255);
				*color_g = (int)(color[1] * 255);
				*color_b = (int)(color[2] * 255);

				g_fiber_pool->queue_job([color_r, color_g, color_b] {
					switch (color_to_change)
					{
					case 0:
						VEHICLE::SET_VEHICLE_CUSTOM_PRIMARY_COLOUR(player_vehicle, *color_r, *color_g, *color_b);
						break;
					case 1:
						VEHICLE::SET_VEHICLE_CUSTOM_SECONDARY_COLOUR(player_vehicle, *color_r, *color_g, *color_b);
						break;
					case 5: VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(player_vehicle, *color_r, *color_g, *color_b); break;
					case 8: VEHICLE::SET_VEHICLE_NEON_COLOUR(player_vehicle, *color_r, *color_g, *color_b); break;
					}
				});
			}
		}
		else
		{
			// standard color

			int selected_color = 0;
			switch (color_type)
			{
			case 6: selected_color = owned_mods[MOD_PEARLESCENT_COL]; break;
			case 7: selected_color = owned_mods[MOD_WHEEL_COL]; break;
			case 9: selected_color = owned_mods[MOD_INTERIOR_COL]; break;
			case 10: selected_color = owned_mods[MOD_DASHBOARD_COL]; break;
			case 11: selected_color = owned_mods[MOD_XENON_COL]; break;
			default:
				selected_color = (color_to_change == 0) ? owned_mods[MOD_PRIMARY_COL] : owned_mods[MOD_SECONDARY_COL];
			}

			if (color_type != 11)
			{
				ImGui::SameLine();
			}

			if (ImGui::BeginListBox("##color", ImVec2(180, 254)))
			{
				switch (color_type)
				{
				case 0: //Chrome
				{
					if (ImGui::Selectable("CHROME"_T.data(), selected_color == COLOR_CHROME))
					{
						if (color_to_change == 0)
						{
							owned_mods[MOD_PRIMARY_COL] = COLOR_CHROME;
						}
						else
						{
							owned_mods[MOD_SECONDARY_COL] = COLOR_CHROME;
						}

						g_fiber_pool->queue_job([] {
							VEHICLE::SET_VEHICLE_COLOURS(player_vehicle, owned_mods[MOD_PRIMARY_COL], owned_mods[MOD_SECONDARY_COL]);
						});
					}
					break;
				}
				case 1: //Classic
				{
					for (const auto& [color, name] : lsc_classic_colors)
					{
						if (ImGui::Selectable(name.c_str(), selected_color == color))
						{
							selected_color = color;

							if (color_to_change == 0)
							{
								owned_mods[MOD_PRIMARY_COL] = color;
							}
							else
							{
								owned_mods[MOD_SECONDARY_COL] = color;
							}

							g_fiber_pool->queue_job([] {
								VEHICLE::SET_VEHICLE_COLOURS(player_vehicle, owned_mods[MOD_PRIMARY_COL], owned_mods[MOD_SECONDARY_COL]);
							});
						}
					}
					break;
				}
				case 2: //Matte
				{
					for (const auto& [color, name] : lsc_matte_colors)
					{
						if (ImGui::Selectable(name.c_str(), selected_color == color))
						{
							selected_color = color;

							if (color_to_change == 0)
							{
								owned_mods[MOD_PRIMARY_COL] = color;
							}
							else
							{
								owned_mods[MOD_SECONDARY_COL] = color;
							}

							g_fiber_pool->queue_job([] {
								VEHICLE::SET_VEHICLE_COLOURS(player_vehicle, owned_mods[MOD_PRIMARY_COL], owned_mods[MOD_SECONDARY_COL]);
							});
						}
					}
					break;
				}
				case 3: //Metals
				{
					for (const auto& [color, name] : lsc_metal_colors)
					{
						if (ImGui::Selectable(name.c_str(), selected_color == color))
						{
							selected_color = color;

							if (color_to_change == 0)
							{
								owned_mods[MOD_PRIMARY_COL] = color;
							}
							else
							{
								owned_mods[MOD_SECONDARY_COL] = color;
							}

							g_fiber_pool->queue_job([] {
								VEHICLE::SET_VEHICLE_COLOURS(player_vehicle, owned_mods[MOD_PRIMARY_COL], owned_mods[MOD_SECONDARY_COL]);
							});
						}
					}
					break;
				}
				case 4: //Util
				{
					for (const auto& [color, name] : lsc_util_colors)
					{
						if (ImGui::Selectable(name.c_str(), selected_color == color))
						{
							selected_color = color;

							if (color_to_change == 0)
							{
								owned_mods[MOD_PRIMARY_COL] = color;
							}
							else
							{
								owned_mods[MOD_SECONDARY_COL] = color;
							}

							g_fiber_pool->queue_job([] {
								VEHICLE::SET_VEHICLE_COLOURS(player_vehicle, owned_mods[MOD_PRIMARY_COL], owned_mods[MOD_SECONDARY_COL]);
							});
						}
					}
					break;
				}	
				case 5: //Worn
				{
					for (const auto& [color, name] : lsc_worn_colors)
					{
						if (ImGui::Selectable(name.c_str(), selected_color == color))
						{
							selected_color = color;

							if (color_to_change == 0)
							{
								owned_mods[MOD_PRIMARY_COL] = color;
							}
							else
							{
								owned_mods[MOD_SECONDARY_COL] = color;
							}

							g_fiber_pool->queue_job([] {
								VEHICLE::SET_VEHICLE_COLOURS(player_vehicle, owned_mods[MOD_PRIMARY_COL], owned_mods[MOD_SECONDARY_COL]);
							});
						}
					}
					break;
				}
				case 6: //Pearlescent
				{
					for (const auto& [color, name] : lsc_classic_colors)
					{
						if (ImGui::Selectable(name.c_str(), selected_color == color))
						{
							selected_color                  = color;
							owned_mods[MOD_PEARLESCENT_COL] = color;

							g_fiber_pool->queue_job([] {
								VEHICLE::SET_VEHICLE_EXTRA_COLOURS(player_vehicle, owned_mods[MOD_PEARLESCENT_COL], owned_mods[MOD_WHEEL_COL]);
							});
						}
					}
					break;
				}
				case 7: //Wheel Color
				{
					for (const auto& [color, name] : lsc_classic_colors)
					{
						if (ImGui::Selectable(name.c_str(), selected_color == color))
						{
							selected_color            = color;
							owned_mods[MOD_WHEEL_COL] = color;

							g_fiber_pool->queue_job([] {
								VEHICLE::SET_VEHICLE_EXTRA_COLOURS(player_vehicle, owned_mods[MOD_PEARLESCENT_COL], owned_mods[MOD_WHEEL_COL]);
							});
						}
					}
					break;
				}
				case 9: //Interior Color
				{
					for (const auto& [color, name] : lsc_classic_colors)
					{
						if (ImGui::Selectable(name.c_str(), selected_color == color))
						{
							selected_color               = color;
							owned_mods[MOD_INTERIOR_COL] = color;

							g_fiber_pool->queue_job([] {
								VEHICLE::SET_VEHICLE_EXTRA_COLOUR_5(player_vehicle, owned_mods[MOD_INTERIOR_COL]);
							});
						}
					}
					break;
				}
				case 10: //Dashboard Color
				{
					for (const auto& [color, name] : lsc_classic_colors)
					{
						if (ImGui::Selectable(name.c_str(), selected_color == color))
						{
							selected_color                = color;
							owned_mods[MOD_DASHBOARD_COL] = color;

							g_fiber_pool->queue_job([] {
								VEHICLE::SET_VEHICLE_EXTRA_COLOUR_6(player_vehicle, owned_mods[MOD_DASHBOARD_COL]);
							});
						}
					}
					break;
				}
				case 11: //Headlight Color
				{
					for (const auto& [color, name] : lsc_headlight_colors)
					{
						if (ImGui::Selectable(name.c_str(), selected_color == color))
						{
							selected_color            = color;
							owned_mods[MOD_XENON_COL] = color;

							g_fiber_pool->queue_job([] {
								VEHICLE::SET_VEHICLE_XENON_LIGHT_COLOR_INDEX(player_vehicle, owned_mods[MOD_XENON_COL]);
							});
						}
					}
					break;
				}
				}

				ImGui::EndListBox();
			}
		}
	}
}
