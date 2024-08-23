#pragma once

namespace big
{
	namespace am_pi_menu
	{
		static void DISPLAY_ONSCREEN_KEYBOARD(rage::scrNativeCallContext* src)
		{
			auto input_type       = src->get_arg<int>(0);
			auto window_title     = src->get_arg<const char*>(1);
			auto unk1             = src->get_arg<const char*>(2);
			auto input_default    = src->get_arg<const char*>(3);
			auto unk2             = src->get_arg<const char*>(4);
			auto unk3             = src->get_arg<const char*>(5);
			auto unk4             = src->get_arg<const char*>(6);
			auto max_input_length = src->get_arg<int>(7);

			if (max_input_length == 15)
				max_input_length = 41;

			MISC::DISPLAY_ONSCREEN_KEYBOARD(input_type, window_title, unk1, input_default, unk2, unk3, unk4, max_input_length);
		}
	}
}
