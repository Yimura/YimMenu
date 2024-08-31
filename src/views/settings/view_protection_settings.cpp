#include "views/view.hpp"

namespace big
{
	static inline void set_all_protections(bool state)
	{
		auto start = reinterpret_cast<bool*>(&g.protections);
		auto end   = reinterpret_cast<bool*>(reinterpret_cast<std::byte*>(start) + sizeof(g.protections));

		for (auto* i = start; i < end; ++i)
			*i = state;
	}

	static inline void reset_protections()
	{
		g.protections = {};
	}

	void view::protection_settings()
	{
		ImGui::BeginGroup(); // 8 Checkbox
		{
			ImGui::Checkbox("BOUNTY"_T.data(), &g.protections.script_events.bounty);
			ImGui::Checkbox("CEO_MONEY"_T.data(), &g.protections.script_events.ceo_money);
			ImGui::Checkbox("FORCE_MISSION"_T.data(), &g.protections.script_events.force_mission);
			ImGui::Checkbox("FORCE_TELEPORT"_T.data(), &g.protections.script_events.force_teleport);
			ImGui::Checkbox("GTA_BANNER"_T.data(), &g.protections.script_events.gta_banner);
			ImGui::Checkbox("MC_TELEPORT"_T.data(), &g.protections.script_events.mc_teleport);
			ImGui::Checkbox("SEND_TO_CUTSCENE"_T.data(), &g.protections.script_events.send_to_cutscene);
			ImGui::Checkbox("REQUEST_CONTROL_EVENT"_T.data(), &g.protections.request_control);
		}
		ImGui::EndGroup();
		ImGui::SameLine();
		ImGui::BeginGroup(); // 8 Checkbox
		{
			ImGui::Checkbox("SEND_TO_LOCATION"_T.data(), &g.protections.script_events.send_to_location);
			ImGui::Checkbox("SOUND_SPAM"_T.data(), &g.protections.sound_spam);
			ImGui::Checkbox("PERSONAL_VEHICLE_DESTROYED"_T.data(), &g.protections.script_events.personal_vehicle_destroyed);
			ImGui::Checkbox("REMOTE_OFF_RADAR"_T.data(), &g.protections.script_events.remote_off_radar);
			ImGui::Checkbox("ROTATE_CAM"_T.data(), &g.protections.script_events.rotate_cam);
			ImGui::Checkbox("TELEPORT_TO_WAREHOUSE"_T.data(), &g.protections.script_events.teleport_to_warehouse);
			ImGui::Checkbox("START_ACTIVITY"_T.data(), &g.protections.script_events.start_activity);
			ImGui::Checkbox("REMOTE_RAGDOLL"_T.data(), &g.protections.ragdoll);
		}
		ImGui::EndGroup();
		ImGui::SameLine();
		ImGui::BeginGroup(); // 7 Checkbox
		{
			ImGui::Checkbox("SPECTATE"_T.data(), &g.protections.script_events.spectate);
			ImGui::Checkbox("VEHICLE_KICK"_T.data(), &g.protections.script_events.vehicle_kick);
			ImGui::Checkbox("CLEAR_WANTED_LEVEL"_T.data(), &g.protections.script_events.clear_wanted_level);
			ImGui::Checkbox("BLOCK_RID_JOINING"_T.data(), &g.protections.rid_join);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("BLOCK_RID_JOINING_DESCRIPTION"_T.data());
			ImGui::Checkbox("RECEIVE_PICKUP"_T.data(), &g.protections.receive_pickup);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("VIEW_PROTECTION_SETTINGS_RECEIVE_PICKUP_TOOLTIP"_T.data());
			ImGui::Checkbox("GIVE_COLLECTIBLE"_T.data(), &g.protections.script_events.give_collectible);
			ImGui::Checkbox("REMOTE_WANTED_LEVEL"_T.data(), &g.protections.remote_wanted_level);
		}
		ImGui::EndGroup();
		ImGui::SameLine();
		ImGui::BeginGroup(); // 7 Checkbox
		{
			ImGui::Checkbox("ADMIN_CHECK"_T.data(), &g.protections.admin_check);
			ImGui::Checkbox("VIEW_PROTECTION_SETTINGS_KICK_REJOIN"_T.data(), &g.protections.kick_rejoin);
			ImGui::Checkbox("BLOCK_TRAFFIC_MANIPULATION"_T.data(), &g.protections.stop_traffic);
			components::command_checkbox<"forcerelays">();
			ImGui::Checkbox("DESYNC_KICK"_T.data(), &g.protections.desync_kick);
			ImGui::Checkbox("REMOTE_CLEAR_PED_TASK"_T.data(), &g.protections.clear_ped_tasks);
			ImGui::Checkbox("REMOTE_VEH_SPECIAL_ABILITY_EVENT"_T.data(), &g.protections.vehicle_special_ability);
		}
		ImGui::EndGroup();

		ImGui::SeparatorText("VIEW_PROTECTION_SETTINGS_OPTIONS"_T.data());
		ImGui::BeginGroup();
		if (ImGui::Button("VIEW_PROTECTION_SETTINGS_ENABLE_ALL_PROTECTIONS"_T.data()))
			set_all_protections(true);
		ImGui::SameLine();
		if (ImGui::Button("VIEW_PROTECTION_SETTINGS_DISABLE_ALL_PROTECTIONS"_T.data()))
			set_all_protections(false);
		ImGui::SameLine();
		if (ImGui::Button("VIEW_PROTECTION_SETTINGS_RESET_PROTECTIONS"_T.data()))
			reset_protections();
		ImGui::EndGroup();
	};
}
