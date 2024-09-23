#include "packet.hpp"

#include "gta_util.hpp"

#include <network/Network.hpp>
#include <network/netConnection.hpp>

namespace big
{
	packet::packet() :
	    m_buffer(m_data, sizeof(m_data))
	{
	}

	void packet::send(uint32_t msg_id, bool unk_flag)
	{
		g_pointers->m_gta.m_queue_packet(gta_util::get_network()->m_game_session_ptr->m_net_connection_mgr, msg_id, m_data, (m_buffer.m_curBit + 7) >> 3, unk_flag ? 0x4000001 : 1, nullptr);
	}

	void packet::send(player_ptr player, int connection_id)
	{
		send_direct(player->get_session_player()->m_player_data.m_peer_id_2, connection_id);
	}

	void packet::send_direct(int peer_id, int connection_id)
	{
		auto mgr  = gta_util::get_network()->m_game_session_ptr->m_net_connection_mgr;
		auto peer = g_pointers->m_gta.m_get_connection_peer(mgr, peer_id);
		g_pointers->m_gta.m_send_packet(mgr, &peer->m_peer_address, connection_id, m_data, (m_buffer.m_curBit + 7) >> 3, 0x1000000);
	}
}