#include "battleye_service.hpp"
#include "services/players/player_service.hpp"
#include "packet.hpp"
#include "script_mgr.hpp"
#include "thread_pool.hpp"
#include "util/session.hpp"
#include "backend/looped_command.hpp"

#include <network/snSession.hpp>

namespace
{
	static std::string base64_encode(const std::string& data)
	{
		static constexpr char sEncodingTable[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

		size_t in_len  = data.size();
		size_t out_len = 4 * ((in_len + 2) / 3);
		std::string ret(out_len, '\0');
		size_t i;
		char* p = ret.data();

		for (i = 0; i < in_len - 2; i += 3)
		{
			*p++ = sEncodingTable[(data[i] >> 2) & 0x3F];
			*p++ = sEncodingTable[((data[i] & 0x3) << 4) | ((int)(data[i + 1] & 0xF0) >> 4)];
			*p++ = sEncodingTable[((data[i + 1] & 0xF) << 2) | ((int)(data[i + 2] & 0xC0) >> 6)];
			*p++ = sEncodingTable[data[i + 2] & 0x3F];
		}
		if (i < in_len)
		{
			*p++ = sEncodingTable[(data[i] >> 2) & 0x3F];
			if (i == (in_len - 1))
			{
				*p++ = sEncodingTable[((data[i] & 0x3) << 4)];
				*p++ = '=';
			}
			else
			{
				*p++ = sEncodingTable[((data[i] & 0x3) << 4) | ((int)(data[i + 1] & 0xF0) >> 4)];
				*p++ = sEncodingTable[((data[i + 1] & 0xF) << 2)];
			}
			*p++ = '=';
		}

		return ret;
	}

}

namespace big
{
	class battleye_server : looped_command
	{
		using looped_command::looped_command;

		virtual void on_enable() override
		{
			g_battleye_service.start();
		}

		virtual void on_tick() override
		{
		}

		virtual void on_disable() override
		{
			g_battleye_service.stop();
		}
	};

	battleye_server g_battleye_server("battleyeserver", "BATTLEYE_SERVER", "BATTLEYE_SERVER_DESC", g.debug.battleye_server);

	bool battleye_service::is_running()
	{
		return m_battleye_api.m_shutdown != nullptr;
	}

	void battleye_service::send_message(std::uint64_t token, void* message, int size)
	{
		packet pkt;

		char header_buf[32];
		rage::datBitBuffer header(header_buf, sizeof(header_buf));

		header.Write<int>(size, 11);
		header.Write<bool>(false, 1); // we are not the client

		pkt.write_message(rage::eNetMessage::MsgBattlEyeCmd);
		pkt.m_buffer.WriteArray(&header_buf, header.GetDataLength() * 8);
		pkt.m_buffer.WriteArray(message, size * 8);

		// send to player
		if (auto plyr = g_player_service->get_by_host_token(token); plyr && plyr->get_session_player())
		{
			pkt.send(plyr->get_session_player()->m_msg_id, true);
		}
	}

	void battleye_service::kick_player(std::uint64_t token, const char* reason)
	{
		if (auto plyr = g_player_service->get_by_host_token(token))
		{
			LOGF(WARNING, "BattlEye wants us to kick {} for {}", plyr->get_name(), reason);
			if (!plyr->tampered_with_be)
			{
				session::add_infraction(plyr, Infraction::TRIGGERED_ANTICHEAT);
			}
		}
	}

	void battleye_service::script_func()
	{
		bool was_host = false;
		while (is_running())
		{
			if (g_player_service->get_self()->is_valid())
			{
				bool is_host = g_player_service->get_self()->is_host();

				if (is_host != was_host)
				{
					if (is_host)
					{
						for (auto& [_, player] : g_player_service->players())
						{
							add_player(player->get_net_game_player()->get_host_token(), player->get_rockstar_id(), player->get_name());
						}
					}
					was_host = is_host;
				}
			}

			big::script::get_current()->yield();
		}
	}

	void battleye_service::thread_func()
	{
		while (is_running() && g_running)
		{
			// we need to run this every frame (lockstep with game or otherwise)
			{
				std::lock_guard lock(m_mutex);
				if (is_running() && !m_battleye_api.m_run())
				{
					LOG(WARNING) << "BE::Run failed";
				}
			}

			std::this_thread::yield();
		}
	}

	battleye_service::battleye_service()
	{
	}

	battleye_service::~battleye_service()
	{
		stop();
	}

	void battleye_service::start()
	{
		std::lock_guard lock(m_mutex);

		if (is_running())
		{
			return;
		}

		auto handle = LoadLibraryA("BattlEye\\BEServer_x64.dll");

		if (!handle)
		{
			LOG(WARNING) << "Failed to load BattlEye";
			return;
		}

		m_battleye_user_data.m_game_name = "paradise";
		m_battleye_user_data.m_log_func  = [](const char* message, int level) {
			LOG(INFO) << "BattlEye: " << message;
		};
		m_battleye_user_data.m_kick_player = [](std::uint64_t player, const char* reason) {
			g_battleye_service.kick_player(player, reason);
		};
		m_battleye_user_data.m_send_message = [](std::uint64_t player, const void* pkt_data, int size) {
			g_battleye_service.send_message(player, const_cast<void*>(pkt_data), size);
		};
		if (reinterpret_cast<init_t>(GetProcAddress(handle, "Init"))(1, &m_battleye_user_data, &m_battleye_api))
		{
			// background thread (not game thread)
			g_thread_pool->push([this] { this->thread_func(); });

			// background script (game thread)
			g_script_mgr.add_script(std::make_unique<big::script>([this] { this->script_func(); }, "BE Background Script", false));
		}
		else
		{
			LOG(WARNING) << "Failed to initialize BattlEye";
		}
	}

	void battleye_service::stop()
	{
		std::lock_guard lock(m_mutex);

		if (!is_running())
		{
			return;
		}

		m_battleye_api.m_shutdown();
		memset(&m_battleye_api, 0, sizeof(CApi));
	}

	void battleye_service::add_player(std::uint64_t token, std::uint64_t rockstar_id, const char* name)
	{
		std::lock_guard lock(m_mutex);

		if (!is_running() || !g_player_service->get_self()->is_host())
		{
			return;
		}

		char string[32]{};

		snprintf(string, sizeof(string), "%I64d", rockstar_id);
		auto guid = base64_encode(string);
		
		m_battleye_api.m_add_player(token, -1, 0, name, false);
		m_battleye_api.m_assign_guid(token, guid.data(), guid.length());
		m_battleye_api.m_assign_guid_verified(token, guid.data(), guid.length());
		m_battleye_api.m_set_player_state(token, 1);
	}

	void battleye_service::remove_player(std::uint64_t token)
	{
		std::lock_guard lock(m_mutex);

		if (!is_running())
		{
			return;
		}

		m_battleye_api.m_set_player_state(token, -1);
	}

	void battleye_service::receive_message(std::uint64_t token, void* message, int size)
	{
		std::lock_guard lock(m_mutex);

		if (!is_running())
		{
			return;
		}

		m_battleye_api.m_receive_message(token, const_cast<const void*>(message), size);
	}
}