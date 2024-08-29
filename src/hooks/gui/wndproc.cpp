#include "hooking/hooking.hpp"
#include "renderer/renderer.hpp"
#include "util/input_method_editor.hpp"

namespace big
{
	LRESULT hooks::wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		if (g_running) [[likely]]
		{
			g_renderer.wndproc(hwnd, msg, wparam, lparam);

			if (msg == WM_IME_COMPOSITION && lparam & GCS_RESULTSTR) [[unlikely]]
			{
				handle_ime_result();
			}
		}

		return CallWindowProcW(g_hooking->m_og_wndproc, hwnd, msg, wparam, lparam);
	}
}
