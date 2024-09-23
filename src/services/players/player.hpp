#pragma once
#include "player_service.hpp"
#include "rate_limiter.hpp"

class CVehicle;
class CPed;
class CNetGamePlayer;
class CPlayerInfo;

namespace rage
{
	class snPlayer;
	class snPeer;
	class rlGamerInfo;
	class netConnectionPeer;
}

namespace big
{
	class player final
	{
		friend class player_service;

		CNetGamePlayer* m_net_game_player = nullptr;
		std::string m_identifier;
		bool m_is_friend;

	public:
		explicit player(CNetGamePlayer* net_game_player);
		~player() = default;

		player(const player&)                = default;
		player(player&&) noexcept            = default;
		player& operator=(const player&)     = default;
		player& operator=(player&&) noexcept = default;

		[[nodiscard]] CVehicle* get_current_vehicle() const;
		[[nodiscard]] const char* get_name() const;
		[[nodiscard]] rage::rlGamerInfo* get_net_data() const;
		[[nodiscard]] int64_t get_rockstar_id() const;
		[[nodiscard]] CNetGamePlayer* get_net_game_player() const;
		[[nodiscard]] CPed* get_ped() const;
		[[nodiscard]] CPlayerInfo* get_player_info() const;
		[[nodiscard]] class rage::snPlayer* get_session_player();
		[[nodiscard]] class rage::snPeer* get_session_peer();
		[[nodiscard]] class rage::netConnectionPeer* get_connection_peer();
		[[nodiscard]] std::optional<netAddress> get_ip_address();
		[[nodiscard]] uint16_t get_port();

		[[nodiscard]] uint8_t id() const;

		[[nodiscard]] bool is_friend() const;
		[[nodiscard]] bool is_host() const;
		[[nodiscard]] bool is_valid() const;

		std::optional<CommandAccessLevel> command_access_level = std::nullopt;

		bool off_radar    = false;
		bool never_wanted = false;
		bool semi_godmode = false;
		bool fix_vehicle  = false;

		bool kill_loop       = false;
		bool explosion_loop  = false;
		bool ragdoll_loop    = false;
		bool rotate_cam_loop = false;

		rate_limiter m_host_migration_rate_limit{2s, 15};
		rate_limiter m_play_sound_rate_limit{1s, 10};
		rate_limiter m_play_sound_rate_limit_tse{5s, 2};
		rate_limiter m_invites_rate_limit{10s, 2};
		rate_limiter m_radio_request_rate_limit{5s, 2};
		rate_limiter m_radio_station_change_rate_limit{1s, 3};

		bool block_radio_requests = false;
		bool received_object_id_request = false;
		bool received_object_id_response = false;

		bool is_modder        = false;
		bool is_trusted       = false;
		bool block_join       = false;
		int block_join_reason = 0;
		bool is_spammer       = false;
		bool is_admin         = false;
		std::optional<uint32_t> player_time_value;
		std::optional<std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>> player_time_value_received_time;
		std::optional<uint32_t> time_difference;
		std::optional<std::chrono::time_point<std::chrono::steady_clock>> last_message_time;
		uint32_t num_time_syncs_sent = 9999;
		std::optional<int> script_host_mission;

		bool block_explosions   = false;
		bool block_clone_create = false;
		bool block_clone_sync   = false;
		bool block_net_events   = false;
		bool log_clones         = false;
		bool log_network_events = false;

		bool trigger_desync_kick = false;
		bool trigger_end_session_kick = false;

		bool spam_killfeed = false;

		int spectating_player = -1;

		bool tampered_with_be = false;

		menu_settings::script_block_opts script_block_opts{};
	protected:
		bool equals(const CNetGamePlayer* net_game_player) const;

		[[nodiscard]] std::string to_lowercase_identifier() const;
	};
}