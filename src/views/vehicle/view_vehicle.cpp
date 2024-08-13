#include "core/data/speed_units.hpp"
#include "util/mobile.hpp"
#include "util/teleport.hpp"
#include "views/view.hpp"

namespace big
{
	void view::vehicle()
	{
		components::button("MORS_FIX_ALL"_T, [] {
			int amount_fixed = mobile::mors_mutual::fix_all();
			auto v_fixed     = amount_fixed == 1 ? "VEHICLE_FIX_HAS"_T.data() : "VEHICLE_FIX_HAVE"_T.data(); 

			g_notification_service.push_success("MOBILE"_T.data(),
			    std::vformat("VEHICLE_FIX_AMOUNT"_T.data(),
			        std::make_format_args(amount_fixed, v_fixed)));
		});

		ImGui::SameLine();
		components::button("DELETE"_T, [] {
			auto handle = self::veh;
			if (ENTITY::DOES_ENTITY_EXIST(handle))
				TASK::CLEAR_PED_TASKS_IMMEDIATELY(self::ped), entity::delete_entity(handle);
		});

		/*ImGui::SameLine();*/
		components::button("REPAIR"_T, [] {
			vehicle::repair(self::veh);
		});
		ImGui::SameLine();
		components::button("SET_VEHICLE_DEFORMATION_FIXED"_T, [] {
			VEHICLE::SET_VEHICLE_DEFORMATION_FIXED(self::veh);
		});
		ImGui::SameLine();
		components::button("SYNC_DAMAGES"_T, [] {
			if (self::veh == 0)
				return;

			auto net_obj = g_local_player->m_vehicle->m_net_object;
			if (net_obj /*&& !net_obj->m_is_remote*/) //There shouldn't be a scenario where we are driving and don't have ownership of the netObj.
			{
				g_pointers->m_gta.m_set_vehicle_damage_sync_tree(net_obj);
			}
		});


		/*ImGui::SameLine();*/
		components::command_checkbox<"keepfixed">();

		ImGui::Separator();

		components::button("TP_IN_PV"_T, [] {
			Vehicle veh = mobile::mechanic::get_personal_vehicle();
			teleport::into_vehicle(veh);
		});
		ImGui::SameLine();
		components::button("BRING_PV"_T, [] {
			Vehicle veh = mobile::mechanic::get_personal_vehicle();
			vehicle::bring(veh, self::pos, true);
		});
		ImGui::SameLine();
		components::button("BRING_CLOSEST_VEHICLE"_T, [] {
			Vehicle veh = vehicle::get_closest_to_location(self::pos, 200);
			vehicle::bring(veh, self::pos, true, -1);
		});

		ImGui::Separator();

		components::button("TURN_ENGINE_ON"_T, [] {
			vehicle::set_engine_state(self::veh, true, g.vehicle.change_engine_state_immediately, g.vehicle.disable_engine_auto_start);
		});
		ImGui::SameLine();
		components::button("TURN_ENGINE_OFF"_T, [] {
			vehicle::set_engine_state(self::veh, false, g.vehicle.change_engine_state_immediately, g.vehicle.disable_engine_auto_start);
		});
		ImGui::Checkbox("DISABLE_ENGINE_AUTO_START"_T.data(), &g.vehicle.disable_engine_auto_start);
		ImGui::SameLine();
		ImGui::Checkbox("CHANGE_STATE_IMMEDIATELY"_T.data(), &g.vehicle.change_engine_state_immediately);
		ImGui::SameLine();
		components::command_checkbox<"keepengine">();

		ImGui::SeparatorText("GENERAL"_T.data());
		{
			ImGui::BeginGroup();

			components::command_checkbox<"vehgodmode">("GOD_MODE"_T.data());
			components::command_checkbox<"infinitevehammo">();
			components::command_checkbox<"hornboost">();
			components::command_checkbox<"vehjump">();
			components::command_checkbox<"invisveh">();
			if (g.vehicle.vehinvisibility)
			{
				components::command_checkbox<"localinvisveh">();
			}
			components::command_checkbox<"vehnocollision">();
			components::command_checkbox<"vehallweapons">();
			components::command_checkbox<"allvehsinheists">();

			ImGui::EndGroup();
			ImGui::SameLine();
			ImGui::BeginGroup();

			components::command_checkbox<"instantbrake">();
			components::command_checkbox<"blockhoming">();
			components::command_checkbox<"driveonwater">();
			components::command_checkbox<"vehiclecontrol">();
			components::command_checkbox<"keepvehicleclean">();


			ImGui::EndGroup();
			ImGui::SameLine();
			ImGui::BeginGroup();

			components::command_checkbox<"seatbelt">();
			ImGui::Checkbox("TURN_SIGNALS"_T.data(), &g.vehicle.turn_signals);
			if (g.vehicle.turn_signals)
			{
				ImGui::Checkbox("FULLY_AUTOMATIC_SIGNAL"_T.data(), &g.vehicle.auto_turn_signals);
			}
			components::command_checkbox<"driveunder">();
			components::command_checkbox<"keeponground">();

			components::command_checkbox<"mutesiren">();

			components::command_checkbox<"speedometer">();
			components::options_modal("Speedometer", [] {
				ImGui::Text("POS_X_Y"_T.data());

				float pos[2] = {g.vehicle.speedo_meter.x, g.vehicle.speedo_meter.y};

				if (ImGui::SliderFloat2("###speedo_pos", pos, .001f, .999f, "%.3f"))
				{
					g.vehicle.speedo_meter.x = pos[0];
					g.vehicle.speedo_meter.y = pos[1];
				}

				components::command_checkbox<"speedometerleftside">();
				ImGui::SameLine();
				components::command_checkbox<"speedometergears">();
			});

			ImGui::EndGroup();
		}
		ImGui::SeparatorText("PROOFS"_T.data());
		{
			if (ImGui::Button("CHECK_ALL"_T.data()))
			{
				g.vehicle.proof_bullet    = true;
				g.vehicle.proof_fire      = true;
				g.vehicle.proof_collision = true;
				g.vehicle.proof_melee     = true;
				g.vehicle.proof_explosion = true;
				g.vehicle.proof_steam     = true;
				g.vehicle.proof_water     = true;
			}

			ImGui::SameLine();

			if (ImGui::Button("UNCHECK_ALL"_T.data()))
			{
				g.vehicle.proof_bullet    = false;
				g.vehicle.proof_fire      = false;
				g.vehicle.proof_collision = false;
				g.vehicle.proof_melee     = false;
				g.vehicle.proof_explosion = false;
				g.vehicle.proof_steam     = false;
				g.vehicle.proof_water     = false;
			}

			ImGui::BeginGroup();

			ImGui::Checkbox("BULLET"_T.data(), &g.vehicle.proof_bullet);
			ImGui::Checkbox("FIRE"_T.data(), &g.vehicle.proof_fire);

			ImGui::EndGroup();
			ImGui::SameLine();
			ImGui::BeginGroup();

			ImGui::Checkbox("COLLISION"_T.data(), &g.vehicle.proof_collision);
			ImGui::Checkbox("MELEE"_T.data(), &g.vehicle.proof_melee);

			ImGui::EndGroup();
			ImGui::SameLine();
			ImGui::BeginGroup();

			ImGui::Checkbox("EXPLOSION"_T.data(), &g.vehicle.proof_explosion);
			ImGui::Checkbox("STEAM"_T.data(), &g.vehicle.proof_steam);

			ImGui::EndGroup();
			ImGui::SameLine();
			ImGui::BeginGroup();

			ImGui::Checkbox("WATER"_T.data(), &g.vehicle.proof_water);

			ImGui::EndGroup();
		}
		ImGui::SeparatorText("SPEED_UNIT"_T.data());
		{
			ImGui::RadioButton(speed_unit_strings[(int)SpeedUnit::KMPH].c_str(), (int*)&g.vehicle.speed_unit, (int)SpeedUnit::KMPH);
			ImGui::SameLine();
			ImGui::RadioButton(speed_unit_strings[(int)SpeedUnit::MIPH].c_str(), (int*)&g.vehicle.speed_unit, (int)SpeedUnit::MIPH);
			ImGui::SameLine();
			ImGui::RadioButton(speed_unit_strings[(int)SpeedUnit::MPS].c_str(), (int*)&g.vehicle.speed_unit, (int)SpeedUnit::MPS);
		}

		g.vehicle.proof_mask = 0;
		if (g.vehicle.god_mode)
		{
			g.vehicle.proof_mask |= static_cast<int>(eEntityProofs::GOD);
		}
		if (g.vehicle.proof_bullet)
		{
			g.vehicle.proof_mask |= static_cast<int>(eEntityProofs::BULLET);
		}
		if (g.vehicle.proof_fire)
		{
			g.vehicle.proof_mask |= static_cast<int>(eEntityProofs::FIRE);
		}
		if (g.vehicle.proof_collision)
		{
			g.vehicle.proof_mask |= static_cast<int>(eEntityProofs::COLLISION);
		}
		if (g.vehicle.proof_melee)
		{
			g.vehicle.proof_mask |= static_cast<int>(eEntityProofs::MELEE);
		}
		if (g.vehicle.proof_explosion)
		{
			g.vehicle.proof_mask |= static_cast<int>(eEntityProofs::EXPLOSION);
		}
		if (g.vehicle.proof_steam)
		{
			g.vehicle.proof_mask |= static_cast<int>(eEntityProofs::STEAM);
		}
		if (g.vehicle.proof_water)
		{
			g.vehicle.proof_mask |= static_cast<int>(eEntityProofs::WATER);
		}
	}
}
