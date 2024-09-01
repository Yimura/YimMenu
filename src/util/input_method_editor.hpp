#pragma once
#include "common.hpp"
#include "pointers.hpp"

#include <imgui_internal.h>
#include <misc/InputMethodEditor.hpp>

namespace
{
	// https://github.com/ocornut/imgui/blob/864a2bf6b824f9c1329d8493386208d4b0fd311c/imgui_widgets.cpp#L3948
	static void insert_into_input_text(ImGuiInputTextState* obj, const ImWchar* new_text, int new_text_len)
	{
		const bool is_resizable = (obj->Flags & ImGuiInputTextFlags_CallbackResize) != 0;
		const int pos           = obj->Stb.cursor;
		const int text_len      = obj->CurLenW;

		const int new_text_len_utf8 = ImTextCountUtf8BytesFromStr(new_text, new_text + new_text_len);
		if (!is_resizable && (new_text_len_utf8 + obj->CurLenA + 1 > obj->BufCapacityA))
			return;

		// Grow internal buffer if needed
		if (new_text_len + text_len + 1 > obj->TextW.Size)
		{
			if (!is_resizable)
				return;

			obj->TextW.resize(text_len + ImClamp(new_text_len * 4, 32, ImMax(256, new_text_len)) + 1);
		}

		ImWchar* text = obj->TextW.Data;
		if (pos != text_len)
			memmove(text + pos + new_text_len, text + pos, (size_t)(text_len - pos) * sizeof(ImWchar));
		memcpy(text + pos, new_text, (size_t)new_text_len * sizeof(ImWchar));

		obj->Edited = true;
		obj->CurLenW += new_text_len;
		obj->CurLenA += new_text_len_utf8;
		obj->TextW[obj->CurLenW] = '\0';

		obj->Stb.cursor += new_text_len;
		obj->Stb.has_preferred_x = 0;

		obj->CursorFollow = true;
	}

	static void char16_to_char(char* out, const char16_t* in)
	{
		while (*in)
		{
			char16_t c = *in++;

			if (c < 0x80)
			{
				*out++ = static_cast<char>(c);
			}
			else if (c < 0x800)
			{
				*out++ = static_cast<char>(0xC0 | (c >> 6));
				*out++ = static_cast<char>(0x80 | (c & 0x3F));
			}
			else
			{
				*out++ = static_cast<char>(0xE0 | (c >> 12));
				*out++ = static_cast<char>(0x80 | ((c >> 6) & 0x3F));
				*out++ = static_cast<char>(0x80 | (c & 0x3F));
			}
		}
		*out = '\0';
	}
}

namespace big
{
	inline void handle_ime_result()
	{
		auto state = ImGui::GetInputTextState(ImGui::GetActiveID());

		if (!state)
			return;

		auto context = ImmGetContext(g_pointers->m_hwnd);

		ImWchar buf[31]{};
		int len = ImmGetCompositionStringW(context, GCS_RESULTSTR, buf, sizeof(buf) - 1) / sizeof(ImWchar);

		insert_into_input_text(state, buf, len);

		ImmReleaseContext(g_pointers->m_hwnd, context);
	}

	inline void draw_input_method_editor()
	{
		if (!g_pointers->m_gta.m_ime->m_active)
			return;

		constexpr size_t buf_size = ARRAYSIZE(InputMethodEditor::m_composition_string) * 3;
		char buf[buf_size];
		char16_to_char(buf, g_pointers->m_gta.m_ime->m_composition_string);

		std::string text = buf;

		for (uint32_t i = 0; i < g_pointers->m_gta.m_ime->m_count; ++i)
		{
			char16_to_char(buf, g_pointers->m_gta.m_ime->m_candidate_list[i]);

			text += '\n';
			text += (i == g_pointers->m_gta.m_ime->m_selected_index ? '>' : ' ');
			text += std::to_string(i + 1);
			text += '\t';
			text += buf;
		}

		constexpr float pd = 7.5f; // padding
		constexpr float lt = 1.f;  // line thickness

		ImVec2 ts = ImGui::CalcTextSize(text.c_str());

		ImVec2 bl = ImGui::GetItemRectMin();      // bottom-left
		ImVec2 br = {bl.x + ts.x + 2 * pd, bl.y}; // bottom-right
		ImVec2 tl = {bl.x, bl.y - ts.y - 2 * pd}; // top-left
		ImVec2 tr = {br.x, tl.y};                 // top-right

		auto dl = ImGui::GetForegroundDrawList();

		dl->AddRectFilled(tl, br, g.window.background_color | IM_COL32_A_MASK);
		dl->AddText({tl.x + pd, tl.y + pd}, g.window.text_color, text.c_str());

		dl->AddLine(tl, tr, IM_COL32_BLACK);                               // top
		dl->AddLine({bl.x, bl.y - lt}, {br.x, br.y - lt}, IM_COL32_BLACK); // bottom
		dl->AddLine(tl, bl, IM_COL32_BLACK);                               // left
		dl->AddLine({tr.x - lt, tr.y}, {br.x - lt, br.y}, IM_COL32_BLACK); // right
	}
}