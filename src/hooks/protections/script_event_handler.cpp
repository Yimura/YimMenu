#include "gta/net_game_event.hpp"
#include "gta_util.hpp"
#include "hooking/hooking.hpp"
#include "lua/lua_manager.hpp"
#include "util/math.hpp"
#include "util/protection.hpp"
#include "util/session.hpp"

#include <network/CNetGamePlayer.hpp>
#include <script/globals/GPBD_FM_3.hpp>
#include <script/globals/GlobalPlayerBD.hpp>

namespace big
{
	void format_string(std::string_view player_name, std::string_view protection_type, bool should_log, bool should_notify)
	{
		if (should_log)
			LOG(WARNING) << "BLOCKED_SCRIPT_EVENT From: " << player_name << " Event Type: " << protection_type;

		if (should_notify)
			g_notification_service.push_warning("Script Event Protection",
			    std::format("From: {}\nEvent Type: {}", player_name.data(), protection_type.data()));
	}

	inline bool is_player_driver_of_local_vehicle(Player sender)
	{
		auto plyr = g_player_service->get_by_id(sender);

		if (!plyr || !plyr->get_current_vehicle() || !g_player_service->get_self()->get_current_vehicle())
			return false;

		return g_player_service->get_self()->get_current_vehicle()->m_driver == plyr->get_ped();
	}

	inline bool is_player_our_goon(Player sender)
	{
		auto& boss_goon = scr_globals::gpbd_fm_3.as<GPBD_FM_3*>()->Entries[self::id].BossGoon;

		if (boss_goon.Boss != self::id)
			return false;

		for (int i = 0; i < boss_goon.Goons.Size; i++)
		{
			if (boss_goon.Goons[i] == sender)
			{
				return true;
			}
		}

		return false;
	}

	inline bool is_player_our_boss(Player sender)
	{
		return sender == scr_globals::gpbd_fm_3.as<GPBD_FM_3*>()->Entries[self::id].BossGoon.Boss;
	}

	bool hooks::scripted_game_event(CScriptedGameEvent* scripted_game_event, CNetGamePlayer* player)
	{
		const auto args       = scripted_game_event->m_args;
		const auto args_count = scripted_game_event->m_args_size / 8;

		const auto hash        = static_cast<eRemoteEvent>(args[0]);
		const auto player_name = player->get_name();

		auto plyr = g_player_service->get_by_id(player->m_player_id);

		if (g_lua_manager && g_lua_manager->get_module_count() > 0)
		{
			std::vector<int32_t> script_event_args;

			script_event_args.reserve(args_count);
			for (int i = 0; i < args_count; i++)
				script_event_args.push_back(args[i]);

			auto event_ret = g_lua_manager->trigger_event<menu_event::ScriptedGameEventReceived, bool>((int)player->m_player_id, script_event_args);
			if (event_ret.has_value())
				return true; // don't care, block event if any bool is returned
		}

		switch (hash)
		{
		case eRemoteEvent::Bounty:
			if (g.protections.script_events.bounty && args[3] == self::id)
			{
				g.reactions.bounty.process(plyr);
				return true;
			}
			break;
		case eRemoteEvent::CeoKick:
			if (player->m_player_id != scr_globals::gpbd_fm_3.as<GPBD_FM_3*>()->Entries[self::id].BossGoon.Boss)
			{
				g.reactions.ceo_kick.process(plyr);
				return true;
			}
			break;
		case eRemoteEvent::CeoMoney:
			if (g.protections.script_events.ceo_money
			    && player->m_player_id != scr_globals::gpbd_fm_3.as<GPBD_FM_3*>()->Entries[self::id].BossGoon.Boss)
			{
				g.reactions.ceo_money.process(plyr);
				return true;
			}
			break;
		case eRemoteEvent::ClearWantedLevel:
			if (g.protections.script_events.clear_wanted_level && !is_player_driver_of_local_vehicle(player->m_player_id))
			{
				g.reactions.clear_wanted_level.process(plyr);
				return true;
			}
			break;
		case eRemoteEvent::Crash: g.reactions.crash.process(plyr); return true;
		case eRemoteEvent::Crash2:
			if (args[3] > 32) // actual crash condition is if args[2] is above 255
			{
				g.reactions.crash.process(plyr);
				return true;
			}
			break;
		case eRemoteEvent::Crash3:
		{
			if (isnan(*(float*)&args[3]) || isnan(*(float*)&args[4]) || isnan(*(float*)&args[5]))
			{
				session::add_infraction(plyr, Infraction::TRIED_CRASH_PLAYER);
				g.reactions.crash.process(plyr);
				return true;
			}
			if (args[3] == -4640169 && args[7] == -36565476 && args[8] == -53105203)
			{
				session::add_infraction(plyr, Infraction::TRIED_CRASH_PLAYER);
				g.reactions.crash.process(plyr);

				return true;
			}
			break;
		}
		case eRemoteEvent::Notification:
		{
			switch (static_cast<eRemoteEvent>(args[3]))
			{
			case eRemoteEvent::NotificationMoneyBanked: // never used
			case eRemoteEvent::NotificationMoneyRemoved:
			case eRemoteEvent::NotificationMoneyStolen: g.reactions.fake_deposit.process(plyr); return true;
			case eRemoteEvent::NotificationCrash1:                             // this isn't used by the game
				session::add_infraction(plyr, Infraction::TRIED_CRASH_PLAYER); // stand user detected
				return true;
			case eRemoteEvent::NotificationCrash2:
				if (!gta_util::find_script_thread("gb_salvage"_J))
				{
					// This looks like it's meant to trigger a sound crash by spamming too many notifications. We've already patched it, but the notifications are still annoying
					session::add_infraction(plyr, Infraction::TRIED_CRASH_PLAYER); // stand user detected
					return true;
				}
				break;
			}

			break;
		}
		case eRemoteEvent::ForceMission:
			if (g.protections.script_events.force_mission)
			{
				g.reactions.force_mission.process(plyr);
				return true;
			}
			break;
		case eRemoteEvent::GiveCollectible:
			if (g.protections.script_events.give_collectible)
			{
				g.reactions.give_collectible.process(plyr);
				return true;
			}
			break;
		case eRemoteEvent::GtaBanner:
			if (g.protections.script_events.gta_banner)
			{
				g.reactions.gta_banner.process(plyr);
				return true;
			}
			break;
		case eRemoteEvent::MCTeleport:
			if (g.protections.script_events.mc_teleport && args[4] <= 32 && !is_player_our_boss(plyr->id()))
			{
				for (int i = 0; i < 32; i++)
				{
					if (args[5 + i] == NETWORK::NETWORK_HASH_FROM_PLAYER_HANDLE(self::id))
					{
						g.reactions.mc_teleport.process(plyr);
						return true;
					}
				}
			}
			else if (args[4] > 32)
			{
				g.reactions.crash.process(plyr);
				return true;
			}
			break;
		case eRemoteEvent::PersonalVehicleDestroyed:
			if (g.protections.script_events.personal_vehicle_destroyed)
			{
				g.reactions.personal_vehicle_destroyed.process(plyr);
				return true;
			}
			break;
		case eRemoteEvent::RemoteOffradar:
			if (g.protections.script_events.remote_off_radar && !is_player_our_boss(plyr->id()) && !is_player_driver_of_local_vehicle(plyr->id()))
			{
				g.reactions.remote_off_radar.process(plyr);
				return true;
			}
			break;
		case eRemoteEvent::TSECommand:
		{
			if (NETWORK::NETWORK_IS_ACTIVITY_SESSION())
				break;

			if (g.protections.script_events.rotate_cam && static_cast<eRemoteEvent>(args[3]) == eRemoteEvent::TSECommandRotateCam)
			{
				g.reactions.rotate_cam.process(plyr);
				return true;
			}

			if (g.protections.script_events.sound_spam && static_cast<eRemoteEvent>(args[3]) == eRemoteEvent::TSECommandSound)
			{
				if (!plyr || plyr->m_play_sound_rate_limit_tse.process())
				{
					if (plyr->m_play_sound_rate_limit_tse.exceeded_last_process())
						g.reactions.sound_spam.process(plyr);
					return true;
				}
			}

			break;
		}
		case eRemoteEvent::SendToCayoPerico:
			if (g.protections.script_events.send_to_location && args[4] == 0)
			{
				g.reactions.send_to_location.process(plyr);
				return true;
			}
			break;
		case eRemoteEvent::SendToCutscene:
			if (g.protections.script_events.send_to_cutscene && !is_player_our_boss(plyr->id()))
			{
				g.reactions.send_to_cutscene.process(plyr);
				return true;
			}
			break;
		case eRemoteEvent::SendToLocation:
		{
			if (is_player_our_boss(plyr->id()))
				break;

			bool known_location = false;

			if (args[3] == 0 && args[4] == 0)
			{
				if (args[5] == 4 && args[6] == 0)
				{
					known_location = true;

					if (g.protections.script_events.send_to_location)
					{
						g.reactions.send_to_location.process(plyr);
						return true;
					}
				}
				else if ((args[5] == 3 || args[5] == 4) && args[6] == 1)
				{
					known_location = true;

					if (g.protections.script_events.send_to_location)
					{
						g.reactions.send_to_location.process(plyr);
						return true;
					}
				}
			}

			if (!known_location)
			{
				g.reactions.tse_freeze.process(plyr);
				return true;
			}
			break;
		}
		case eRemoteEvent::SoundSpam:
		{
			if (g.protections.script_events.sound_spam && (!plyr || plyr->m_invites_rate_limit.process()))
			{
				if (plyr->m_invites_rate_limit.exceeded_last_process())
					g.reactions.sound_spam.process(plyr);
				return true;
			}
			break;
		}
		case eRemoteEvent::Spectate:
			if (g.protections.script_events.spectate)
			{
				g.reactions.spectate_notification.process(plyr);
				return true;
			}
			break;
		case eRemoteEvent::Teleport:
			if (g.protections.script_events.force_teleport && !is_player_driver_of_local_vehicle(player->m_player_id))
			{
				g.reactions.force_teleport.process(plyr);
				return true;
			}
			break;
		case eRemoteEvent::TransactionError: g.reactions.transaction_error.process(plyr); return true;
		case eRemoteEvent::VehicleKick:
			if (g.protections.script_events.vehicle_kick)
			{
				g.reactions.vehicle_kick.process(plyr);
				return true;
			}
			break;
		case eRemoteEvent::NetworkBail:
			session::add_infraction(plyr, Infraction::TRIED_KICK_PLAYER);
			g.reactions.kick.process(plyr);
			return true;
		case eRemoteEvent::TeleportToWarehouse:
			if (g.protections.script_events.teleport_to_warehouse && !is_player_driver_of_local_vehicle(player->m_player_id))
			{
				g.reactions.teleport_to_warehouse.process(plyr);
				return true;
			}
			break;
		case eRemoteEvent::StartActivity:
		{
			eActivityType activity = static_cast<eActivityType>(args[3]);
			if (g.protections.script_events.start_activity)
			{
				if (activity == eActivityType::Survival || activity == eActivityType::Mission || activity == eActivityType::Deathmatch || activity == eActivityType::BaseJump || activity == eActivityType::Race)
				{
					g.reactions.tse_freeze.process(plyr);
					return true;
				}
				else if (activity == eActivityType::Darts)
				{
					g.reactions.start_activity.process(plyr);
					return true;
				}
				else if (activity == eActivityType::PilotSchool)
				{
					g.reactions.start_activity.process(plyr);
					return true;
				}
				else if (activity == eActivityType::ImpromptuDeathmatch)
				{
					g.reactions.start_activity.process(plyr);
					return true;
				}
				else if (activity == eActivityType::DefendSpecialCargo || activity == eActivityType::GunrunningDefend || activity == eActivityType::BikerDefend || args[3] == 238)
				{
					g.reactions.trigger_business_raid.process(plyr);
					return true;
				}
			}
			else if (activity == eActivityType::Tennis)
			{
				g.reactions.crash.process(plyr);
				return true;
			}

			if (g.protections.script_events.start_activity && !is_player_our_goon(player->m_player_id))
			{
				g.reactions.start_activity.process(plyr);
				return true;
			}

			break;
		}
		case eRemoteEvent::InteriorControl:
		{
			int interior = (int)args[3];
			if (interior < 0 || interior > 171) // the upper bound will change after an update
			{
				if (auto plyr = g_player_service->get_by_id(player->m_player_id))
					session::add_infraction(plyr, Infraction::TRIED_KICK_PLAYER);
				g.reactions.kick.process(plyr);
				return true;
			}

			if (NETWORK::NETWORK_IS_ACTIVITY_SESSION())
				break;

			if (!g_local_player)
				break;

			if (is_player_our_boss(plyr->id()))
				break;

			if (is_player_driver_of_local_vehicle(plyr->id()))
				break;

			if (!plyr->get_ped() || math::distance_between_vectors(*plyr->get_ped()->get_position(), *g_local_player->get_position()) > 75.0f)
			{
				// g.reactions.send_to_interior.process(plyr); false positives
				return true; // this is fine, the game will reject our false positives anyway
			}

			break;
		}
		case eRemoteEvent::DestroyPersonalVehicle: g.reactions.destroy_personal_vehicle.process(plyr); return true;
		case eRemoteEvent::KickFromInterior:
			if (scr_globals::globalplayer_bd.as<GlobalPlayerBD*>()->Entries[self::id].SimpleInteriorData.Owner != plyr->id())
			{
				g.reactions.kick_from_interior.process(plyr);
				return true;
			}
			break;
		case eRemoteEvent::TriggerCEORaid:
		{
			if (auto script = gta_util::find_script_thread("freemode"_J))
			{
				if (script->m_net_component && ((CGameScriptHandlerNetComponent*)script->m_net_component)->m_host
				    && ((CGameScriptHandlerNetComponent*)script->m_net_component)->m_host->m_net_game_player != player)
				{
					g.reactions.trigger_business_raid.process(plyr);
				}
			}

			return true;
		}
		case eRemoteEvent::StartScriptProceed:
		{
			// TODO: Breaks stuff
			if (auto script = gta_util::find_script_thread("freemode"_J))
			{
				if (script->m_net_component && ((CGameScriptHandlerNetComponent*)script->m_net_component)->m_host
				    && ((CGameScriptHandlerNetComponent*)script->m_net_component)->m_host->m_net_game_player != player)
				{
					g.reactions.start_script.process(plyr);
					return true;
				}
			}
			break;
		}
		case eRemoteEvent::StartScriptBegin:
		{
			auto script_id = args[3];

			if (!protection::should_allow_script_launch(script_id))
			{
				LOGF(stream::script_events, WARNING, "Blocked StartScriptBegin from {} with script ID {}", plyr->get_name(), script_id);
				g.reactions.start_script.process(plyr);
				return true;
			}
			else
			{
				LOGF(stream::script_events, INFO, "Allowed StartScriptBegin from {} with script ID {}", plyr->get_name(), script_id);
			}
		}
		}

		// detect pasted menus setting args[1] to something other than PLAYER_ID()
		if (*(int*)&args[1] != player->m_player_id && player->m_player_id != -1) [[unlikely]]
		{
			LOG(INFO) << "Hash = " << (int)args[0];
			LOG(INFO) << "Sender = " << args[1];
			g.reactions.tse_sender_mismatch.process(plyr);
			return true;
		}

		if (g.debug.logs.script_event.logs
		    && (!g.debug.logs.script_event.filter_player || g.debug.logs.script_event.player_id == player->m_player_id)) [[unlikely]]
		{
			std::stringstream output;
			output << "Script Event From: " << player->get_name() << " (" << plyr->get_rockstar_id() << ") Args: { ";
			for (int i = 0; i < args_count; i++)
			{
				if (i)
					output << ", ";

				output << (int)args[i];
			}
			output << " }; ";

			auto now        = std::chrono::system_clock::now();
			auto ms         = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
			auto timer      = std::chrono::system_clock::to_time_t(now);
			auto local_time = *std::localtime(&timer);

			static std::ofstream log(g_file_manager.get_project_file("./script_events.log").get_path(), std::ios::app);
			log << "[" << std::put_time(&local_time, "%m/%d/%Y %I:%M:%S") << ":" << std::setfill('0') << std::setw(3) << ms.count() << " " << std::put_time(&local_time, "%p") << "] "
			    << output.str() << std::endl;
			log.flush();
		}

		if (g.debug.logs.script_event.block_all) [[unlikely]]
			return true;

		return false;
	}
}
