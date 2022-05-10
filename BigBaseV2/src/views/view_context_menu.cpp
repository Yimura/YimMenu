#include "view.hpp"
#include "services/context_menu_service.hpp"

namespace big
{
	void view::context_menu()
	{
		if (const auto draw_list = ImGui::GetBackgroundDrawList(); draw_list)
		{
			if (g_context_menu_service->enabled &&
				g_context_menu_service->pointer &&
				g_context_menu_service->pointer->m_navigation)
			{
				float context_screen_x;
				float context_screen_y;

				rage::fvector3& context_target_pos = g_context_menu_service->pointer->m_navigation->m_position;

				const float context_target_distance = math::calculate_distance_from_game_cam(context_target_pos);
				const float context_target_multplr = context_target_distance > g->esp.global_render_distance[1] ? -1.f : 6.17757f / context_target_distance;

				if (g_pointers->m_get_screen_coords_for_world_coords(context_target_pos.data, &context_screen_x, &context_screen_y))
				{
					s_context_menu* cm = g_context_menu_service->get_context_menu();

					float cm_start_x = (float)*g_pointers->m_resolution_x * context_screen_x + (67.5f * context_target_multplr);
					float cm_start_y = (float)*g_pointers->m_resolution_y * context_screen_y - (175.f * context_target_multplr);

					ImU32 cm_col = ImGui::ColorConvertFloat4ToU32({ 0.549f, 0.639f, 0.710f, 0.3f });

					draw_list->AddRectFilled({ cm_start_x - 2.f , cm_start_y }, { cm_start_x + 2.f + cm->menu_size.x, cm_start_y + cm->menu_size.y }, cm_col, 5.f);

					for (int i = 0; i < cm->options.size(); i++)
					{
						context_option co = cm->options.at(i);
						draw_list->AddText({ cm_start_x + 7.f, cm_start_y + (20.f * (float)i) + 5.f }, cm->current_option == i ? ImGui::ColorConvertFloat4ToU32({ 0.f, 1.f, 0.f, 1.f }) : ImGui::ColorConvertFloat4ToU32({ 1.f, 1.f, 1.f, 1.f }), co.name.c_str());
					}
				}
			}
		}
	}
}