#include "backend/looped_command.hpp"
#include "core/scr_globals.hpp"
#include "natives.hpp"

namespace big
{
	class disable_phone : looped_command
	{
		using looped_command::looped_command;

		virtual void on_tick() override
		{
			*scr_globals::disable_phone.as<PBOOL>() = TRUE;
		}

		virtual void on_disable() override
		{
			*scr_globals::disable_phone.as<PBOOL>() = FALSE;
		}
	};

	disable_phone g_disable_phone("nophone", "DISABLE_PHONE", "DISABLE_PHONE_DESC", g.tunables.disable_phone);
}
