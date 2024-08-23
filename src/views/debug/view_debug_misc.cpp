#include "gta/joaat.hpp"
#include "gui/components/components.hpp"
#include "hooking/hooking.hpp"
#include "util/pathfind.hpp"
#include "util/system.hpp"
#include "view_debug.hpp"
#include "network/CNetworkPlayerMgr.hpp"

namespace big
{
	void debug::misc()
	{
		if (ImGui::BeginTabItem("DEBUG_TAB_MISC"_T.data()))
		{
			components::command_checkbox<"external_console">();

			components::command_checkbox<"windowhook">("VIEW_DEBUG_MISC_DISABLE_GTA_WINDOW_HOOK"_T);

			ImGui::Text(std::format("{}: {}/{}", "VIEW_DEBUG_MISC_FIBER_POOL_USAGE"_T, g_fiber_pool->get_used_fibers(), g_fiber_pool->get_total_fibers()).c_str());
			ImGui::SameLine();
			if (components::button("RESET"_T))
			{
				g_fiber_pool->reset();
			}

			if (components::button("VIEW_DEBUG_MISC_TRIGGER_GTA_ERROR_BOX"_T))
			{
				hooks::log_error_message_box(0xBAFD530B, 1);
			}

			if (components::button("DUMP_ENTRYPOINTS"_T))
			{
				system::dump_entry_points();
			}

			components::button("NETWORK_BAIL"_T, [] {
				NETWORK::NETWORK_BAIL(16, 0, 0);
			});

			components::button("DEBUG_REMOVE_FROM_BAD_SPORT"_T, [] {
				STATS::STAT_SET_INT("MPPLY_BADSPORT_MESSAGE"_J, 0, 1);
				STATS::STAT_SET_INT("MPPLY_BECAME_BADSPORT_NUM"_J, 0, 1);
				STATS::STAT_SET_FLOAT("MPPLY_OVERALL_BADSPORT"_J, 0, true);
				STATS::STAT_SET_BOOL("MPPLY_CHAR_IS_BADSPORT"_J, false, true);
			});

			components::button("LOAD_MP_MAP"_T, [] {
				DLC::ON_ENTER_MP();
			});

			ImGui::SameLine();

			components::button("LOAD_SP_MAP"_T, [] {
				DLC::ON_ENTER_SP();
			});

			components::button("SKIP_CUTSCENE"_T, [] {
				CUTSCENE::STOP_CUTSCENE_IMMEDIATELY();
			});

			components::button("REFRESH_INTERIOR"_T, [] {
				Interior interior = INTERIOR::GET_INTERIOR_AT_COORDS(self::pos.x, self::pos.y, self::pos.z);
				INTERIOR::REFRESH_INTERIOR(interior);
			});

			components::button("NET_SHUTDOWN_AND_LOAD_SP"_T, [] {
				NETWORK::SHUTDOWN_AND_LAUNCH_SINGLE_PLAYER_GAME();
			});

			components::button("NET_SHUTDOWN_AND_LOAD_SAVE"_T, [] {
				NETWORK::SHUTDOWN_AND_LOAD_MOST_RECENT_SAVE();
			});

			components::button("REMOVE_BLACKSCREEN"_T, [] {
				CAM::DO_SCREEN_FADE_IN(0);
				PLAYER::SET_PLAYER_CONTROL(self::id, true, 0);
				ENTITY::FREEZE_ENTITY_POSITION(self::ped, false);
				MISC::FORCE_GAME_STATE_PLAYING();
				if (self::veh == 0)
					TASK::CLEAR_PED_TASKS_IMMEDIATELY(self::ped);
				HUD::DISPLAY_RADAR(true);
				HUD::DISPLAY_HUD(true);
			});

			components::button("TP_TO_SAFE_POS"_T, [] {
				Vector3 safepos{};
				float heading;
				if (pathfind::find_closest_vehicle_node(self::pos, safepos, heading, 0))
					ENTITY::SET_ENTITY_COORDS(self::ped, safepos.x, safepos.y, safepos.z, 0, 0, 0, false);
				else
					g_notification_service.push_error("DEBUG_TAB_MISC"_T.data(), "VIEW_DEBUG_MISC_TP_TO_SAFE_POS_FAILED"_T.data());
			});

			ImGui::Checkbox("VIEW_DEBUG_MISC_IMGUI_DEMO"_T.data(), &g.window.demo);

			components::command_button<"fastquit">();

			if (ImGui::TreeNode("VIEW_DEBUG_MISC_FUZZER"_T.data()))
			{
				ImGui::Checkbox("ENABLED"_T.data(), &g.debug.fuzzer.enabled);

				for (int i = 0; i < net_object_type_strs.size(); i++)
				{
					ImGui::Checkbox(net_object_type_strs[i], &g.debug.fuzzer.enabled_object_types[i]);
				}

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("ADDRESSES"_T.data()))
			{
				uint64_t local_cped = (uint64_t)g_local_player;
				ImGui::InputScalar("LOCAL_CPED"_T.data(), ImGuiDataType_U64, &local_cped, NULL, NULL, "%p", ImGuiInputTextFlags_CharsHexadecimal);

				if (g_local_player)
				{
					uint64_t local_playerinfo = (uint64_t)g_local_player->m_player_info;
					ImGui::InputScalar("LOCAL_CPLAYERINFO"_T.data(), ImGuiDataType_U64, &local_playerinfo, NULL, NULL, "%p", ImGuiInputTextFlags_CharsHexadecimal);

					uint64_t local_vehicle = (uint64_t)g_local_player->m_vehicle;
					ImGui::InputScalar("LOCAL_CAUTOMOBILE"_T.data(), ImGuiDataType_U64, &local_vehicle, NULL, NULL, "%p", ImGuiInputTextFlags_CharsHexadecimal);
				}

				if (auto mgr = *g_pointers->m_gta.m_network_player_mgr)
				{
					uint64_t local_netplayer = (uint64_t)mgr->m_local_net_player;
					ImGui::InputScalar("LOCAL_CNETGAMEPLAYER"_T.data(), ImGuiDataType_U64, &local_netplayer, NULL, NULL, "%p", ImGuiInputTextFlags_CharsHexadecimal);

					if (mgr->m_local_net_player)
					{
						uint64_t local_netplayer = (uint64_t)mgr->m_local_net_player->get_net_data();
						ImGui::InputScalar("LOCAL_NETPLAYERDATA"_T.data(), ImGuiDataType_U64, &local_netplayer, NULL, NULL, "%p", ImGuiInputTextFlags_CharsHexadecimal);
					}
				}

				if (auto network = *g_pointers->m_gta.m_network)
				{
					uint64_t nw = (uint64_t)network;
					ImGui::InputScalar("NETWORK"_T.data(), ImGuiDataType_U64, &nw, NULL, NULL, "%p", ImGuiInputTextFlags_CharsHexadecimal);
				}

				if (auto omgr = *g_pointers->m_gta.m_network_object_mgr)
				{
					uint64_t nw = (uint64_t)omgr;
					ImGui::InputScalar("NETWORK_OBJ_MGR"_T.data(), ImGuiDataType_U64, &nw, NULL, NULL, "%p", ImGuiInputTextFlags_CharsHexadecimal);
				}

				ImGui::TreePop();
			}

			ImGui::EndTabItem();
		}
	}
}
