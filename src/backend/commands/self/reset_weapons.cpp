#include "backend/command.hpp"
#include "script_function.hpp"

namespace big
{
	class reset_weapons : command
	{
		using command::command;

		virtual void execute(const command_arguments&, const std::shared_ptr<command_context> ctx)
		{
			if (*g_pointers->m_gta.m_is_session_started && scripts::is_running("freemode"_J))
			{
				*scr_globals::should_reset_fm_weapons.as<PBOOL>() = TRUE;
				scr_functions::give_owned_freemode_weapons.call<void>();
			}
		}
	};

	reset_weapons g_reset_weapons("resetweaps", "RESET_WEAPONS", "RESET_WEAPONS_DESC", 0);
}