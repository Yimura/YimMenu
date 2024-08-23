#include "backend/player_command.hpp"
#include "script_function.hpp"

namespace big
{
	class join_ceo : player_command
	{
		using player_command::player_command;

		virtual void execute(player_ptr player, const command_arguments& _args, const std::shared_ptr<command_context> ctx) override
		{
			scr_functions::set_as_ceo.call<void>(player->id(), 0, false, false);
		}
	};

	join_ceo g_join_ceo("joinceo", "JOIN_CEO", "JOIN_CEO_DESC", 0, false);
}