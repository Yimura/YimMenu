#pragma once
#include "core/scr_globals.hpp"
#include "core/settings/notifications.hpp"
#include "native_hooks.hpp"

namespace big
{
	namespace shop_controller
	{
		inline void SET_WARNING_MESSAGE_WITH_HEADER(rage::scrNativeCallContext* src)
		{
			if (auto entry_line = src->get_arg<const char*>(1); !strcmp(entry_line, "CTALERT_F_2"))
			{
				if (g_notifications.transaction_rate_limit.log)
					LOG(WARNING) << "Received transaction rate limit";
				if (g_notifications.transaction_rate_limit.notify)
					g_notification_service->push_warning("Transaction Rate Limit", "You're receiving transaction rate limits, whatever you're doing do it less.");

				*scr_globals::transaction_overlimit.as<PBOOL>() = FALSE;

				return;
			}

			HUD::SET_WARNING_MESSAGE_WITH_HEADER(src->get_arg<const char*>(0), src->get_arg<const char*>(1), src->get_arg<int>(2), src->get_arg<const char*>(3), src->get_arg<BOOL>(4), src->get_arg<Any>(5), src->get_arg<Any*>(6), src->get_arg<Any*>(7), src->get_arg<BOOL>(8), src->get_arg<Any>(9));
		}
	}
}