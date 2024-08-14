#pragma once
#include "backend/reactions/interloper_reaction.hpp"
#include "backend/reactions/reaction.hpp"
#include "core/data/hud_colors.hpp"
#include "core/data/language_codes.hpp"
#include "core/data/ptfx_effects.hpp"
#include "enums.hpp"
#include "file_manager.hpp"

#include <bitset>
#include <ped/CPedBoneInfo.hpp>
#include <rage/rlSessionInfo.hpp>
#include <weapon/CAmmoInfo.hpp>
#include <weapon/CAmmoRocketInfo.hpp>
#include <weapon/CWeaponInfo.hpp>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#define TYPING_TICKS 3

class CNetGamePlayer;
enum class eNetObjType;

namespace rage
{
	class scrThread;
	class scrProgram;
}

namespace big
{
	struct color
	{
		int r;
		int g;
		int b;
		int a;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(color, r, g, b, a)
	};

	struct remote_player_teleport
	{
		std::int16_t m_player_net_id;
		rage::fvector3 m_position;
	};

	class menu_settings
	{
	public:
		void init(const file& save_file);

		void attempt_save();
		bool load();
		bool write_default_config();

	private:
		bool deep_compare(nlohmann::json& current_settings, const nlohmann::json& default_settings, bool compare_value = false);
		bool save();

	private:
		file m_save_file;

		nlohmann::json m_default_options;
		nlohmann::json m_options;

	public:
		int friend_count = 0;
		int player_count = 0;

		CNetGamePlayer* m_syncing_player  = nullptr;
		std::uint8_t m_sync_target_player  = -1;
		eNetObjType m_syncing_object_type = (eNetObjType)-1;

		int m_remote_controller_vehicle = -1;
		int m_remote_controlled_vehicle = -1;

		std::unordered_map<uint16_t, remote_player_teleport> m_remote_player_teleports;

		rage::scrThread* m_hunt_the_beast_thread = nullptr;

		rage::scrThread* m_mission_creator_thread = nullptr;

		struct script_block_opts
		{
			bool lsc = false;
			bool atms = false;
			bool interiors = false;
			bool drones    = false;
			bool strip_club = false;
			bool ammunation = false;
			bool stores     = false;
			bool sitting    = false;
			bool sleeping    = false;
			bool casino_games = false;
			bool arcade_games = false;
			bool prostitutes  = false;
			bool movies       = false;
			bool street_dealer = false;
			bool impromptu_dm   = false;
			bool impromptu_race = false;
			bool gang_attacks   = false;
			bool vending_machines = false;

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(script_block_opts, lsc, atms, interiors, drones, strip_club, ammunation, stores, sitting, sleeping, casino_games, arcade_games, prostitutes, movies, street_dealer, impromptu_dm, impromptu_race, gang_attacks, vending_machines);
		};

		struct cmd_executor
		{
			bool enabled = false;
		} cmd_executor{};

		bool in_script_vm = false;

		struct debug
		{
			struct logs
			{
				int metric_logs{};
				int packet_logs{};

				bool script_hook_logs{};

				struct script_event
				{
					bool logs = false;

					bool filter_player    = true;
					std::int8_t player_id = -1;

					bool block_all = false; //should not save

					NLOHMANN_DEFINE_TYPE_INTRUSIVE(script_event, logs, filter_player, player_id)
				} script_event{};

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(logs, metric_logs, packet_logs, script_hook_logs, script_event)
			} logs{};

			struct fuzzer
			{
				bool enabled = false;
				bool enabled_object_types[14];
				bool active                    = false;
				int thread_id                  = 0;
				std::int16_t syncing_object_id = -1;
			} fuzzer{};

			bool external_console = true;
			bool window_hook      = false;
			bool block_all_metrics = false;

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(debug, logs, external_console, window_hook, block_all_metrics)
		} debug{};

		struct tunables
		{
			bool disable_phone = false;
			bool no_idle_kick  = false;

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(tunables, disable_phone, no_idle_kick)
		} tunables{};

		struct notifications
		{
			struct pair
			{
				bool log    = false;
				bool notify = false;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(pair, log, notify)
			};

			pair gta_thread_kill{};
			pair gta_thread_start{};

			pair network_player_mgr_init{};
			pair network_player_mgr_shutdown{};

			struct player_join
			{
				bool above_map = true;
				bool log       = false;
				bool notify    = false;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(player_join, above_map, log, notify)
			} player_join{};

			pair player_leave{};

			pair send_net_info_to_lobby{};
			pair transaction_rate_limit{};

			bool warn_metric = false;

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(notifications, gta_thread_kill, gta_thread_start, network_player_mgr_init, network_player_mgr_shutdown, player_join, player_leave, send_net_info_to_lobby, transaction_rate_limit, warn_metric)
		} notifications{};

		struct reactions
		{
			// first constructor param is an internal identifier for the event
			reaction bounty{"BOUNTY", "REACTION_BOUNTY_NOTIFY", "REACTION_BOUNTY_ANNOUNCE"};
			reaction break_game{"BREAK_GAME", "REACTION_BREAK_GAME_NOTIFY", "REACTION_BREAK_GAME_ANNOUNCE"};
			reaction ceo_kick{"CEO_KICK", "REACTION_CEO_KICK_NOTIFY", "REACTION_CEO_KICK_ANNOUNCE"};
			reaction ceo_money{"CEO_MONEY", "REACTION_CEO_MONEY_NOTIFY", "REACTION_CEO_MONEY_ANNOUNCE"};
			reaction clear_wanted_level{"CLEAR_WANTED_LEVEL", "REACTION_CLEAR_WANTED_LEVEL_NOTIFY", "REACTION_CLEAR_WANTED_LEVEL_ANNOUNCE"};
			reaction crash{"CRASH", "REACTION_CRASH_NOTIFY", "REACTION_CRASH_ANNOUNCE"};
			reaction delete_vehicle{"DELETE_VEHICLE", "REACTION_DELETE_VEHICLE_NOTIFY", "REACTION_DELETE_VEHICLE_ANNOUNCE"};
			reaction fake_deposit{"FAKE_DEPOSIT", "REACTION_FAKE_DEPOSIT_NOTIFY", "REACTION_FAKE_DEPOSIT_ANNOUNCE"};
			reaction force_mission{"FORCE_MISSION", "REACTION_FORCE_MISSION_NOTIFY", "REACTION_FORCE_MISSION_ANNOUNCE"};
			reaction force_teleport{"FORCE_TELEPORT", "REACTION_FORCE_TELEPORT_NOTIFY", "REACTION_FORCE_TELEPORT_ANNOUNCE"};
			reaction gta_banner{"GTA_BANNER", "REACTION_GTA_BANNER_NOTIFY", "REACTION_GTA_BANNER_ANNOUNCE"}; // please don't enable this
			reaction kick{"KICKS", "REACTION_KICK_NOTIFY", "REACTION_GENERIC_KICK_ANNOUNCE"};
			reaction kick_from_interior{"KICK_FROM_INTERIOR", "REACTION_KICK_FROM_INTERIOR_NOTIFY", "REACTION_KICK_FROM_INTERIOR_ANNOUNCE"};
			reaction mc_teleport{"MC_TELEPORT", "REACTION_MC_TELEPORT_NOTIFY", "REACTION_MC_TELEPORT_ANNOUNCE"};
			reaction personal_vehicle_destroyed{"PERSONAL_VEHICLE_DESTROYED", "REACTION_PERSONAL_VEHICLE_DESTROYED_NOTIFY", "REACTION_PERSONAL_VEHICLE_DESTROYED_ANNOUNCE"};
			reaction remote_off_radar{"REMOTE_OFF_RADAR", "REACTION_OFF_RADAR_NOTIFY", "REACTION_OFF_RADAR_ANNOUNCE"};
			reaction rotate_cam{"ROTATE_CAM", "REACTION_ROTATE_CAM_NOTIFY", "REACTION_ROTATE_CAM_ANNOUNCE"};
			reaction send_to_cutscene{"SEND_TO_CUTSCENE", "REACTION_SEND_TO_CUTSCENE_NOTIFY", "REACTION_SEND_TO_CUTSCENE_ANNOUNCE"};
			reaction send_to_location{"SEND_TO_LOCATION", "REACTION_SEND_TO_LOCATION_NOTIFY", "REACTION_SEND_TO_LOCATION_ANNOUNCE"};
			reaction send_to_interior{"TP_TO_INTERIOR", "REACTION_SEND_TO_INTERIOR_NOTIFY", "REACTION_SEND_TO_INTERIOR_ANNOUNCE"};
			reaction sound_spam{"SOUND_SPAM", "REACTION_SOUND_SPAM_NOTIFY", "REACTION_SOUND_SPAM_ANNOUNCE"};
			reaction spectate_notification{"SPECTATE_NOTIFICATION", "REACTION_SPECTATE_NOTIFICATION_NOTIFY", "REACTION_SPECTATE_NOTIFICATION_ANNOUNCE"};
			reaction give_collectible{"GIVE_COLLECTIBLE", "REACTION_GIVE_COLLECTIBLE_NOTIFY", "REACTION_GIVE_COLLECTIBLE_ANNOUNCE"};
			reaction transaction_error{"SHOW_TRANSACTION_ERROR", "REACTION_TRANSACTION_ERROR_NOTIFY", "REACTION_TRANSACTION_ERROR_ANNOUNCE"};
			reaction tse_freeze{"TSE_FREEZE", "REACTION_TSE_FREEZE_NOTIFY", "REACTION_TSE_FREEZE_ANNOUNCE"};
			reaction tse_sender_mismatch{"TSE_SENDER_MISMATCH", "REACTION_TSE_SENDER_MISMATCH_NOTIFY", "REACTION_TSE_SENDER_MISMATCH_ANNOUNCE"};
			reaction vehicle_kick{"VEHICLE_KICK", "REACTION_VEHICLE_KICK_NOTIFY", "REACTION_VEHICLE_KICK_ANNOUNCE"};
			reaction teleport_to_warehouse{"TELEPORT_TO_WAREHOUSE", "REACTION_TELEPORT_TO_WAREHOUSE_NOTIFY", "REACTION_TELEPORT_TO_WAREHOUSE_ANNOUNCE"};
			reaction start_activity{"START_ACTIVITY", "REACTION_START_ACTIVITY_NOTIFY", "REACTION_START_ACTIVITY_ANNOUNCE"};
			reaction start_script{"START_SCRIPT", "REACTION_START_SCRIPT_NOTIFY", "REACTION_START_SCRIPT_ANNOUNCE"};
			reaction destroy_personal_vehicle{"DESTROY_PERSONAL_VEHICLE", "REACTION_DESTROY_PERSONAL_VEHICLE_NOTIFY", "REACTION_DESTROY_PERSONAL_VEHICLE_ANNOUNCE"};
			reaction trigger_business_raid{"TRIGGER_BUSINESS_RAID", "REACTION_TRIGGER_BUSINESS_RAID_NOTIFY", "REACTION_TRIGGER_BUSINESS_RAID_ANNOUNCE"};
			reaction turn_into_beast{"TURN_INTO_BEAST", "REACTION_TURN_INTO_BEAST_NOTIFY", "REACTION_TURN_INTO_BEAST_RAID_ANNOUNCE"};
			reaction remote_wanted_level{"WANTED_LEVEL", "REACTION_REMOTE_WANTED_LEVEL_NOTIFY", "REACTION_REMOTE_WANTED_LEVEL_ANNOUNCE"};
			interloper_reaction remote_wanted_level_others{"WANTED_LEVEL_ON_OTHERS", "REACTION_REMOTE_WANTED_LEVEL_OTHERS_NOTIFY", "REACTION_REMOTE_WANTED_LEVEL_OTHERS_ANNOUNCE", false, false};

			reaction clear_ped_tasks{"CLEAR_PED_TASKS", "REACTION_CLEAR_PED_TASKS_NOTIFY", "REACTION_CLEAR_PED_TASKS_ANNOUNCE"};
			reaction remote_ragdoll{"REMOTE_RAGDOLL", "REACTION_REMOTE_RAGDOLL_NOTIFY", "REACTION_REMOTE_RAGDOLL_ANNOUNCE"};
			reaction kick_vote{"VOTE_KICK", "REACTION_VOTE_KICK_NOTIFY", "REACTION_VOTE_KICK_ANNOUNCE"};
			reaction report_cash_spawn{"CASH_SPAWN", "REACTION_CASH_SPAWN_GENERIC", "REACTION_CASH_SPAWN_GENERIC"};
			reaction modder_detection{"MODDER_DETECTION", "REACTION_MODDER_DETECTED_GENERIC", "REACTION_MODDER_DETECTED_GENERIC"};
			reaction game_anti_cheat_modder_detection{"GAME_ANTICHEAT_DETECTION", "REACTION_MODDER_DETECTED_BY_ANTICHEAT_GENERIC", "REACTION_MODDER_DETECTED_BY_ANTICHEAT_GENERIC"};
			reaction request_control_event{"REQUEST_CONTROL_EVENT", "REACTION_REQUEST_CONTROL_NOTIFY", "REACTION_REQUEST_CONTROL_ANNOUNCE"};
			reaction report{"REPORT", "REACTION_REPORT_NOTIFY", "REACTION_REPORT_ANNOUNCE"};
			reaction chat_spam{"CHAT_SPAM", "REACTION_CHAT_SPAM_NOTIFY", "REACTION_CHAT_SPAM_ANNOUNCE"};
			reaction spectate{"SPECTATE", "REACTION_SPECTATING_NOTIFY", "REACTION_SPECTATING_ANNOUNCE"};
			interloper_reaction spectate_others{"SPECTATE_OTHERS", "REACTION_SPECTATING_OTHERS_NOTIFY", "REACTION_SPECTATING_OTHERS_ANNOUNCE", false, false};

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(reactions, bounty, ceo_money, ceo_kick, clear_wanted_level, crash, delete_vehicle, fake_deposit, force_mission, force_teleport, gta_banner, kick, kick_from_interior, mc_teleport, personal_vehicle_destroyed, remote_off_radar, rotate_cam, send_to_cutscene, send_to_location, sound_spam, spectate_notification, give_collectible, transaction_error, tse_freeze, tse_sender_mismatch, vehicle_kick, teleport_to_warehouse, trigger_business_raid, start_activity, start_script, destroy_personal_vehicle, clear_ped_tasks, turn_into_beast, remote_wanted_level, remote_wanted_level_others, remote_ragdoll, kick_vote, report_cash_spawn, modder_detection, game_anti_cheat_modder_detection, request_control_event, report, send_to_interior, chat_spam, spectate, spectate_others)
		} reactions{};

		struct player
		{
			bool spectating            = false;
			bool override_cam_distance = false;
			int cam_distance           = 10;

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(player, spectating, override_cam_distance, cam_distance)
		} player{};

		struct player_db
		{
			bool update_player_online_states   = true;
			bool notify_when_online            = false;
			bool notify_when_joinable          = true;
			bool notify_when_unjoinable        = false;
			bool notify_when_offline           = false;
			bool notify_on_session_type_change = false;
			bool notify_on_session_change      = false;
			bool notify_on_spectator_change    = false;
			bool notify_on_become_host         = false;
			bool notify_on_transition_change   = false;
			bool notify_on_mission_change      = false;

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(player_db, update_player_online_states, notify_when_online, notify_when_joinable, notify_when_unjoinable, notify_when_offline, notify_on_session_type_change, notify_on_session_change, notify_on_spectator_change, notify_on_become_host, notify_on_transition_change, notify_on_mission_change)
		} player_db{};

		struct protections
		{
			struct script_events
			{
				bool bounty                     = true;
				bool ceo_money                  = true;
				bool clear_wanted_level         = true;
				bool force_mission              = true;
				bool force_teleport             = true;
				bool gta_banner                 = false;
				bool mc_teleport                = true;
				bool personal_vehicle_destroyed = true;
				bool remote_off_radar           = true;
				bool rotate_cam                 = true;
				bool send_to_cutscene           = true;
				bool send_to_location           = true;
				bool sound_spam                 = true;
				bool spectate                   = true;
				bool give_collectible           = true;
				bool vehicle_kick               = true;
				bool teleport_to_warehouse      = true;
				bool start_activity             = true;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(script_events, bounty, ceo_money, clear_wanted_level, force_mission, force_teleport, gta_banner, mc_teleport, personal_vehicle_destroyed, remote_off_radar, rotate_cam, send_to_cutscene, send_to_location, sound_spam, spectate, give_collectible, vehicle_kick, teleport_to_warehouse, start_activity)
			} script_events{};

			bool rid_join                = false;
			bool receive_pickup          = false;
			bool admin_check             = true;
			bool kick_rejoin             = true;
			bool force_relay_connections = false;
			bool stop_traffic            = true;
			bool desync_kick             = false;

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(protections, script_events, rid_join, receive_pickup, admin_check, kick_rejoin, force_relay_connections, stop_traffic, desync_kick)
		} protections{};

		struct self
		{
			struct ptfx_effects
			{
				bool show          = false;
				float size         = 0.2f;
				int select         = 0;
				const char* asset  = "scr_agencyheist";
				const char* effect = "scr_fbi_mop_drips";

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(ptfx_effects, show, size)
			} ptfx_effects{};

			struct ipls
			{
				int select = 0;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(ipls, select)
			} ipls{};

			bool clean_player                 = false;
			bool never_wanted                 = false;
			bool force_wanted_level           = false;
			bool passive                      = false;
			bool free_cam                     = false;
			bool invisibility                 = false;
			bool local_visibility             = true;
			bool no_ragdoll                   = false;
			bool noclip                       = false;
			float noclip_aim_speed_multiplier = 0.25f;
			float noclip_speed_multiplier     = 20.f;
			bool off_radar                    = false;
			bool ghost_org                    = false;
			bool super_run                    = false;
			bool no_collision                 = false;
			bool unlimited_oxygen             = false;
			bool no_water_collision           = false;
			int wanted_level                  = 0;
			bool god_mode                     = false;
			bool proof_bullet                 = false;
			bool proof_fire                   = false;
			bool proof_collision              = false;
			bool proof_melee                  = false;
			bool proof_explosion              = false;
			bool proof_steam                  = false;
			bool proof_water                  = false;
			uint32_t proof_mask               = 0;
			bool mobile_radio                 = false;
			bool fast_respawn                 = false;
			bool auto_tp                      = false;
			bool super_jump                   = false;
			bool beast_jump                   = false;
			bool graceful_landing             = false;
			bool healthregen                  = false;
			float healthregenrate             = 1.0f;
			bool superman                     = false;
			bool custom_weapon_stop           = true;
			bool prompt_ambient_animations    = false;
			std::string persist_outfit        = "";
			bool persist_outfits_mis          = false;
			bool interaction_menu_freedom     = false;

			struct hud
			{
				bool color_override                                      = false;
				bool color_override_initialized                          = false;
				std::array<color, hud_colors.size()> hud_color_overrides = {};
				std::array<color, hud_colors.size()> hud_color_defaults  = {}; // don't save

				bool hide_radar                            = false;
				bool hide_ammo                             = false;
				int selected_hud_component                 = 1;
				std::array<bool, 22> hud_components_states = {false};
				bool force_show_hud_element                = false;
				bool force_show_hud                        = false;
				std::atomic_uint_fast8_t typing            = 0;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(hud, color_override, color_override_initialized, hud_color_overrides, hide_radar, hide_ammo, selected_hud_component, hud_components_states, force_show_hud_element, force_show_hud)
			} hud{};

			struct super_hero_fly
			{
				bool enabled         = false;
				bool gradual         = true;
				bool explosions      = true;
				bool auto_land       = false;
				bool charge          = true;
				bool ptfx            = true;
				float fly_speed      = 15.f;
				float initial_launch = 15.f;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(super_hero_fly, gradual, explosions, auto_land, charge, ptfx, fly_speed, initial_launch)
			} super_hero_fly{};

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(self, ipls, ptfx_effects, clean_player, never_wanted, force_wanted_level, passive, free_cam, invisibility, local_visibility, no_ragdoll, noclip, noclip_aim_speed_multiplier, noclip_speed_multiplier, off_radar, super_run, no_collision, unlimited_oxygen, no_water_collision, wanted_level, god_mode, proof_bullet, proof_fire, proof_collision, proof_melee, proof_explosion, proof_steam, proof_water, proof_mask, mobile_radio, fast_respawn, auto_tp, super_jump, beast_jump, healthregen, healthregenrate, hud, superman, custom_weapon_stop, prompt_ambient_animations, persist_outfit, persist_outfits_mis, interaction_menu_freedom, super_hero_fly, graceful_landing)

		} self{};

		struct session
		{
			bool log_chat_messages                 = false;
			bool log_text_messages                 = false;
			bool decloak_players                   = false;
			bool unhide_players_from_player_list   = true;
			int spoof_host_token_type              = 0;
			std::uint64_t custom_host_token        = 0x000000200235F2EA;
			bool hide_token_spoofing_when_host     = true;
			bool force_script_host                 = false;
			bool player_magnet_enabled             = false;
			int player_magnet_count                = 32;
			bool is_team                           = false;
			bool join_in_sctv_slots                = false;
			bool lock_session                      = false;
			bool allow_friends_into_locked_session = false;
			bool trust_friends                     = false;
			bool trust_session                     = false;
			bool use_spam_timer                    = true;
			float spam_timer                       = 2.5f;
			int spam_length                        = 55;

			const char chat_command_prefix = '/';
			const char chat_output_prefix  = '>';

			bool chat_commands                                   = false;
			CommandAccessLevel chat_command_default_access_level = CommandAccessLevel::FRIENDLY;

			bool kick_host_when_forcing_host = false;
			bool exclude_modders_from_kick_host = false;

			bool explosion_karma = false;
			bool damage_karma    = false;

			bool disable_traffic = false;
			bool disable_peds    = false;
			bool force_thunder   = false;

			bool block_ceo_money      = false;
			bool randomize_ceo_colors = false;
			bool block_jobs           = false;
			bool block_muggers        = false;
			bool block_ceo_raids      = false;
			bool block_ceo_creation   = false;

			int send_to_apartment_idx = 1;
			int send_to_warehouse_idx = 1;

			script_block_opts script_block_opts;

			// not to be saved
			std::atomic_bool spoof_host_token_dirty = true;
			std::uint64_t original_host_token = 0;
			bool join_queued = false;
			rage::rlSessionInfo info;
			bool never_wanted_all = false;
			bool off_radar_all    = false;
			bool semi_godmode_all = false;
			bool wanted_level_all = false;
			bool vehicle_fix_all  = false;
			bool harass_players   = false;
			bool spam_killfeed    = false;

			bool anonymous_bounty      = true;

			bool fast_join = false;

			struct chat_translator
			{
				bool enabled                = false;
				bool print_result           = false;
				bool draw_result            = true;
				bool bypass_same_language   = true;
				std::string target_language = "en";
				std::string endpoint        = "http://localhost:5000/translate";

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(chat_translator, enabled, print_result, draw_result, bypass_same_language, target_language, endpoint);
			} chat_translator{};

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(session, log_chat_messages, log_text_messages, decloak_players, spoof_host_token_type, custom_host_token, hide_token_spoofing_when_host, force_script_host, player_magnet_enabled, player_magnet_count, is_team, join_in_sctv_slots, kick_host_when_forcing_host, exclude_modders_from_kick_host, explosion_karma, damage_karma, disable_traffic, disable_peds, force_thunder, block_ceo_money, randomize_ceo_colors, block_jobs, block_muggers, block_ceo_raids, block_ceo_creation, send_to_apartment_idx, send_to_warehouse_idx, chat_commands, chat_command_default_access_level, anonymous_bounty, lock_session, fast_join, unhide_players_from_player_list, allow_friends_into_locked_session, trust_friends, use_spam_timer, spam_timer, spam_length, chat_translator, script_block_opts)
		} session{};

		struct settings
		{
			bool onboarding_complete = false;
			bool dev_dlc             = false;

			struct hotkeys
			{
				bool editing_menu_toggle = false;
				std::atomic<bool> is_mp_chat_active;
				int menu_toggle             = VK_INSERT;
				int teleport_waypoint       = 0;
				int teleport_objective      = 0;
				int teleport_pv             = 0;
				int teleport_selected       = 0;
				int noclip                  = 0;
				int vehicle_flymode         = 0;
				int bringvehicle            = 0;
				int invis                   = 0;
				int heal                    = 0;
				int fill_inventory          = 0;
				int skip_cutscene           = 0;
				int freecam                 = 0;
				int superrun                = 0;
				int passive                 = 0;
				int superjump               = 0;
				int beastjump               = 0;
				int invisveh                = 0;
				int localinvisveh           = 0;
				int fill_ammo               = 0;
				int fast_quit               = 0;
				int cmd_excecutor           = 'U';
				int repairpv                = 0;
				int open_vehicle_controller = 0;
				int clear_wanted            = 0;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(hotkeys, editing_menu_toggle, menu_toggle, teleport_waypoint, teleport_objective, teleport_selected, teleport_pv, noclip, vehicle_flymode, bringvehicle, invis, heal, fill_inventory, skip_cutscene, freecam, superrun, passive, superjump, beastjump, invisveh, localinvisveh, fill_ammo, fast_quit, cmd_excecutor, repairpv, open_vehicle_controller, clear_wanted)
			} hotkeys{};

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(settings, hotkeys, dev_dlc, onboarding_complete)
		} settings{};

		struct spawn_vehicle
		{
			bool preview_vehicle = false;
			bool spawn_inside    = false;
			bool spawn_maxed     = false;
			std::string plate    = "";
			int spawn_type       = 0;

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(spawn_vehicle, preview_vehicle, spawn_inside, spawn_maxed, plate, spawn_type)
		} spawn_vehicle{};

		struct clone_pv
		{
			bool preview_vehicle = false;
			bool spawn_inside    = false;
			bool spawn_clone     = false;
			bool spawn_maxed     = false;
			bool clone_plate     = false;
			std::string plate    = "";
			std::string garage   = "";

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(clone_pv, preview_vehicle, spawn_inside, spawn_clone, spawn_maxed, clone_plate, plate, garage)
		} clone_pv{};

		struct persist_car
		{
			bool preview_vehicle                   = false;
			bool spawn_inside                      = false;
			std::string persist_vehicle_sub_folder = "";

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(persist_car, preview_vehicle, spawn_inside, persist_vehicle_sub_folder)
		} persist_car{};

		struct world
		{
			struct orbital_drone
			{
				bool enabled             = false;
				bool detect_player       = false;
				float nav_ovverride_fast = 3.f;
				float nav_ovverride_slow = 0.25f;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(orbital_drone, detect_player, nav_ovverride_fast, nav_ovverride_slow);
			} orbital_drone{};

			struct train
			{
				bool drive_train  = false;
				bool derail_train = false;
			} train{};

			struct gravity
			{
				bool modify_gravity   = false;
				float current_gravity = 9.8f;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(gravity, modify_gravity, current_gravity)
			} gravity{};

			struct ocean
			{
				bool modify_ocean  = false;
				bool disable_ocean = false;
				int ocean_opacity  = 100;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(ocean, modify_ocean, disable_ocean, ocean_opacity)
			} ocean{};

			struct waypoint_n_objective
			{
				bool waypoint_beacon            = false;
				bool objective_beacon           = false;
				float waypoint_beacon_color[3]  = {1, 0, 1};
				float objective_beacon_color[3] = {1, 1, 0};

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(waypoint_n_objective, waypoint_beacon, objective_beacon, objective_beacon_color, waypoint_beacon_color)
			} waypoint_n_objective{};

			struct spawn_ped
			{
				bool preview_ped       = false;
				bool spawn_invincible  = false;
				bool spawn_invisible   = false;
				bool spawn_as_attacker = false;
				bool randomize_outfit  = false;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(spawn_ped, preview_ped, spawn_invincible, spawn_invisible, spawn_as_attacker, randomize_outfit)
			} spawn_ped{};

			struct custom_time
			{
				bool override_time = {};
				int hour{}, minute{}, second{};

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(custom_time, override_time, hour, minute, second)
			} custom_time;


			struct blackhole
			{
				bool enable           = false;
				bool include_peds     = false;
				bool include_vehicles = false;
				float scale           = 6.f;
				float color[3]        = {1, 1, 1};
				int alpha             = 150;
				rage::fvector3 pos;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(blackhole, include_peds, include_vehicles, color, alpha)
			} blackhole{};

			struct nearby
			{
				bool ignore     = false;
				bool ped_rain   = false;
				bool veh_rain   = false;
				bool high_alert = false;
				bool ped_rush   = false;
				bool combative  = false;

				struct auto_disarm
				{
					bool enable     = false;
					bool neutralize = false;
					NLOHMANN_DEFINE_TYPE_INTRUSIVE(auto_disarm, enable, neutralize)
				} auto_disarm{};

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(nearby, ignore, high_alert, ped_rush, auto_disarm)
			} nearby{};

			struct model_swapper
			{
				std::vector<std::pair<std::string, std::string>> models;
				std::mutex m;
				bool update = false;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(model_swapper, models)
			} model_swapper{};


			bool override_weather = false;
			int local_weather     = 0;

			bool blackout    = false;
			bool ground_snow = false;

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(world, spawn_ped, custom_time, blackhole, model_swapper, nearby, orbital_drone, local_weather, override_weather, blackout, ground_snow)
		} world{};

		struct spoofing
		{
			bool hide_from_player_list = false;

			bool spoof_blip = false;
			int blip_type   = 0; // actual blip type + 1

			bool spoof_rank = false;
			int rank        = 1;

			bool spoof_job_points = false;
			int job_points        = 1;

			bool spoof_kd_ratio = false;
			float kd_ratio      = 2.0f;

			bool spoof_bad_sport = false;
			int badsport_type    = 0;

			bool spoof_player_model   = false;
			std::string player_model  = "";

			bool spoof_cheater = false;

			bool spoof_hide_god      = true;
			bool spoof_hide_veh_god  = true;
			bool spoof_hide_spectate = true;

			bool spoof_crew_data = false;
			std::string crew_tag = "";
			bool rockstar_crew   = false;
			bool square_crew_tag = false;

			bool spoof_session_region_type      = false;
			int session_region_type             = 0;
			bool spoof_session_language         = false;
			eGameLanguage session_language      = eGameLanguage::ENGLISH;
			bool spoof_session_player_count     = false;
			int session_player_count            = 25;
			int spoof_session_bad_sport_status  = 0;
			bool multiplex_session              = false;
			int multiplex_count                 = 2;
			bool increase_player_limit          = false;

			bool voice_chat_audio = false;

			bool spoof_game_data_hash = false;
			std::array<std::uint32_t, 15> game_data_hash{};
			bool spoof_dlc_hash = false;
			std::uint32_t dlc_hash;

			// do not save

			bool game_data_hash_dirty = true;

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(spoofing, hide_from_player_list, spoof_blip, blip_type, spoof_rank, rank, spoof_job_points, job_points, spoof_kd_ratio, kd_ratio, spoof_bad_sport, badsport_type, spoof_player_model, player_model, spoof_cheater, spoof_hide_god, spoof_hide_veh_god, spoof_hide_spectate, spoof_crew_data, crew_tag, rockstar_crew, square_crew_tag, spoof_session_region_type, session_region_type, spoof_session_language, session_language, spoof_session_player_count, session_player_count, spoof_session_bad_sport_status, multiplex_session, multiplex_count, increase_player_limit, voice_chat_audio, spoof_game_data_hash, game_data_hash, spoof_dlc_hash, dlc_hash)
		} spoofing{};

		struct vehicle
		{
			struct speedo_meter
			{
				float x = .9f;
				float y = .72f;

				bool enabled           = false;
				bool left_side         = false;
				bool show_current_gear = true;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(speedo_meter, x, y, enabled, left_side, show_current_gear)
			} speedo_meter{};

			struct fly
			{
				bool dont_stop    = false;
				bool enabled      = false;
				bool no_collision = false;
				bool stop_on_exit = false;
				float speed       = 100;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(fly, dont_stop, enabled, no_collision, stop_on_exit, speed)
			} fly{};

			struct rainbow_paint
			{
				RainbowPaintType type = RainbowPaintType::Off;
				bool neon             = false;
				bool primary          = false;
				bool secondary        = false;
				bool smoke            = false;
				int speed             = 1;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(rainbow_paint, type, neon, primary, secondary, smoke, speed)
			} rainbow_paint{};

			SpeedUnit speed_unit = SpeedUnit::MIPH;

			bool god_mode        = false;
			bool proof_bullet    = false;
			bool proof_fire      = false;
			bool proof_collision = false;
			bool proof_melee     = false;
			bool proof_explosion = false;
			bool proof_steam     = false;
			bool proof_water     = false;
			uint32_t proof_mask  = 0;

			AutoDriveDestination auto_drive_destination = AutoDriveDestination::STOPPED;
			AutoDriveStyle auto_drive_style             = AutoDriveStyle::LAW_ABIDING;
			float auto_drive_speed                      = 1;
			bool auto_turn_signals                      = false;
			eBoostBehaviors boost_behavior              = eBoostBehaviors::DEFAULT;
			bool drive_on_water                         = false;
			bool horn_boost                             = false;
			bool instant_brake                          = false;
			bool infinite_veh_ammo					    = false;
			bool block_homing                           = true;
			bool seatbelt                               = false;
			bool turn_signals                           = false;
			bool vehicle_jump                           = false;
			bool keep_vehicle_repaired                  = false;
			bool no_water_collision                     = false;
			bool disable_engine_auto_start              = false;
			bool change_engine_state_immediately        = false;
			bool keep_engine_running                    = false;
			bool keep_vehicle_clean                     = false;
			bool vehinvisibility                        = false;
			bool localveh_visibility                    = false;
			bool keep_on_ground                         = false;
			bool no_collision                           = false;
			bool unlimited_weapons                      = false;
			bool siren_mute                             = false;
			bool all_vehs_in_heists                     = false;

			struct abilities
			{
				bool enabled   = false;
				bool jump      = false;
				bool rocket    = false;
				bool parachute = false;
				bool ramp      = false;
				bool glider    = false;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(abilities, enabled)
			} abilities{};

			struct vehicle_ammo_special
			{
				bool enabled                       = false;
				eAmmoSpecialType type              = eAmmoSpecialType::None;
				eExplosionTag explosion_tag        = eExplosionTag::EXP_TAG_ROGUE_CANNON;
				float speed                        = 2000;
				float time_between_shots           = 0.04;
				float alternate_wait_time          = -1;
				float weapon_range                 = 250;
				float rocket_time_between_shots    = 0.66;
				float rocket_alternate_wait_time   = 0.66;
				float rocket_lock_on_range         = 500;
				float rocket_range                 = 1000;
				float rocket_reload_time           = -1;
				eExplosionTag rocket_explosion_tag = eExplosionTag::TANKSHELL;
				float rocket_lifetime              = 15;
				float rocket_launch_speed          = 1200;
				float rocket_time_before_homing    = 0.75;
				bool rocket_improve_tracking       = true;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(vehicle_ammo_special, enabled, type, explosion_tag, speed, time_between_shots, alternate_wait_time, weapon_range, rocket_time_between_shots, rocket_alternate_wait_time, rocket_lock_on_range, rocket_range, rocket_reload_time, rocket_explosion_tag, rocket_lifetime, rocket_launch_speed, rocket_time_before_homing, rocket_improve_tracking)
			} vehicle_ammo_special{};

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(vehicle, speedo_meter, fly, rainbow_paint, speed_unit, god_mode, proof_bullet, proof_fire, proof_collision, proof_melee, proof_explosion, proof_steam, proof_water, proof_mask, auto_drive_destination, auto_drive_style, auto_drive_speed, auto_turn_signals, boost_behavior, drive_on_water, horn_boost, instant_brake, infinite_veh_ammo, block_homing, seatbelt, turn_signals, vehicle_jump, keep_vehicle_repaired, no_water_collision, disable_engine_auto_start, change_engine_state_immediately, keep_engine_running, keep_vehicle_clean, vehinvisibility, localveh_visibility, keep_on_ground, no_collision, unlimited_weapons, siren_mute, all_vehs_in_heists, abilities, vehicle_ammo_special)
		} vehicle{};

		struct weapons
		{
			struct ammo_special
			{
				bool toggle                 = false;
				eAmmoSpecialType type       = eAmmoSpecialType::None;
				eExplosionTag explosion_tag = eExplosionTag::DONTCARE;

				NLOHMANN_JSON_SERIALIZE_ENUM(eAmmoSpecialType,
				    {
				        {eAmmoSpecialType::None, "none"},
				        {eAmmoSpecialType::ArmorPiercing, "armor_piercing"},
				        {eAmmoSpecialType::Explosive, "explosive"},
				        {eAmmoSpecialType::FMJ, "fmj"},
				        {eAmmoSpecialType::HollowPoint, "hollow_point"},
				        {eAmmoSpecialType::Incendiary, "incendiary"},
				        {eAmmoSpecialType::Tracer, "tracer"},
				    })
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(ammo_special, toggle, type, explosion_tag)
			} ammo_special{};

			struct gravity_gun
			{
				bool launch_on_release = false;
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(gravity_gun, launch_on_release)
			} gravity_gun{};

			struct paintgun
			{
				bool rainbow = false;
				float speed  = 1.f;
				float col[4] = {0.f, 0.f, 1.f, 1.f};
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(paintgun, rainbow, col)
			} paintgun{};

			struct aimbot
			{
				bool enable              = false;
				int64_t only_on_ped_type = -1;
				bool only_on_player      = false;
				bool exclude_friends     = false;
				bool only_on_enemy       = false;
				bool has_target          = false;
				bool use_weapon_range    = false;
				float fov                = 60.f;
				float distance           = 200.f;
				int32_t selected_bone    = (int32_t)ePedBoneType::HEAD;
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(aimbot, enable, only_on_ped_type, only_on_player, only_on_enemy, fov, distance, selected_bone, use_weapon_range)
			} aimbot{};

			struct flying_axe
			{
				bool enable = false;
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(flying_axe, enable)
			} flying_axe{};

			CustomWeapon custom_weapon    = CustomWeapon::NONE;
			bool infinite_ammo            = false;
			bool always_full_ammo         = false;
			bool infinite_mag             = false;
			float increased_damage        = 1;
			bool increase_damage          = false;
			float set_explosion_radius    = 1.f;
			bool modify_explosion_radius  = false;
			bool no_recoil                = false;
			bool no_spread                = false;
			bool no_sway                  = false;
			std::string vehicle_gun_model = "bus";
			bool increased_c4_limit       = false;
			bool increased_flare_limit    = false;
			bool rapid_fire               = false;
			bool interior_weapon          = false;
			bool triggerbot               = false;
			bool infinite_range           = false;
			bool enable_weapon_hotkeys    = false;
			bool enable_mk1_variants      = false;
			std::map<int, std::vector<uint32_t>> weapon_hotkeys{};

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(weapons, ammo_special, custom_weapon, aimbot, infinite_ammo, always_full_ammo, infinite_mag, increased_damage, increase_damage, set_explosion_radius, modify_explosion_radius, no_recoil, no_spread, vehicle_gun_model, increased_c4_limit, increased_flare_limit, rapid_fire, gravity_gun, paintgun, interior_weapon, triggerbot, infinite_range, enable_weapon_hotkeys, weapon_hotkeys, enable_mk1_variants, no_sway)
		} weapons{};

		struct window
		{
			ImU32 background_color = 3696311571;
			ImU32 text_color       = 4294967295;
			ImU32 button_color     = 2947901213;
			ImU32 frame_color      = 2942518340;
			float gui_scale        = 1.f;

			ImFont* font_title     = nullptr;
			ImFont* font_sub_title = nullptr;
			ImFont* font_small     = nullptr;
			ImFont* font_icon      = nullptr;

			bool demo          = false;
			bool switched_view = true;

			struct ingame_overlay
			{
				bool opened                = true;
				bool show_with_menu_opened = false;

				bool show_fps              = true;
				bool show_indicators       = true;
				bool show_players          = true;
				bool show_time             = true;
				bool show_replay_interface = true;
				bool show_position         = false;
				bool show_game_versions    = true;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(ingame_overlay, opened, show_with_menu_opened, show_fps, show_indicators, show_players, show_time, show_replay_interface, show_position, show_game_versions)
			} ingame_overlay{};

			struct ingame_overlay_indicators
			{
				bool show_player_godmode   = true;
				bool show_off_radar        = true;
				bool show_vehicle_godmode  = true;
				bool show_never_wanted     = true;
				bool show_infinite_ammo    = false;
				bool show_always_full_ammo = false;
				bool show_infinite_mag     = false;
				bool show_aimbot           = false;
				bool show_triggerbot       = false;
				bool show_invisibility     = false;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(ingame_overlay_indicators, show_player_godmode, show_off_radar, show_vehicle_godmode, show_never_wanted, show_always_full_ammo, show_infinite_ammo, show_infinite_mag, show_aimbot, show_triggerbot, show_invisibility)
			} ingame_overlay_indicators{};

			struct vehicle_control
			{
				bool opened                 = false;
				bool operation_animation    = true;
				bool render_distance_on_veh = false;
				bool show_info              = false;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(vehicle_control, operation_animation, render_distance_on_veh, show_info)
			} vehicle_control{};

			struct gui
			{
				bool format_money = true;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(gui, format_money)
			} gui{};

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(window, background_color, demo, text_color, button_color, frame_color, gui_scale, switched_view, ingame_overlay, vehicle_control, ingame_overlay_indicators, gui)
		} window{};

		struct context_menu
		{
			bool enabled = false;

			uint8_t allowed_entity_types = static_cast<uint8_t>(ContextEntityType::PED) | static_cast<uint8_t>(ContextEntityType::PLAYER) | static_cast<uint8_t>(ContextEntityType::VEHICLE) | static_cast<uint8_t>(ContextEntityType::OBJECT);

			ImU32 selected_option_color = 4278255360;

			bool bounding_box_enabled = true;
			ImU32 bounding_box_color  = 4278255360;

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(context_menu, enabled, allowed_entity_types, selected_option_color, bounding_box_enabled, bounding_box_color)
		} context_menu{};

		struct esp
		{
			bool enabled                    = true;
			float global_render_distance[2] = {0.f, 600.f};
			float tracer_render_distance[2] = {200.f, 600.f};
			float box_render_distance[2]    = {0.f, 150.f};
			bool tracer                     = true;
			float tracer_draw_position[2]   = {0.5f, 1.f};
			bool box                        = true;
			bool health                     = true;
			bool armor                      = true;
			bool god                        = true;
			bool distance                   = true;
			bool name                       = true;
			bool change_esp_color_from_dist = false;
			bool scale_health_from_dist     = false;
			bool scale_armor_from_dist      = false;
			float distance_threshold[2]     = {100.f, 200.f};
			ImU32 enemy_color               = 4281479904;
			ImU32 enemy_near_color          = 4283794943;
			ImU32 default_color             = 4285713522;
			ImU32 friend_color              = 4293244509;

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(esp, enabled, global_render_distance, tracer_render_distance, box_render_distance, tracer, tracer_draw_position, box, health, armor, god, distance, name, change_esp_color_from_dist, scale_health_from_dist, scale_armor_from_dist, distance_threshold, enemy_color, enemy_near_color, default_color, friend_color)
		} esp{};

		struct session_browser
		{
			bool region_filter_enabled = true;
			int region_filter          = 0;

			bool language_filter_enabled = false;
			eGameLanguage language_filter = eGameLanguage::ENGLISH;

			bool pool_filter_enabled = false;
			int pool_filter          = 0;

			bool player_count_filter_enabled = false;
			int player_count_filter_minimum  = 0;
			int player_count_filter_maximum  = 32;

			bool filter_multiplexed_sessions = false;

			int sort_method    = 0;
			int sort_direction = 0;

			bool replace_game_matchmaking = false;
			bool exclude_modder_sessions     = false;

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(session_browser, region_filter_enabled, region_filter, language_filter_enabled, language_filter, player_count_filter_enabled, player_count_filter_minimum, player_count_filter_maximum, filter_multiplexed_sessions, sort_method, sort_direction, replace_game_matchmaking, pool_filter_enabled, pool_filter, exclude_modder_sessions)
		} session_browser{};

		struct session_protection
		{
			int block_join_reason = 0;

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(session_protection, block_join_reason)
		} session_protection{};

		struct ugc
		{
			bool infinite_model_memory;

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(ugc, infinite_model_memory)
		} ugc{};

		struct stat_editor
		{
			struct stat
			{
				int radio_button_index      = 0;
				std::string int_text        = "";
				std::string int_value       = "";
				bool int_read               = false;
				std::string bool_text       = "";
				std::string bool_value      = "";
				bool bool_read              = false;
				std::string float_text      = "";
				std::string float_value     = "";
				bool float_read             = false;
				std::string increment_text  = "";
				std::string increment_value = "";
				bool increment_loop_write   = false;
				std::string date_text       = "";
				std::string date_value      = "";
				bool date_read              = false;
				std::string string_text     = "";
				std::string string_value    = "";
				bool string_read            = false;
				std::string label_text      = "";
				std::string label_value     = "";
				std::string user_id_text    = "";
				std::string user_id_value   = "";
				bool user_id_read           = false;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(stat, radio_button_index, int_text, int_value, int_read, bool_text, bool_value, bool_read, float_text, float_value, float_read, increment_text, increment_value, increment_loop_write, date_text, date_value, date_read, string_text, string_value, string_read, label_text, label_value, user_id_text, user_id_value, user_id_read)
			} stat{};

			struct packed_stat
			{
				int radio_button_index = 0;

				std::string int_text  = "";
				std::string int_value = "";
				bool int_read         = false;

				std::string bool_text  = "";
				std::string bool_value = "";
				bool bool_read         = false;

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(packed_stat, radio_button_index, int_text, int_value, int_read, bool_text, bool_value, bool_read)
			} packed_stat{};

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(stat_editor, stat, packed_stat)
		} stat_editor{};

		struct lua
		{
			bool enable_auto_reload_changed_scripts = false;

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(lua, enable_auto_reload_changed_scripts)
		} lua{};

		struct persist_weapons
		{
			bool enabled = false;
			std::string weapon_loadout_file;

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(persist_weapons, enabled, weapon_loadout_file)
		} persist_weapons{};

		struct vfx
		{
			bool enable_custom_sky_color = false;

			float azimuth_east[4]       = {1, 0, 0, 0};
			float azimuth_west[4]       = {1, 0, 0, 0};
			float azimuth_transition[4] = {1, 0, 0, 0};
			float zenith[4]             = {1, 0, 0, 0};

			float stars_intensity = 1;

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(vfx, enable_custom_sky_color, azimuth_east, azimuth_west, azimuth_transition, zenith, stars_intensity)
		} vfx{};

		struct cmd
		{
			std::deque<std::string> command_history;
			NLOHMANN_DEFINE_TYPE_INTRUSIVE(cmd, command_history)
		} cmd{};

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(menu_settings, debug, tunables, notifications, player, player_db, protections, self, session, settings, spawn_vehicle, clone_pv, persist_car, spoofing, vehicle, weapons, window, context_menu, esp, session_browser, ugc, reactions, world, stat_editor, lua, persist_weapons, vfx, cmd)
	};

	inline auto g = menu_settings();
}
