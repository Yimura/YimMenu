#pragma once
#include "common.hpp"
#include "pointers.hpp"

#include <imgui_internal.h>

#pragma pack(push, 8)
class InputMethodEditor
{
public:
	uint32_t m_count; //0x0000
	uint32_t m_selected_index; //0x0004
	wchar_t m_composition_string[31]; //0x0008
	wchar_t m_candidate_list[9][31]; //0x0046
	bool m_active; //0x0274
	char pad_0275[3]; //0x0275
}; //Size: 0x0278
static_assert(sizeof(InputMethodEditor) == 0x278);
#pragma pack(pop)

namespace
{
	// https://github.com/ocornut/imgui/blob/864a2bf6b824f9c1329d8493386208d4b0fd311c/imgui_widgets.cpp#L3948
	static bool STB_TEXTEDIT_INSERTCHARS(ImGuiInputTextState* obj, int pos, const ImWchar* new_text, int new_text_len)
	{
		const bool is_resizable = (obj->Flags & ImGuiInputTextFlags_CallbackResize) != 0;
		const int text_len      = obj->CurLenW;
		IM_ASSERT(pos <= text_len);

		const int new_text_len_utf8 = ImTextCountUtf8BytesFromStr(new_text, new_text + new_text_len);
		if (!is_resizable && (new_text_len_utf8 + obj->CurLenA + 1 > obj->BufCapacityA))
			return false;

		// Grow internal buffer if needed
		if (new_text_len + text_len + 1 > obj->TextW.Size)
		{
			if (!is_resizable)
				return false;
			IM_ASSERT(text_len < obj->TextW.Size);
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

		return true;
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

		wchar_t buf[31]{};
		int len = ImmGetCompositionStringW(context, GCS_RESULTSTR, buf, sizeof(buf) - 1) / 2;

		if (STB_TEXTEDIT_INSERTCHARS(state, state->Stb.cursor, (ImWchar*)buf, len))
		{
			state->Stb.cursor += len;
			state->Stb.has_preferred_x = 0;

			state->CursorFollow = true;
		}

		ImmReleaseContext(g_pointers->m_hwnd, context);
	}

	inline void draw_input_method_editor()
	{
		if (!g_pointers->m_gta.m_ime->m_active)
			return;

		std::string text;

		char buf[62];
		ImTextStrToUtf8(buf, sizeof(buf), (ImWchar*)g_pointers->m_gta.m_ime->m_composition_string, nullptr);

		text += buf;

		for (uint32_t i = 0; i < g_pointers->m_gta.m_ime->m_count; ++i)
		{
			ImTextStrToUtf8(buf, sizeof(buf), (ImWchar*)g_pointers->m_gta.m_ime->m_candidate_list[i], nullptr);

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