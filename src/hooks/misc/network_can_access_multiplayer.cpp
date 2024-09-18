#include "hooking/hooking.hpp"

namespace big
{
	bool hooks::network_can_access_multiplayer(void* a1, int* error)
	{
		if (error)
			*error = 0;

		return true;
	}
}
