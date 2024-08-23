#include "backend/bool_command.hpp"
#include "core/data/apartment_names.hpp"
#include "core/data/region_codes.hpp"
#include "core/data/warehouse_names.hpp"
#include "gta_util.hpp"
#include "util/session.hpp"
#include "util/toxic.hpp"
#include "views/view.hpp"

#include <network/Network.hpp>
#include <script/globals/GPBD_FM_3.hpp>


namespace big
{
	struct SessionType
	{
		eSessionType id;
		const char* name;
	};

	static uint64_t rid = 0;

	void render_join_game()
	{
		ImGui::SeparatorText("JOIN_GAME"_T.data());

		ImGui::BeginGroup();

		ImGui::SetNextItemWidth(200);
		bool rid_submitted = ImGui::InputScalar("##inputrid", ImGuiDataType_U64, &rid, nullptr, nullptr, nullptr, ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::SameLine();
		if (components::button("JOIN_BY_RID"_T) || rid_submitted)
		{
			const auto rid_lambda = rid;
			g_fiber_pool->queue_job([rid_lambda]() {
				session::join_by_rockstar_id(rid_lambda);
			});
		}

		ImGui::SetNextItemWidth(200);

		static std::string username;
		bool username_submitted = components::input_text_with_hint("##usernameinput", "INPUT_USERNAME"_T, username, ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::SameLine();
		if (components::button("JOIN_BY_USERNAME"_T) || username_submitted)
		{
			session::join_by_username(username);
		}

		static char base64[500]{};
		ImGui::SetNextItemWidth(200);
		components::input_text_with_hint("##sessioninfoinput", "SESSION_INFO"_T, base64, sizeof(base64));
		ImGui::SameLine();
		components::button("JOIN_SESSION_INFO"_T, [] {
			rage::rlSessionInfo info;
			if (g_pointers->m_gta.m_decode_session_info(&info, base64, nullptr))
				session::join_session(info);
			else
				g_notification_service.push_error("RID_JOINER"_T.data(), "VIEW_NET_RIDJOINER_SESSION_INFO_INVALID"_T.data());
		});

		components::button("COPY_SESSION_INFO"_T, [] {
			char buf[0x100]{};
			g_pointers->m_gta.m_encode_session_info(&gta_util::get_network()->m_last_joined_session.m_session_info, buf, 0xA9, nullptr);
			ImGui::SetClipboardText(buf);
		});

		ImGui::Checkbox("JOIN_IN_SCTV"_T.data(), &g.session.join_in_sctv_slots);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("JOIN_IN_SCTV_DESC"_T.data());

		ImGui::Checkbox("PLAYER_MAGNET"_T.data(), &g.session.player_magnet_enabled);
		if (g.session.player_magnet_enabled)
		{
			ImGui::Text("PLAYER_COUNT"_T.data());
			ImGui::InputInt("##playercount", &g.session.player_magnet_count);
		}
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		std::string region_str = regions[*g_pointers->m_gta.m_region_code].name;

		ImGui::SetNextItemWidth(250);
		if (ImGui::BeginCombo("##regionswitcher", region_str.c_str()))
		{
			for (const auto& region_type : regions)
			{
				components::selectable(region_type.name, *g_pointers->m_gta.m_region_code == region_type.id, [&region_type] {
					*g_pointers->m_gta.m_region_code = region_type.id;
				});
			}
			ImGui::EndCombo();
		}

		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("SESSION_SELECT_COMBO_DESC"_T.data());
		}

		ImGui::Separator();

		static constexpr auto sessions = std::to_array<SessionType>({// This has to be here because if it's generated at compile time, the translations break for some reason.
		    {eSessionType::JOIN_PUBLIC, "BACKEND_SESSION_TYPE_JOIN_PUBLIC"},
		    {eSessionType::NEW_PUBLIC, "BACKEND_SESSION_TYPE_NEW_PUBLIC"},
		    {eSessionType::CLOSED_CREW, "BACKEND_SESSION_TYPE_CLOSED_CREW"},
		    {eSessionType::CREW, "BACKEND_SESSION_TYPE_CREW"},
		    {eSessionType::CLOSED_FRIENDS, "BACKEND_SESSION_TYPE_CLOSED_FRIENDS"},
		    {eSessionType::FIND_FRIEND, "BACKEND_SESSION_TYPE_FIND_FRIEND"},
		    {eSessionType::SOLO, "BACKEND_SESSION_TYPE_SOLO"},
		    {eSessionType::INVITE_ONLY, "BACKEND_SESSION_TYPE_INVITE_ONLY"},
		    {eSessionType::JOIN_CREW, "BACKEND_SESSION_TYPE_JOIN_CREW"},
		    {eSessionType::SC_TV, "BACKEND_SESSION_TYPE_SC_TV"},
		    {eSessionType::LEAVE_ONLINE, "BACKEND_SESSION_TYPE_LEAVE_ONLINE"}});

		for (const auto& [id, name] : sessions)
		{
			if (id == eSessionType::LEAVE_ONLINE && !*g_pointers->m_gta.m_is_session_started) // Don't show a Leave Online option in single player (it actually sends us INTO online)
				continue;

			components::selectable(g_translation_service.get_translation(name), false, [&id] {
				session::join_type(id);
			});
		}

		ImGui::EndGroup();
	}

	bool_command whitelist_friends("trustfriends", "TRUST_FRIENDS", "TRUST_FRIENDS_DESC", g.session.trust_friends);
	bool_command whitelist_session("trustsession", "TRUST_SESSION", "TRUST_SESSION_DESC", g.session.trust_session);
	bool_command
	    chat_translate("translatechat", "TRANSLATOR_TOGGLE", "TRANSLATOR_TOGGLE_DESC", g.session.chat_translator.enabled);

	void render_general_options()
	{
		ImGui::SeparatorText("PLAYER_TOGGLES"_T.data());

		ImGui::BeginGroup();
		components::command_checkbox<"trustfriends">();
		components::command_checkbox<"trustsession">();
		components::script_patch_checkbox("REVEAL_OTR_PLAYERS"_T,
		    &g.session.decloak_players,
		    "REVEAL_OTR_PLAYERS_DESC"_T.data());
		components::script_patch_checkbox("REVEAL_HIDDEN_PLAYERS"_T,
		    &g.session.unhide_players_from_player_list,
		    "REVEAL_HIDDEN_PLAYERS_DESC"_T.data());
		components::command_button<"sextall">({}, "SEND_SEXT"_T);
		components::command_button<"fakebanall">({}, "FAKE_BAN_MESSAGE"_T);
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		ImGui::Checkbox("OFF_THE_RADAR"_T.data(), &g.session.off_radar_all);
		ImGui::Checkbox("NEVER_WANTED"_T.data(), &g.session.never_wanted_all);
		ImGui::Checkbox("SEMI_GODMODE"_T.data(), &g.session.semi_godmode_all);
		ImGui::Checkbox("VIEW_NET_SESSION_FIX_VEHICLE"_T.data(), &g.session.vehicle_fix_all);
		components::command_checkbox<"harass">();
		ImGui::Checkbox("SPAM_KILLFEED"_T.data(), &g.session.spam_killfeed);
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		ImGui::Checkbox("EXPLOSION_KARMA"_T.data(), &g.session.explosion_karma);
		ImGui::Checkbox("DAMAGE_KARMA"_T.data(), &g.session.damage_karma);
		ImGui::Checkbox("DISABLE_PEDS"_T.data(), &g.session.disable_peds);
		ImGui::Checkbox("DISABLE_TRAFFIC"_T.data(), &g.session.disable_traffic);
		ImGui::Checkbox("FORCE_THUNDER"_T.data(), &g.session.force_thunder);
		ImGui::Checkbox("LOBBY_LOCK"_T.data(), &g.session.lock_session);
		if (g.session.lock_session)
		{
			ImGui::Checkbox("LOBBY_LOCK_ALLOW_FRIENDS"_T.data(), &g.session.allow_friends_into_locked_session);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("LOBBY_LOCK_ALLOW_FRIENDS_DESC"_T.data());
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("LOBBY_LOCK_DESC"_T.data());
		ImGui::EndGroup();
	}

	void render_teleport_options()
	{
		ImGui::SeparatorText("TELEPORTS"_T.data());

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

		components::command_button<"apartmenttpall">({(uint64_t)g.session.send_to_apartment_idx}, "TP_ALL_TO_APARTMENT"_T);

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

		components::command_button<"warehousetpall">({(uint64_t)g.session.send_to_warehouse_idx}, "TP_ALL_TO_WAREHOUSE"_T);

		ImGui::BeginGroup();
		components::button("TP_ALL_TO_DARTS"_T, [] {
			g_player_service->iterate([](auto& plyr) {
				toxic::start_activity(plyr.second, eActivityType::Darts);
			});
		});
		components::button("TP_ALL_TO_FLIGHT_SCHOOL"_T, [] {
			g_player_service->iterate([](auto& plyr) {
				toxic::start_activity(plyr.second, eActivityType::PilotSchool);
			});
		});
		components::button("TP_ALL_TO_MAP_CENTER"_T, [] {
			g_player_service->iterate([](auto& plyr) {
				toxic::start_activity(plyr.second, eActivityType::ArmWresling);
			});
		});
		components::button("TP_ALL_TO_SKYDIVE"_T, [] {
			g_player_service->iterate([](auto& plyr) {
				toxic::start_activity(plyr.second, eActivityType::Skydive);
			});
		});
		components::command_button<"interiortpall">({81}, "TP_ALL_TO_MOC"_T);
		components::command_button<"interiortpall">({123}, "TP_ALL_TO_CASINO"_T);
		components::command_button<"interiortpall">({124}, "TP_ALL_TO_PENTHOUSE"_T);
		components::command_button<"interiortpall">({128}, "TP_ALL_TO_ARCADE"_T);
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		components::command_button<"interiortpall">({146}, "TP_ALL_TO_MUSIC_LOCKER"_T);
		components::command_button<"interiortpall">({148}, "TP_ALL_TO_RECORD_A_STUDIOS"_T);
		components::command_button<"interiortpall">({149}, "TP_ALL_TO_CUSTOM_AUTO_SHOP"_T);
		components::command_button<"interiortpall">({155}, "TP_ALL_TO_AGENCY"_T);
		components::command_button<"interiortpall">({160}, "TP_ALL_TO_FREAKSHOP"_T);
		components::command_button<"interiortpall">({161}, "TP_ALL_TO_MULTI_FLOOR_GARAGE"_T);
		components::command_button<"tutorialall">();
		components::command_button<"golfall">();
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		components::command_button<"flightschoolall">();
		components::command_button<"dartsall">();
		components::command_button<"badlandsall">();
		components::command_button<"spacemonkeyall">();
		components::command_button<"wizardall">();
		components::command_button<"qub3dall">();
		components::command_button<"camhedzall">();
		ImGui::EndGroup();
	}

	void view::network()
	{
		render_join_game();
		render_general_options();
		render_teleport_options();
	}
}
