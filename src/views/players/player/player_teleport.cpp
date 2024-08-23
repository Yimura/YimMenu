#include "core/data/apartment_names.hpp"
#include "core/data/warehouse_names.hpp"
#include "util/teleport.hpp"
#include "util/toxic.hpp"
#include "views/view.hpp"

namespace big
{
	void view::player_teleport()
	{
		ImGui::SeparatorText("GUI_TAB_TELEPORT"_T.data());

		components::player_command_button<"playertp">(g_player_service->get_selected());
		ImGui::SameLine();
		components::player_command_button<"playervehtp">(g_player_service->get_selected());
		ImGui::SameLine();
		components::player_command_button<"bring">(g_player_service->get_selected());
		ImGui::SameLine();
		components::button("VIEW_PLAYER_TELEPORT_YOUR_WAYPOINT"_T, [] {
			Vector3 location;
			if (blip::get_blip_location(location, (int)BlipIcons::RADAR_WAYPOINT))
				entity::load_ground_at_3dcoord(location), teleport::teleport_player_to_coords(g_player_service->get_selected(), location);
		});

		components::player_command_button<"intkick">(g_player_service->get_selected(), {});

		ImGui::SetNextItemWidth(300);
		if (ImGui::BeginCombo("##apartment", apartment_names[g.session.send_to_apartment_idx]))
		{
			for (int i = 1; i < apartment_names.size(); i++)
			{
				if (ImGui::Selectable(apartment_names[i], i == g.session.send_to_apartment_idx))
				{
					g.session.send_to_apartment_idx = i;
				}

				if (i == g.session.send_to_apartment_idx)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}

		ImGui::SameLine();

		components::player_command_button<"apartmenttp">(g_player_service->get_selected(),
			{(uint64_t)g.session.send_to_apartment_idx});

		ImGui::SetNextItemWidth(300);
		if (ImGui::BeginCombo("##warehouse", warehouse_names[g.session.send_to_warehouse_idx]))
		{
			for (int i = 1; i < warehouse_names.size(); i++)
			{
				if (ImGui::Selectable(warehouse_names[i], i == g.session.send_to_warehouse_idx))
				{
					g.session.send_to_warehouse_idx = i;
				}

				if (i == g.session.send_to_warehouse_idx)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}

		ImGui::SameLine();

		components::player_command_button<"warehousetp">(g_player_service->get_selected(),
			{(uint64_t)g.session.send_to_warehouse_idx});

		ImGui::BeginGroup();
		components::button("TP_TO_DARTS"_T, [] {
			toxic::start_activity(g_player_service->get_selected(), eActivityType::Darts);
		});
		components::button("TP_TO_FLIGHT_SCHOOL"_T, [] {
			toxic::start_activity(g_player_service->get_selected(), eActivityType::PilotSchool);
		});
		components::button("TP_TO_MAP_CENTER"_T, [] {
			toxic::start_activity(g_player_service->get_selected(), eActivityType::ArmWresling);
		});
		components::button("TP_TO_SKYDIVE"_T, [] {
			toxic::start_activity(g_player_service->get_selected(), eActivityType::Skydive);
		});
		components::player_command_button<"interiortp">(g_player_service->get_selected(), {81}, "VIEW_PLAYER_TELEPORT_TP_TO_MOC"_T);
		components::player_command_button<"interiortp">(g_player_service->get_selected(), {123}, "VIEW_PLAYER_TELEPORT_TP_TO_CASINO"_T);
		components::player_command_button<"interiortp">(g_player_service->get_selected(), {124}, "VIEW_PLAYER_TELEPORT_TP_TO_PENTHOUSE"_T);
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		components::player_command_button<"interiortp">(g_player_service->get_selected(), {146}, "VIEW_PLAYER_TELEPORT_TP_TO_MUSIC_LOCKER"_T);
		components::player_command_button<"interiortp">(g_player_service->get_selected(), {148}, "VIEW_PLAYER_TELEPORT_TP_TO_RECORD_A_STUDIOS"_T);
		components::player_command_button<"interiortp">(g_player_service->get_selected(), {149}, "VIEW_PLAYER_TELEPORT_TP_TO_CUSTOM_AUTO_SHOP"_T);
		components::player_command_button<"interiortp">(g_player_service->get_selected(), {155}, "VIEW_PLAYER_TELEPORT_TP_TO_AGENCY"_T);
		components::player_command_button<"interiortp">(g_player_service->get_selected(), {160}, "VIEW_PLAYER_TELEPORT_TP_TO_FREAKSHOP"_T);
		components::player_command_button<"interiortp">(g_player_service->get_selected(), {161}, "VIEW_PLAYER_TELEPORT_TP_TO_MULTI_FLOOR_GARAGE"_T);
		components::player_command_button<"interiortp">(g_player_service->get_selected(), {128}, "VIEW_PLAYER_TELEPORT_TP_TO_ARCADE"_T);
		ImGui::EndGroup();

		if (g_player_service->get_selected()->get_ped())
		{
			static float new_location[3];
			auto& current_location = *reinterpret_cast<float(*)[3]>(g_player_service->get_selected()->get_ped()->get_position());

			components::small_text("VIEW_PLAYER_TELEPORT_CUSTOM_TP"_T);
			ImGui::SetNextItemWidth(400);
			ImGui::InputFloat3("##customlocation", new_location);
			components::button("GUI_TAB_TELEPORT"_T, [] {
				teleport::teleport_player_to_coords(g_player_service->get_selected(), *reinterpret_cast<rage::fvector3*>(&new_location));
			});
			ImGui::SameLine();
			if (ImGui::Button("VIEW_PLAYER_TELEPORT_GET_CURRENT"_T.data()))
			{
				std::copy(std::begin(current_location), std::end(current_location), std::begin(new_location));
			}
		}
	}
}
