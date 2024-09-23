#pragma once
#include "gta/net_game_event.hpp"
#include "network/CNetGamePlayer.hpp"
#include "services/players/player_service.hpp"

namespace big
{
	class packet
	{
	public:
		char m_data[0x480]{};
		rage::datBitBuffer m_buffer;

		packet();
		void send(uint32_t msg_id, bool unk_flag = false);
		void send(player_ptr player, int connection_id);
		void send_direct(int peer_id, int connection_id);

		inline operator rage::datBitBuffer&()
		{
			return m_buffer;
		}

		template<typename T>
		inline void write(T data, int length)
		{
			m_buffer.Write<T>(data, length);
		}

		inline void write_message(rage::eNetMessage message)
		{
			write<int>(0x3246, 14);
			if ((int)message > 0xFF)
			{
				write<bool>(true, 1);
				write<rage::eNetMessage>(message, 16);
			}
			else
			{
				write<bool>(false, 1);
				write<rage::eNetMessage>(message, 8);
			}
		}

		inline void write_peer_id(uint64_t peer_id)
		{
			char b[8];
			rage::datBitBuffer buf(b, 8);
			buf.WriteQWord(peer_id, 64);
			m_buffer.WriteArray(b, 8 * buf.GetDataLength());
		}
	};
}