#include "backend/looped_command.hpp"
#include "services/tunables/tunables_service.hpp"

namespace big
{
	class allow_weapons_in_interiors : looped_command
	{
		using looped_command::looped_command;

		virtual void on_tick() override
		{
			if (auto tunable = g_tunables_service->get_tunable<PBOOL>(-298441869))
			{
				*tunable = FALSE;
			}
		}

		virtual void on_disable() override
		{
			if (auto tunable = g_tunables_service->get_tunable<PBOOL>(-298441869))
			{
				*tunable = TRUE;
			}
		}
	};

	allow_weapons_in_interiors g_disable_phone("allowwepsinside", "VIEW_WEAPON_ALLOW_WEAPONS_IN_INTERIORS", "", g.weapons.interior_weapon);
}
