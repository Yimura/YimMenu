#include "fiber_pool.hpp"
#include "gui/components/components.hpp"
#include "misc/cpp/imgui_stdlib.h"
#include "util/input_method_editor.hpp"

namespace big
{
	bool components::input_text(const std::string_view label, char* buf, size_t buf_size, ImGuiInputTextFlags_ flag, std::function<void()> cb)
	{
		bool retval = false;
		if (ImGui::InputText(label.data(), buf, buf_size, flag))
		{
			if (cb)
				g_fiber_pool->queue_job(std::move(cb));
			retval = true;
		}

		if (ImGui::IsItemActive())
		{
			g.self.hud.typing = TYPING_TICKS;

			draw_input_method_editor();

			*g_pointers->m_gta.m_allow_keyboard_layout_change = true;
		}

		if (ImGui::IsItemDeactivated())
		{
			*g_pointers->m_gta.m_allow_keyboard_layout_change = false;
		}

		return retval;
	}

	bool components::input_text(const std::string_view label, std::string& buf, ImGuiInputTextFlags_ flag, std::function<void()> cb)
	{
		bool retval = false;
		if (ImGui::InputText(label.data(), &buf, flag))
		{
			if (cb)
				g_fiber_pool->queue_job(std::move(cb));
			retval = true;
		}

		if (ImGui::IsItemActive())
		{
			g.self.hud.typing = TYPING_TICKS;

			draw_input_method_editor();

			*g_pointers->m_gta.m_allow_keyboard_layout_change = true;
		}

		if (ImGui::IsItemDeactivated())
		{
			*g_pointers->m_gta.m_allow_keyboard_layout_change = false;
		}

		return retval;
	}
}