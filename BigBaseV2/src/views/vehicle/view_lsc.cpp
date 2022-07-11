#include "fiber_pool.hpp"
#include "natives.hpp"
#include "script.hpp"
#include "services/vehicle_helper/vehicle_helper.hpp"
#include "views/view.hpp"
#include "util/vehicle.hpp"
#include "core/data/vehicle_wheel_types.hpp"
#include <imgui_internal.h>

namespace big
{
	void view::lsc()
	{
		static Vehicle player_vehicle = 0;
		static std::vector<std::string> slot_display_names{};
		static std::map<int, int> owned_mods{};
		static std::map<int, std::vector<std::string>> mod_display_names{};
		static int selected_slot = -1;
		static bool can_tires_burst{}, tiresmoke{}, turbo{}, xenon{};
		static int primary_color{}, secondary_color{}, pearlescent{}, wheel_color{}, interior_color{}, dashboard_color{};
		static Hash veh_model_hash = 0;
		static bool is_bike = false;
		static int wheel_type = -1;

		if (self::veh == 0)
		{
			player_vehicle = 0;
			slot_display_names.clear();
			owned_mods.clear();
			mod_display_names.clear();

			ImGui::Text("Please enter a vehicle.");
			return;
		}

		g_fiber_pool->queue_job([] {
			if (player_vehicle != self::veh)
			{
				player_vehicle = 0;
				slot_display_names.clear();
				owned_mods.clear();
				mod_display_names.clear();

				if (!HUD::HAS_THIS_ADDITIONAL_TEXT_LOADED("MOD_MNU", 10))
				{
					HUD::CLEAR_ADDITIONAL_TEXT(10, TRUE);
					HUD::REQUEST_ADDITIONAL_TEXT("MOD_MNU", 10);
					script::get_current()->yield();
				}

				player_vehicle = PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), FALSE);
				can_tires_burst = !VEHICLE::GET_VEHICLE_TYRES_CAN_BURST(player_vehicle);
				tiresmoke = VEHICLE::IS_TOGGLE_MOD_ON(player_vehicle, MOD_TYRE_SMOKE);
				turbo = VEHICLE::IS_TOGGLE_MOD_ON(player_vehicle, MOD_TURBO);
				xenon = VEHICLE::IS_TOGGLE_MOD_ON(player_vehicle, MOD_XENON_LIGHTS);
				veh_model_hash = g_local_player->m_vehicle->m_model_info->m_model_hash;
				wheel_type = VEHICLE::GET_VEHICLE_WHEEL_TYPE(player_vehicle);
				is_bike = VEHICLE::IS_THIS_MODEL_A_BIKE(veh_model_hash) || VEHICLE::IS_THIS_MODEL_A_BICYCLE(veh_model_hash);

				VEHICLE::GET_VEHICLE_COLOURS(player_vehicle, &primary_color, &secondary_color);
				VEHICLE::GET_VEHICLE_EXTRA_COLOURS(player_vehicle, &pearlescent, &wheel_color);
				VEHICLE::GET_VEHICLE_INTERIOR_COLOR_(player_vehicle, &interior_color);
				VEHICLE::GET_VEHICLE_DASHBOARD_COLOR_(player_vehicle, &dashboard_color);

				std::vector<std::string> dsp_names{};
				for (int slot = MOD_SPOILERS; slot <= MOD_LIVERY; slot++)
				{
					int count = VEHICLE::GET_NUM_VEHICLE_MODS(player_vehicle, slot);
					if (count > 0)
					{
						owned_mods[slot] = (VEHICLE::GET_VEHICLE_MOD(player_vehicle, selected_slot) + 1);
						dsp_names.push_back(vehicle_helper::get_mod_slot_name(slot, player_vehicle));

						std::vector<std::string> names;
						for (int mod = -1; mod < count; mod++)
						{
							std::string name = vehicle_helper::get_mod_name(mod, slot, count, player_vehicle);

							// remove repetitive wheel mods
							if (slot == MOD_FRONTWHEEL || slot == MOD_REARWHEEL)
							{
								// exit the loop if the mod is already in the list
								if (names.size() > 0 && names[1] == name)
								{
									break;
								}
							}

							names.push_back(name);
						}
						mod_display_names[slot] = names;
					}
					else
					{
						dsp_names.push_back("");
					}
				}
				slot_display_names = dsp_names;
			}
		});

		if (slot_display_names.empty())
		{
			return;
		}

		components::button("Start LS Customs", [] {
			g->vehicle.ls_customs = true;
		});
		ImGui::SameLine();
		if (components::button("Max Vehicle"))
		{
			g_fiber_pool->queue_job([] {
				vehicle::max_vehicle(self::veh);
			});
		}

		ImGui::Separator();
		components::small_text("Mod Options");

		if (ImGui::Checkbox("Bulletproof Tires", &can_tires_burst))
		{
			g_fiber_pool->queue_job([] {
				VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(player_vehicle, !can_tires_burst);
			});
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Turbo", &turbo))
		{
			g_fiber_pool->queue_job([] {
				VEHICLE::TOGGLE_VEHICLE_MOD(player_vehicle, MOD_TURBO, turbo);
			});
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Tiresmoke", &tiresmoke))
		{
			g_fiber_pool->queue_job([] {
				VEHICLE::TOGGLE_VEHICLE_MOD(player_vehicle, MOD_TYRE_SMOKE, tiresmoke);
			});
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Xenon", &xenon))
		{
			g_fiber_pool->queue_job([] {
				VEHICLE::TOGGLE_VEHICLE_MOD(player_vehicle, MOD_XENON_LIGHTS, xenon);
			});
		}


		static char plate[9] = { 0 };
		ImGui::SetNextItemWidth(200.f);
		components::input_text_with_hint("##plate", "Plate Number", plate, sizeof(plate), ImGuiInputTextFlags_None, [] {
			g->spawn.plate = plate;
		});
		ImGui::SameLine();
		if (components::button("Change Plate Number"))
		{
			g_fiber_pool->queue_job([] {
				vehicle::set_plate(self::veh, plate);
			});
		}




		if (!is_bike)
		{
			static int windowtint{};
			static char* windowtint_combo[] = { "None", "Black", "Dark", "Light" };

			ImGui::SetNextItemWidth(200);
			if (ImGui::Combo("Window Tint", &windowtint, windowtint_combo, IM_ARRAYSIZE(windowtint_combo)))
			{
				g_fiber_pool->queue_job([] {
					VEHICLE::SET_VEHICLE_WINDOW_TINT(player_vehicle, windowtint);
					});
			}

			ImGui::SetNextItemWidth(200);
			if (ImGui::BeginCombo("Wheel Type", vehicle_wheel_types[wheel_type].c_str()))
			{
				for (const auto& [type, name] : vehicle_wheel_types)
				{
					if (ImGui::Selectable(name.c_str(), (int)type == wheel_type))
					{
						if (wheel_type != type)
						{
							wheel_type = type;

							g_fiber_pool->queue_job([] {
								VEHICLE::SET_VEHICLE_WHEEL_TYPE(player_vehicle, wheel_type);
								VEHICLE::SET_VEHICLE_MOD(player_vehicle, MOD_FRONTWHEEL, 0, false);
								selected_slot = MOD_FRONTWHEEL;

								// refresh mod names
								player_vehicle = 0;
							});
						}
					}

					if (type == wheel_type)
					{
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}
		}

		ImGui::Separator();

		if (ImGui::ListBoxHeader("Slot", ImVec2(200, 200)))
		{
			for (int slot = MOD_SPOILERS; slot <= MOD_LIVERY; slot++)
			{
				if (slot_display_names[slot].empty())
					continue;
				if (ImGui::Selectable(slot_display_names[slot].c_str(), slot == selected_slot))
				{
					selected_slot = slot;
				}
			}
			ImGui::ListBoxFooter();
		}
		if (selected_slot != -1)
		{
			ImGui::SameLine();
			if (ImGui::ListBoxHeader("Mod", ImVec2(200, 200)))
			{
				for (int i = 0; i < mod_display_names[selected_slot].size(); i++)
				{
					if (mod_display_names[selected_slot][i].empty())
						continue;

					if (ImGui::Selectable(mod_display_names[selected_slot][i].c_str(), i == owned_mods[selected_slot]))
					{
						g_fiber_pool->queue_job([i] {
							NETWORK::NETWORK_REQUEST_CONTROL_OF_ENTITY(self::veh);

							owned_mods[selected_slot] = i;
							VEHICLE::SET_VEHICLE_MOD(player_vehicle, selected_slot, i - 1, false);
						});
					}
				}
				ImGui::ListBoxFooter();
			}
		}

		ImGui::Separator();
		components::small_text("Color Options");


		static const char* classic_names[] = { "Black", "Carbon Black", "Graphite", "Anthracite Black", "Black Steel", "Dark Steel", "Silver", "Bluish Silver", "Rolled Steel", "Shadow SIlver", "Stone Silver", "Midnight Silver", "Cast Iron Silver", "Red", "Torino Red", "Formula Red", "Lava Red", "Blaze Red", "Grace Red", "Garnet Red", "Sunset Red", "Cabernet Red", "Wine Red", "Candy Red", "Hot Pink", "Pfister Pink", "Salmon Pink", "Sunrise Orange", "Orange", "Bright Orange", "Gold", "Bronze", "Yellow", "Race Yellow", "Dew Yellow", "Dark Green", "Racing Green", "Sea Green", "Olive Green", "Bright Green", "Gasoline Green", "Lime Green", "Midnight Blue", "Galaxy Blue", "Dark Blue", "Saxon Blue", "Blue", "Mariner Blue", "Harbor Blue", "Diamond Blue", "Surf Blue", "Nautical Blue", "Racing Blue", "Ultra Blue", "Light Blue", "Chocolate Brown", "Bison Brown", "Creek Brown", "Feltzer Brown", "Maple Brown", "Beechwood Brown", "Sienna Brown", "Saddle Brown", "Moss Brown", "Woodbeech Brown", "Straw Brown", "Sandy Brown", "Bleached Brown", "Schafter Purple", "Spinnaker Purple", "Midnight Purple", "Bright Purple", "Cream", "Ice White", "Frost White" };
		static int classic_ids[] = { COLOR_CLASSIC_BLACK, COLOR_CLASSIC_CARBON_BLACK, COLOR_CLASSIC_GRAPHITE, COLOR_CLASSIC_ANHRACITE_BLACK, COLOR_CLASSIC_BLACK_STEEL, COLOR_CLASSIC_DARK_STEEL, COLOR_CLASSIC_SILVER, COLOR_CLASSIC_BLUISH_SILVER, COLOR_CLASSIC_ROLLED_STEEL, COLOR_CLASSIC_SHADOW_SILVER, COLOR_CLASSIC_STONE_SILVER, COLOR_CLASSIC_MIDNIGHT_SILVER, COLOR_CLASSIC_CAST_IRON_SILVER, COLOR_CLASSIC_RED, COLOR_CLASSIC_TORINO_RED, COLOR_CLASSIC_FORMULA_RED, COLOR_CLASSIC_LAVA_RED, COLOR_CLASSIC_BLAZE_RED, COLOR_CLASSIC_GRACE_RED, COLOR_CLASSIC_GARNET_RED, COLOR_CLASSIC_SUNSET_RED, COLOR_CLASSIC_CABERNET_RED, COLOR_CLASSIC_WINE_RED, COLOR_CLASSIC_CANDY_RED, COLOR_CLASSIC_HOT_PINK, COLOR_CLASSIC_PFSITER_PINK, COLOR_CLASSIC_SALMON_PINK, COLOR_CLASSIC_SUNRISE_ORANGE, COLOR_CLASSIC_ORANGE, COLOR_CLASSIC_BRIGHT_ORANGE, COLOR_CLASSIC_GOLD, COLOR_CLASSIC_BRONZE, COLOR_CLASSIC_YELLOW, COLOR_CLASSIC_RACE_YELLOW, COLOR_CLASSIC_DEW_YELLOW, COLOR_CLASSIC_DARK_GREEN, COLOR_CLASSIC_RACING_GREEN, COLOR_CLASSIC_SEA_GREEN, COLOR_CLASSIC_OLIVE_GREEN, COLOR_CLASSIC_BRIGHT_GREEN, COLOR_CLASSIC_GASOLINE_GREEN, COLOR_CLASSIC_LIME_GREEN, COLOR_CLASSIC_MIDNIGHT_BLUE, COLOR_CLASSIC_GALAXY_BLUE, COLOR_CLASSIC_DARK_BLUE, COLOR_CLASSIC_SAXON_BLUE, COLOR_CLASSIC_BLUE, COLOR_CLASSIC_MARINER_BLUE, COLOR_CLASSIC_HARBOR_BLUE, COLOR_CLASSIC_DIAMOND_BLUE, COLOR_CLASSIC_SURF_BLUE, COLOR_CLASSIC_NAUTICAL_BLUE, COLOR_CLASSIC_RACING_BLUE, COLOR_CLASSIC_ULTRA_BLUE, COLOR_CLASSIC_LIGHT_BLUE, COLOR_CLASSIC_CHOCOLATE_BROWN, COLOR_CLASSIC_BISON_BROWN, COLOR_CLASSIC_CREEEN_BROWN, COLOR_CLASSIC_FELTZER_BROWN, COLOR_CLASSIC_MAPLE_BROWN, COLOR_CLASSIC_BEECHWOOD_BROWN, COLOR_CLASSIC_SIENNA_BROWN, COLOR_CLASSIC_SADDLE_BROWN, COLOR_CLASSIC_MOSS_BROWN, COLOR_CLASSIC_WOODBEECH_BROWN, COLOR_CLASSIC_STRAW_BROWN, COLOR_CLASSIC_SANDY_BROWN, COLOR_CLASSIC_BLEACHED_BROWN, COLOR_CLASSIC_SCHAFTER_PURPLE, COLOR_CLASSIC_SPINNAKER_PURPLE, COLOR_CLASSIC_MIDNIGHT_PURPLE, COLOR_CLASSIC_BRIGHT_PURPLE, COLOR_CLASSIC_CREAM, COLOR_CLASSIC_ICE_WHITE, COLOR_CLASSIC_FROST_WHITE };
		static const char* matte_names[] = { "Black", "Gray", "Light Gray", "Ice White", "Blue", "Dark Blue", "Midnight Blue", "Midnight Purple", "Shafter Purple", "Red", "Dark Red", "Orange", "Yellow", "Lime Green", "Green", "Forest Green", "Foliage Green", "Olive Drab", "Dark Earth", "Desert Tan" };
		static int matte_ids[] = { COLOR_MATTE_BLACK, COLOR_MATTE_GRAY, COLOR_MATTE_LIGHT_GRAY, COLOR_MATTE_ICE_WHITE, COLOR_MATTE_BLUE, COLOR_MATTE_DARK_BLUE, COLOR_MATTE_MIDNIGHT_BLUE, COLOR_MATTE_MIDNIGHT_PURPLE, COLOR_MATTE_SCHAFTER_PURPLE, COLOR_MATTE_RED, COLOR_MATTE_DARK_RED, COLOR_MATTE_ORANGE, COLOR_MATTE_YELLOW, COLOR_MATTE_LIME_GREEN, COLOR_MATTE_GREEN, COLOR_MATTE_FOREST_GREEN, COLOR_MATTE_FOLIAGE_GREEN, COLOR_MATTE_OLIVE_DARB, COLOR_MATTE_DARK_EARTH, COLOR_MATTE_DESERT_TAN };
		static const char* metal_names[] = { "Brushed Steel", "Brushed Black Steel", "Brushed Aluminium", "Pure Gold", "Brushed Gold" };
		static int metal_ids[] = { COLOR_METALS_BRUSHED_STEEL, COLOR_METALS_BRUSHED_BLACK_STEEL, COLOR_METALS_BRUSHED_ALUMINUM, COLOR_METALS_PURE_GOLD, COLOR_METALS_BRUSHED_GOLD };

		static int color_to_change = 0;
		static int color_btn_clicked = 1;
		static int color_type = 8;

		if (ImGui::RadioButton("Primary", &color_to_change, 0))
		{
			color_btn_clicked = 1;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Secondary", &color_to_change, 1))
		{
			color_btn_clicked = 1;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Tire Smake", &color_to_change, 2))
		{
			color_btn_clicked = 1;
		}
		ImGui::SameLine();
		if (ImGui::Button("Remove Custom Color"))
		{
			g_fiber_pool->queue_job([]
				{
					VEHICLE::CLEAR_VEHICLE_CUSTOM_PRIMARY_COLOUR(player_vehicle);
					VEHICLE::CLEAR_VEHICLE_CUSTOM_SECONDARY_COLOUR(player_vehicle);
					VEHICLE::SET_VEHICLE_COLOURS(player_vehicle, primary_color, secondary_color);
				});
		}

		ImGui::Separator();

		ImGui::BeginGroup();

		if (color_to_change == 0 || color_to_change == 1)
		{
			// primary and secondary color

			if (ImGui::ListBoxHeader("##colors", ImVec2(200, 254)))
			{
				if (ImGui::Selectable("Custom", color_type == 8, ImGuiSelectableFlags_SelectOnClick))
				{
					color_type = 8;
				}
				if (ImGui::Selectable("Chrome", color_type == 0))
				{
					color_type = 0;
				}
				if (ImGui::Selectable("Classic", color_type == 1))
				{
					color_type = 1;
				}
				if (ImGui::Selectable("Matte", color_type == 2))
				{
					color_type = 2;
				}
				if (ImGui::Selectable("Metals", color_type == 3))
				{
					color_type = 3;
				}
				if (ImGui::Selectable("Pearlescent", color_type == 4))
				{
					color_type = 4;
				}
				if (ImGui::Selectable("Wheel Color", color_type == 5))
				{
					color_type = 5;
				}
				if (ImGui::Selectable("Interior Color", color_type == 6))
				{
					color_type = 6;
				}
				if (ImGui::Selectable("Dashboard Color", color_type == 7))
				{
					color_type = 7;
				}
				ImGui::ListBoxFooter();
			}
		}
		else
		{
			// tyre smoke color

			color_type = 8;
			ImGui::Text("");
		}

		if (color_type == 8)
		{
			// custom color

			static float color[3] = { 1, 1, 1 };

			ImGui::SameLine();
			ImGui::SetNextItemWidth(212);
			if (ImGui::ColorPicker3("Custom VehColor", color, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_DisplayHex))
			{
				g_fiber_pool->queue_job([]
					{
						switch (color_to_change)
						{
						case 0:
							VEHICLE::SET_VEHICLE_CUSTOM_PRIMARY_COLOUR(player_vehicle, (int)(color[0] * 255), (int)(color[1] * 255), (int)(color[2] * 255));
							break;
						case 1:
							VEHICLE::SET_VEHICLE_CUSTOM_SECONDARY_COLOUR(player_vehicle, (int)(color[0] * 255), (int)(color[1] * 255), (int)(color[2] * 255));
							break;
						case 2:
							VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(player_vehicle, (int)(color[0] * 255), (int)(color[1] * 255), (int)(color[2] * 255));
							break;
						}
					});
			}

			if (color_btn_clicked == 1)
			{
				g_fiber_pool->queue_job([] {
					int color_get[3];

					if (color_to_change == 0)
					{
						VEHICLE::GET_VEHICLE_CUSTOM_PRIMARY_COLOUR(player_vehicle, &color_get[0], &color_get[1], &color_get[2]);
					}
					else if (color_to_change == 1)
					{
						VEHICLE::GET_VEHICLE_CUSTOM_SECONDARY_COLOUR(player_vehicle, &color_get[0], &color_get[1], &color_get[2]);
					}
					else if (color_to_change == 2)
					{
						VEHICLE::GET_VEHICLE_TYRE_SMOKE_COLOR(player_vehicle, &color_get[0], &color_get[1], &color_get[2]);
					}

					color[0] = (float)color_get[0] / 255;
					color[1] = (float)color_get[1] / 255;
					color[2] = (float)color_get[2] / 255;
				});

				color_btn_clicked = 0;
			}

		}
		else
		{
			// standard color

			int color_check = 0;
			switch (color_type)
			{
			case 4:
				color_check = pearlescent;
				break;
			case 5:
				color_check = wheel_color;
				break;
			case 6:
				color_check = interior_color;
				break;
			case 7:
				color_check = dashboard_color;
				break;
			default:
				color_check = (color_to_change == 0) ? primary_color : secondary_color;
			}

			ImGui::SameLine();
			if (ImGui::ListBoxHeader("##color", ImVec2(200, 254)))
			{
				switch (color_type)
				{
				case 0: //Chrome
				{
					if (ImGui::Selectable("Chrome", color_check == COLOR_CHROME))
					{
						if (color_to_change == 0)
							primary_color = COLOR_CHROME;
						else
							secondary_color = COLOR_CHROME;
						g_fiber_pool->queue_job([] {
							VEHICLE::SET_VEHICLE_COLOURS(player_vehicle, primary_color, secondary_color);
						});
					}
					break;
				}
				case 1: //Classic
				{
					for (int i = 0; i < 74; i++)
					{
						if (ImGui::Selectable(classic_names[i], color_check == classic_ids[i]))
						{
							if (color_to_change == 0)
								primary_color = classic_ids[i];
							else
								secondary_color = classic_ids[i];
							g_fiber_pool->queue_job([] {
								VEHICLE::SET_VEHICLE_COLOURS(player_vehicle, primary_color, secondary_color);
							});
						}
					}
					break;
				}
				case 2: //Matte
				{
					for (int i = 0; i < 20; i++)
					{
						if (ImGui::Selectable(matte_names[i], color_check == matte_ids[i]))
						{
							if (color_to_change == 0)
								primary_color = matte_ids[i];
							else
								secondary_color = matte_ids[i];
							g_fiber_pool->queue_job([] {
								VEHICLE::SET_VEHICLE_COLOURS(player_vehicle, primary_color, secondary_color);
							});
						}
					}
					break;
				}
				case 3: //Metals
				{
					for (int i = 0; i < 5; i++)
					{
						if (ImGui::Selectable(metal_names[i], color_check == metal_ids[i]))
						{
							if (color_to_change == 0)
								primary_color = metal_ids[i];
							else
								secondary_color = metal_ids[i];
							g_fiber_pool->queue_job([] {
								VEHICLE::SET_VEHICLE_COLOURS(player_vehicle, primary_color, secondary_color);
							});
						}
					}
					break;
				}
				case 4: //Pearlescent
				{
					for (int i = 0; i < 74; i++)
					{
						if (ImGui::Selectable(classic_names[i], color_check == classic_ids[i]))
						{
							pearlescent = classic_ids[i];
							g_fiber_pool->queue_job([] {
								VEHICLE::SET_VEHICLE_EXTRA_COLOURS(player_vehicle, pearlescent, wheel_color);
							});
						}
					}
					break;
				}
				case 5: //Wheel Color
				{
					for (int i = 0; i < 74; i++)
					{
						if (ImGui::Selectable(classic_names[i], color_check == classic_ids[i]))
						{
							wheel_color = classic_ids[i];
							g_fiber_pool->queue_job([] {
								VEHICLE::SET_VEHICLE_EXTRA_COLOURS(player_vehicle, pearlescent, wheel_color);
							});
						}
					}
					break;
				}
				case 6: //Interior Color
				{
					for (int i = 0; i < 74; i++)
					{
						if (ImGui::Selectable(classic_names[i], color_check == classic_ids[i]))
						{
							interior_color = classic_ids[i];
							g_fiber_pool->queue_job([] {
								VEHICLE::SET_VEHICLE_INTERIOR_COLOR_(player_vehicle, interior_color);
							});
						}
					}
					break;
				}
				case 7: //Dashboard Color
				{
					for (int i = 0; i < 74; i++)
					{
						if (ImGui::Selectable(classic_names[i], color_check == classic_ids[i]))
						{
							dashboard_color = classic_ids[i];
							g_fiber_pool->queue_job([] {
								VEHICLE::SET_VEHICLE_DASHBOARD_COLOR_(player_vehicle, dashboard_color);
							});
						}
					}
					break;
				}
				}
				ImGui::ListBoxFooter();
			}
		}

		ImGui::EndGroup();
	}
}
