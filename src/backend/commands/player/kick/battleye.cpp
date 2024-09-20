#include "backend/player_command.hpp"
#include "gta_util.hpp"
#include "pointers.hpp"

#include <network/Network.hpp>

#include "packet.hpp"
#include <network/snSession.hpp>

namespace big
{
	class battleye_kick : player_command
	{
		using player_command::player_command;

		virtual CommandAccessLevel get_access_level() override
		{
			return CommandAccessLevel::TOXIC;
		}

		virtual void execute(player_ptr player, const command_arguments& _args, const std::shared_ptr<command_context> ctx) override
		{
			packet msg{};

			msg.write_message(rage::eNetMessage::MsgKickPlayer);
			msg.write<KickReason>(KickReason::BATTLEYE_KICK, 5);
			msg.write<int>(0, 32);
			msg.write<bool>(false, 1);

			auto msg_id = player->get_session_player()->m_msg_id;
			msg.send(msg_id);
		}
	};

	class battleye_ban : player_command
	{
		using player_command::player_command;

		virtual CommandAccessLevel get_access_level() override
		{
			return CommandAccessLevel::TOXIC;
		}

		virtual void execute(player_ptr player, const command_arguments& _args, const std::shared_ptr<command_context> ctx) override
		{
			packet msg{};

			msg.write_message(rage::eNetMessage::MsgKickPlayer);
			msg.write<KickReason>(KickReason::BATTLEYE_BAN, 5);
			msg.write<int>(0, 32);
			msg.write<bool>(false, 1);

			auto msg_id = player->get_session_player()->m_msg_id;
			msg.send(msg_id);
		}
	};

	battleye_kick g_battleye_kick("battlekick", "BATTLEYE_KICK", "BATTLEYE_KICK_DESC", 0);
	battleye_ban g_battleye_ban("battleban", "BATTLEYE_FAKE_BAN", "BATTLEYE_FAKE_BAN_DESC", 0);
}