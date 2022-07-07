#include "core/data/speedo_meters.hpp"
#include "fiber_pool.hpp"
#include "gui/handling/handling_tabs.hpp"
#include "script.hpp"
#include "util/vehicle.hpp"
#include "views/view.hpp"


namespace big
{
	void view::vehicle() {
		ImGui::BeginGroup();
		ImGui::Checkbox("Can Be Targeted", &g->vehicle.is_targetable);
		ImGui::Checkbox("God Mode", &g->vehicle.god_mode);
		ImGui::Checkbox("Horn Boost", &g->vehicle.horn_boost);
		ImGui::Checkbox("Vehicle Jump", &g->vehicle.vehicle_jump);
		ImGui::Checkbox("Instant Brake", &g->vehicle.instant_brake);

		ImGui::EndGroup();
		ImGui::SameLine();
		ImGui::BeginGroup();

		ImGui::Checkbox("Seatbelt", &g->vehicle.seatbelt);

		components::button("Repair", [] {
			vehicle::repair(self::veh);
			});

		ImGui::SameLine();

		components::button("Instant in PV", [] {
			if (!*g_pointers->m_is_session_started) return g_notification_service->push_warning("WARNING", "Go into GTA V Online to use this option");

			vehicle::go_into_personal_vehicle();
			});

		if (ImGui::TreeNode("Paint"))
		{
			ImGui::ListBox("RGB Type", &g->vehicle.rainbow_paint, vehicle::rgb_types, 3);

			if (g->vehicle.rainbow_paint)
			{
				ImGui::SliderInt("RGB Speed", &g->rgb.speed, 1, 10);
			}

			ImGui::TreePop();
		}

		ImGui::Checkbox("Turn Signals", &g->vehicle.turn_signals);

		if (g->vehicle.turn_signals)
		{
			ImGui::Checkbox("Fully Automatic Signal", &g->vehicle.auto_turn_signals);
		}

		ImGui::Checkbox("Drive On Water", &g->vehicle.drive_on_water);

		ImGui::EndGroup();

		ImGui::Separator();

		static char model[8];
		components::input_text_with_hint("###vehicle_license_text", "Vehicle License Text", model, sizeof(model), ImGuiInputTextFlags_EnterReturnsTrue, [] {
			g_fiber_pool->queue_job([] {
				const Hash hash = rage::joaat(model);				
				if (!STREAMING::HAS_MODEL_LOADED(hash)) {
					VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(self::veh, model);
					return;
				}
				});
			});

		ImGui::Separator();

		components::small_text("Auto Drive");

		components::button("Drive To Waypoint", [] {

			g->vehicle.auto_drive_to_waypoint = true;
			});
		ImGui::SameLine();

		components::button("Wander", [] {

			g->vehicle.auto_drive_wander = true;
			});

		ImGui::SliderInt("Top Speed", &g->vehicle.auto_drive_speed, 1, 200);

		components::button("E-Stop", [] {

				g->vehicle.auto_drive_to_waypoint = false;
				g->vehicle.auto_drive_wander = false;
				VEHICLE::SET_VEHICLE_FORWARD_SPEED(self::veh, 0);
				TASK::CLEAR_VEHICLE_TASKS_(self::veh);
				TASK::CLEAR_PED_TASKS(self::ped);
			});

		if (ImGui::ListBox("Driving Style", &g->vehicle.driving_style_id, vehicle::driving_style_names, 2))
		{
			g->vehicle.driving_style_flags = vehicle::driving_styles[g->vehicle.driving_style_id];
			g_notification_service->push_warning("Auto Drive", fmt::format("Driving style set to {}.", vehicle::driving_style_names[g->vehicle.driving_style_id]));
		}

		ImGui::Separator();

		components::small_text("Vehicle Fly");

		ImGui::Checkbox("Enabled", &g->vehicle.fly.enabled);
		ImGui::SameLine();
		ImGui::Checkbox("Disable Collision", &g->vehicle.fly.no_collision);

		ImGui::Checkbox("Don't Stop", &g->vehicle.fly.dont_stop);
		ImGui::SameLine();
		ImGui::Checkbox("Stop On Exit", &g->vehicle.fly.stop_on_exit);

		ImGui::SliderFloat("Speed", &g->vehicle.fly.speed, 1.f, 100.f, "%.0f", 1);

		ImGui::Separator();

		components::small_text("LS Customs");

		components::button("Start LS Customs", [] {
			g->vehicle.ls_customs = true;
			});

		ImGui::Separator();

		components::small_text("Speedo Meter");

		SpeedoMeter selected = g->vehicle.speedo_meter.type;

		ImGui::Text("Type:");
		if (ImGui::BeginCombo("###speedo_type", speedo_meters[(int)selected].name))
		{
			for (const speedo_meter& speedo : speedo_meters)
			{
				if (ImGui::Selectable(speedo.name, speedo.id == selected))
				{
					g->vehicle.speedo_meter.type = speedo.id;
				}

				if (speedo.id == selected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		ImGui::Text("Position");

		float pos[2];
		pos[0] = g->vehicle.speedo_meter.x;
		pos[1] = g->vehicle.speedo_meter.y;
		if (ImGui::SliderFloat2("###speedo_pos", pos, .001f, .999f, "%.3f"))
		{
			g->vehicle.speedo_meter.x = pos[0];
			g->vehicle.speedo_meter.y = pos[1];
		}

		ImGui::Checkbox("Left Sided", &g->vehicle.speedo_meter.left_side);
		ImGui::SameLine();
		ImGui::Checkbox("Type Text", &g->vehicle.speedo_meter.type_text);

	}
}
