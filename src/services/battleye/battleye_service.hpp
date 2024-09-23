#pragma once

namespace big
{
	class battleye_service
	{
		using log_func_t      = void (*)(const char* msg, int level);
		using kick_player_t   = void (*)(std::uint64_t id, const char* reason);
		using send_message_t  = void (*)(std::uint64_t id, const void* msg, int size);
		using add_player_t    = void (*)(std::uint64_t id, u_long ip_addr, u_short port, const char* name, char unused);
		using set_player_state_t = void (*)(std::uint64_t id, int reason);
		using assign_guid_t = void (*)(std::uint64_t id, const void* guid, int size); // binary
		using receive_message_t = void (*)(std::uint64_t id, const void* packet, int size);

		using shutdown_t    = void (*)();
		using run_t         = bool (*)();
		using run_command_t = void (*)(const char* command);

		struct CUserData
		{
			const char* m_game_name;
			log_func_t m_log_func;
			kick_player_t m_kick_player;
			send_message_t m_send_message;
			void* m_unk;
		} m_battleye_user_data{};

		struct CApi
		{
			shutdown_t m_shutdown;
			run_t m_run;
			run_command_t m_run_command;
			add_player_t m_add_player;
			set_player_state_t m_set_player_state;
			assign_guid_t m_assign_guid;
			assign_guid_t m_assign_guid_verified;
			receive_message_t m_receive_message;
		} m_battleye_api{};

		using init_t = bool (*)(int api_level, CUserData* data, CApi* api);

		std::mutex m_mutex{};

		bool is_running();
	public:
		battleye_service();
		~battleye_service();
		
		void start();
		void stop();
		void add_player(std::uint64_t token, std::uint64_t rockstar_id, const char* name);
		void remove_player(std::uint64_t token);
		void receive_message(std::uint64_t token, void* message, int size);
		void send_message(std::uint64_t token, void* message, int size);
		void kick_player(std::uint64_t token, const char* reason);
		void script_func();
		void thread_func();
	};

	inline battleye_service g_battleye_service;
}