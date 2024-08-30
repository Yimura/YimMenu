#include "core/data/command_access_levels.hpp"
#include "core/data/language_codes.hpp"
#include "core/scr_globals.hpp"
#include "services/gta_data/gta_data_service.hpp"
#include "services/player_database/player_database_service.hpp"
#include "util/session.hpp"
#include "views/view.hpp"

#include <network/netConnection.hpp>
#include <script/globals/GPBD_FM.hpp>
#include <script/globals/GPBD_FM_3.hpp>
#include <vehicle/CVehicleModelInfo.hpp>

namespace big
{
	const char* get_nat_type_str(int type)
	{
		switch (type)
		{
		case 1: return "VIEW_PLAYER_INFO_NAT_TYPE_OPEN"_T.data();
		case 2: return "VIEW_PLAYER_INFO_NAT_TYPE_MODERATE"_T.data();
		case 3: return "VIEW_PLAYER_INFO_NAT_TYPE_STRICT"_T.data();
		}

		return "VIEW_NET_PLAYER_DB_GAME_MODE_UNKNOWN"_T.data();
	}

	const char* get_connection_type_str(int type)
	{
		switch (type)
		{
		case 1: return "VIEW_PLAYER_INFO_CONNECTION_TYPE_DIRECT"_T.data();
		case 2: return "VIEW_PLAYER_INFO_CONNECTION_TYPE_RELAY"_T.data();
		case 3: return "VIEW_PLAYER_INFO_CONNECTION_TYPE_PEER_RELAY"_T.data();
		}

		return "VIEW_NET_PLAYER_DB_GAME_MODE_UNKNOWN"_T.data();
	}

	void view::player_info()
	{
		ImGui::SeparatorText("INFO"_T.data());

		uint32_t ped_damage_bits = 0;
		uint32_t ped_task_flag   = 0;
		uint32_t ped_health      = 0;
		uint32_t ped_maxhealth   = 0;
		uint32_t veh_damage_bits = 0;
		std::string mode_str{};
		std::string vehicle_name{};

		if (CPed* ped = g_player_service->get_selected()->get_ped(); ped != nullptr)
		{
			ped_damage_bits = ped->m_damage_bits;
			ped_task_flag   = ped->m_ped_task_flag;
			ped_health      = ped->m_health;
			ped_maxhealth   = ped->m_maxhealth;
		}

		components::options_modal(
		    "VIEW_PLAYER_INFO_EXTRA_INFO"_T.data(),
		    [ped_health, ped_maxhealth] {
			    ImGui::BeginGroup();

			    auto id = g_player_service->get_selected()->id();

			    if (id != -1)
			    {
				    auto& stats     = scr_globals::gpbd_fm_1.as<GPBD_FM*>()->Entries[id].PlayerStats;
				    auto& boss_goon = scr_globals::gpbd_fm_3.as<GPBD_FM_3*>()->Entries[id].BossGoon;

				    const auto money  = reinterpret_cast<uint64_t&>(stats.Money);
				    const auto wallet = reinterpret_cast<uint64_t&>(stats.WalletBalance);

				    if (boss_goon.Language >= 0 && boss_goon.Language < 13)
					    ImGui::Text("PLAYER_INFO_LANGUAGE"_T.data(), languages.at((eGameLanguage)boss_goon.Language).data());

				    ImGui::Text(std::format("{}: {}", "PLAYER_INFO_CEO_NAME"_T, boss_goon.GangName.Data).c_str());
				    ImGui::Text(std::format("{}: {}", "PLAYER_INFO_MC_NAME"_T, boss_goon.ClubhouseName.Data).c_str());
				    ImGui::Text(std::format("{}: {}", "PLAYER_INFO_WALLET"_T, wallet).c_str());
				    ImGui::Text(std::format("{}: {}", "PLAYER_INFO_BANK"_T, money - wallet).c_str());
				    ImGui::Text(std::format("{}: {}", "PLAYER_INFO_TOTAL_MONEY"_T, money).c_str());
				    ImGui::Text(
				        std::format("{}: {} ({} {})", "PLAYER_INFO_RANK"_T, stats.Rank, "PLAYER_INFO_RANK_RP"_T, stats.RP)
				            .c_str());
				    ImGui::Text(std::format("{}: {} ({} {})", "VIEW_PLAYER_INFO_HEALTH"_T, ped_health, "VIEW_PLAYER_INFO_MAXHEALTH"_T, ped_maxhealth)
				                    .c_str());
				    ImGui::Text(std::format("{}: {}", "PLAYER_INFO_KD"_T, stats.KdRatio).c_str());
				    ImGui::Text(std::format("{}: {}", "PLAYER_INFO_KILLS"_T, stats.KillsOnPlayers).c_str());
				    ImGui::Text(std::format("{}: {}", "PLAYER_INFO_DEATHS"_T, stats.DeathsByPlayers).c_str());
				    ImGui::Text(std::format("{}: {}", "PLAYER_INFO_PROSTITUTES"_T, stats.ProstitutesFrequented).c_str());
				    ImGui::Text(std::format("{}: {}", "PLAYER_INFO_LAP_DANCES"_T, stats.LapDancesBought).c_str());
				    ImGui::Text(std::format("{}: {}", "PLAYER_INFO_MISSIONS_CREATED"_T, stats.MissionsCreated).c_str());
				    auto meltdown_completed = scr_globals::gpbd_fm_1.as<GPBD_FM*>()->Entries[id].MeltdownComplete ? "YES"_T : "NO"_T;
				    ImGui::Text(std::format("{}: {}", "PLAYER_INFO_METLDOWN_COMPLETE"_T, meltdown_completed).c_str());
			    }

			    ImGui::EndGroup();

			    ImGui::SameLine();

			    ImGui::BeginGroup();

			    ImGui::Text(std::format("{}: {}",
			        "VIEW_PLAYER_INFO_NAT_TYPE"_T,
			        get_nat_type_str(g_player_service->get_selected()->get_net_data()->m_nat_type))
			                    .c_str());

			    if (auto peer = g_player_service->get_selected()->get_connection_peer())
			    {
				    ImGui::Text(std::format("{}: {}",
				        "VIEW_PLAYER_INFO_CONNECTION_TYPE"_T,
				        get_connection_type_str(peer->m_peer_address.m_connection_type))
				                    .c_str());

				    if (peer->m_peer_address.m_connection_type == 2)
				    {
					    auto ip = peer->m_relay_address.m_relay_address;
					    ImGui::Text(std::format("{}: {}.{}.{}.{}", "VIEW_PLAYER_INFO_RELAY_IP"_T, ip.m_field1, ip.m_field2, ip.m_field3, ip.m_field4)
					                    .c_str());
				    }
				    else if (peer->m_peer_address.m_connection_type == 3)
				    {
					    auto ip = peer->m_peer_address.m_relay_address;
					    ImGui::Text(std::format("{}: {}.{}.{}.{}", "VIEW_PLAYER_INFO_PEER_RELAY_IP"_T, ip.m_field1, ip.m_field2, ip.m_field3, ip.m_field4)
					                    .c_str());
				    }

				    ImGui::Text(
				        std::format("{}: {}", "VIEW_PLAYER_INFO_NUM_MESSAGES_SENT"_T, peer->m_num_messages_batched).c_str());
				    ImGui::Text(std::format("{}: {}", "VIEW_PLAYER_INFO_NUM_RELIABLES_SENT"_T, peer->m_num_reliable_messages_batched)
				                    .c_str());
				    ImGui::Text(std::format("{}: {}", "VIEW_PLAYER_INFO_NUM_RELIABLES_RESENT"_T, peer->m_num_resent_reliable_messages_batched)
				                    .c_str());
			    }

			    ImGui::Text(std::format("{}: {:X}",
			        "VIEW_PLAYER_INFO_HOST_TOKEN"_T,
			        g_player_service->get_selected()->get_net_data()->m_host_token)
			                    .c_str());
			    ImGui::SameLine();
			    if (ImGui::Button("Copy"))
			    {
				    ImGui::SetClipboardText(
				        std::format("{:X}", g_player_service->get_selected()->get_net_data()->m_host_token).data());
			    }

			    ImGui::EndGroup();

			    ImGui::Separator();

			    if (ImGui::Checkbox("TRUST"_T.data(), &g_player_service->get_selected()->is_trusted))
			    {
				    auto entry = g_player_database_service->get_or_create_player(g_player_service->get_selected());
				    entry->is_trusted = g_player_service->get_selected()->is_trusted;
				    g_player_database_service->save();
			    }
			    ImGui::Checkbox("VIEW_PLAYER_INFO_BLOCK_EXPLOSIONS"_T.data(), &g_player_service->get_selected()->block_explosions);
			    ImGui::Checkbox("VIEW_PLAYER_INFO_BLOCK_CLONE_CREATE"_T.data(), &g_player_service->get_selected()->block_clone_create);
			    ImGui::Checkbox("VIEW_PLAYER_INFO_BLOCK_CLONE_SYNC"_T.data(), &g_player_service->get_selected()->block_clone_sync);
			    ImGui::Checkbox("VIEW_PLAYER_INFO_BLOCK_NETWORK_EVENTS"_T.data(), &g_player_service->get_selected()->block_net_events);
				ImGui::Checkbox("VIEW_PLAYER_INFO_BLOCK_PTFX_EVENTS"_T.data(), &g_player_service->get_selected()->block_ptfx);
			    ImGui::Checkbox("VIEW_PLAYER_INFO_LOG_CLONES"_T.data(), &g_player_service->get_selected()->log_clones);

			    ImGui::Separator();

			    if (ImGui::BeginCombo("CHAT_COMMAND_PERMISSIONS"_T.data(),
			            COMMAND_ACCESS_LEVELS[g_player_service->get_selected()->command_access_level.value_or(g.session.chat_command_default_access_level)]))
			    {
				    for (const auto& [type, name] : COMMAND_ACCESS_LEVELS)
				    {
					    if (ImGui::Selectable(name,
					            type == g_player_service->get_selected()->command_access_level.value_or(g.session.chat_command_default_access_level)))
					    {
						    g.session.chat_command_default_access_level = type;
						    g_player_database_service->get_or_create_player(g_player_service->get_selected())->command_access_level = type;
						    g_player_database_service->save();
					    }

					    if (type == g_player_service->get_selected()->command_access_level.value_or(g.session.chat_command_default_access_level))
					    {
						    ImGui::SetItemDefaultFocus();
					    }
				    }

				    ImGui::EndCombo();
			    }
		    },
		    false,
		    "VIEW_PLAYER_INFO_EXTRA_INFO"_T.data());

		ImGui::SameLine();

		if (ImGui::SmallButton("PLAYER_INFO_ADD_TO_DB"_T.data()))
		{
			g_player_database_service->get_or_create_player(g_player_service->get_selected());
		}

		ImGui::SameLine();

		if (ImGui::SmallButton("VIEW_PLAYER_INFO_SC_PROFILE"_T.data()))
			session::show_profile_by_rockstar_id(g_player_service->get_selected()->get_net_data()->m_gamer_handle.m_rockstar_id);

		if (CPlayerInfo* player_info = g_player_service->get_selected()->get_player_info(); player_info != nullptr)
		{
			ImGui::Text(std::format("{}: {}", "WANTED_LEVEL"_T, player_info->m_wanted_level).c_str());
		}

		if (ped_damage_bits & (uint32_t)eEntityProofs::GOD)
		{
			mode_str = "PLAYER_INFO_GOD"_T;
		}
		else
		{
			if (ped_damage_bits & (uint32_t)eEntityProofs::BULLET)
			{
				mode_str += "PLAYER_INFO_BULLET"_T;
			}
			if (ped_damage_bits & (uint32_t)eEntityProofs::EXPLOSION)
			{
				if (!mode_str.empty())
					mode_str += " ";
				mode_str += "PLAYER_INFO_EXPLOSION"_T;
			}
			if (ped_health > 328 || ped_maxhealth > 328)
			{
				if (!mode_str.empty())
					mode_str += " ";
				mode_str += "VIEW_PLAYER_INFO_UNNATURAL_HEALTH"_T.data();
			}
		}

		if (mode_str.empty())
		{
			mode_str = "NO"_T;
		}

		ImGui::Text("PLAYER_INFO_PROOFS"_T.data(), mode_str.c_str());

		mode_str.clear();

		if (ped_task_flag & (uint8_t)ePedTask::TASK_DRIVING)
		{
			if (auto vehicle = g_player_service->get_selected()->get_current_vehicle(); vehicle != nullptr)
			{
				veh_damage_bits = vehicle->m_damage_bits;

				if (CVehicleModelInfo* vehicle_model_info = static_cast<CVehicleModelInfo*>(vehicle->m_model_info))
				{
					auto vehicle_item = g_gta_data_service.vehicles()[vehicle_model_info->m_hash];
					vehicle_name = g_gta_data_service.get_vehicle_full_name(vehicle_item);
				}

				if (veh_damage_bits & (uint32_t)eEntityProofs::GOD)
				{
					mode_str = "PLAYER_INFO_GOD"_T;
				}
				else
				{
					if (veh_damage_bits & (uint32_t)eEntityProofs::COLLISION)
					{
						mode_str += "PLAYER_INFO_COLLISION"_T;
					}
					if (veh_damage_bits & (uint32_t)eEntityProofs::EXPLOSION)
					{
						mode_str += "PLAYER_INFO_EXPLOSION"_T;
					}
				}

				if (mode_str.empty())
				{
					mode_str = "NO"_T;
				}
			}
		}
		else
		{
			vehicle_name = "PLAYER_INFO_NO_VEHICLE"_T;
		}

		ImGui::Text("PLAYER_INFO_VEHICLE"_T.data(), vehicle_name.c_str(), mode_str.c_str());

		if (auto net_player_data = g_player_service->get_selected()->get_net_data())
		{
			ImGui::Text("PLAYER_INFO_RID"_T.data(), net_player_data->m_gamer_handle.m_rockstar_id);

			ImGui::SameLine();

			ImGui::PushID("##rid");
			if (ImGui::SmallButton("COPY"_T.data()))
				ImGui::SetClipboardText(std::to_string(net_player_data->m_gamer_handle.m_rockstar_id).data());
			ImGui::PopID();

			auto ip   = g_player_service->get_selected()->get_ip_address();
			auto port = g_player_service->get_selected()->get_port();

			if (ip)
			{
				ImGui::Text("PLAYER_INFO_IP"_T.data(),
				    ip.value().m_field1,
				    ip.value().m_field2,
				    ip.value().m_field3,
				    ip.value().m_field4,
				    port);

				ImGui::SameLine();

				// clang-format off
				ImGui::PushID("##ip");
				if (ImGui::SmallButton("COPY"_T.data()))
					ImGui::SetClipboardText(std::format("{}.{}.{}.{}:{}",
						ip.value().m_field1,
						ip.value().m_field2,
						ip.value().m_field3,
						ip.value().m_field4,
						port).data());
				ImGui::PopID();
				// clang-format on
			}
			else
			{
				if (net_player_data->m_force_relays) // TODO: does this actually do anything?
					ImGui::Text("VIEW_PLAYER_INFO_IP_HIDDEN"_T.data());
				else
					ImGui::Text("VIEW_PLAYER_INFO_IP_UNKNOWN"_T.data());

				auto cxn_type = g_player_service->get_selected()->get_connection_peer() ?
				    g_player_service->get_selected()->get_connection_peer()->m_peer_address.m_connection_type :
				    0;

				if (g.protections.force_relay_connections && ImGui::IsItemHovered())
					ImGui::SetTooltip("VIEW_PLAYER_INFO_IP_FORCE_RELAY_TOOLTIP"_T.data());
				else if (cxn_type == 2 && ImGui::IsItemHovered())
					ImGui::SetTooltip("VIEW_PLAYER_INFO_IP_RELAY_TOOLTIP"_T.data());
				else if (cxn_type == 3 && ImGui::IsItemHovered())
					ImGui::SetTooltip("VIEW_PLAYER_INFO_IP_PEER_RELAY_TOOLTIP"_T.data());
			}
		}
	}
}
