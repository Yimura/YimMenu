#include "view_esp.hpp"

#include "pointers.hpp"
#include "services/players/player_service.hpp"
#include "util/math.hpp"

namespace big
{
	static const ImColor death_bg         = ImColor(0.117f, 0.113f, 0.172f, .75f);
	static const ImColor armor_blue_bg    = ImColor(0.36f, 0.71f, 0.89f, .75f);
	static const ImColor armor_blue       = ImColor(0.36f, 0.71f, 0.89f, 1.f);
	static const ImColor health_green_bg  = ImColor(0.29f, 0.69f, 0.34f, .75f);
	static const ImColor health_green     = ImColor(0.29f, 0.69f, 0.34f, 1.f);
	static const ImColor health_yellow_bg = ImColor(0.69f, 0.49f, 0.29f, .75f);
	static const ImColor health_yellow    = ImColor(0.69f, 0.49f, 0.29f, 1.f);
	static const ImColor health_red_bg    = ImColor(0.69f, 0.29f, 0.29f, .75f);
	static const ImColor health_red       = ImColor(0.69f, 0.29f, 0.29f, 1.f);

	void esp::draw_player(const player_ptr& plyr, ImDrawList* const draw_list)
	{
		auto ped = plyr->get_ped();

		if (!ped || !ped->m_navigation)
			return;

		auto& player_pos = *ped->m_navigation->get_position();

		float screen_x, screen_y;

		const float distance = math::calculate_distance_from_game_cam(player_pos);
		const float multplr  = distance > g.esp.global_render_distance[1] ? -1.f : 6.17757f / distance;

		if (multplr == -1.f || g.esp.global_render_distance[0] > distance)
			return;

		uint32_t ped_damage_bits = ped->m_damage_bits;

		if (g_pointers->m_gta.m_get_screen_coords_for_world_coords(player_pos.data, &screen_x, &screen_y))
		{
			const auto esp_x = (float)*g_pointers->m_gta.m_resolution_x * screen_x;
			const auto esp_y = (float)*g_pointers->m_gta.m_resolution_y * screen_y;

			std::string name_str;
			ImVec2 name_pos = {esp_x - (62.5f * multplr), esp_y - (175.f * multplr) - 20.f};
			ImU32 esp_color = g.esp.default_color;

			if (plyr->is_friend())
			{
				esp_color = g.esp.friend_color;
			}
			else if (g.esp.change_esp_color_from_dist)
			{
				if (distance <= g.esp.distance_threshold[0])
					esp_color = g.esp.enemy_color;
				else if (distance >= g.esp.distance_threshold[0] && distance < g.esp.distance_threshold[1])
					esp_color = g.esp.enemy_near_color;
			}

			const auto armor_perc  = ped->m_armor / 50.f;
			const auto health_perc = ped->m_health / (ped->m_maxhealth + 0.001f);

			if (distance < g.esp.tracer_render_distance[1] && distance > g.esp.tracer_render_distance[0] && g.esp.tracer)
				draw_list->AddLine({(float)*g_pointers->m_gta.m_resolution_x * g.esp.tracer_draw_position[0],
				                       (float)*g_pointers->m_gta.m_resolution_y * g.esp.tracer_draw_position[1]},
				    {esp_x, esp_y},
				    esp_color);

			if (distance < g.esp.box_render_distance[1] && distance > g.esp.box_render_distance[0] && g.esp.box)
				draw_list->AddRect({esp_x - (62.5f * multplr), esp_y - (175.f * multplr)}, {esp_x - (62.5f * multplr) + (125.f * multplr), esp_y - (175.f * multplr) + (350.f * multplr)}, esp_color);

			if (g.esp.name)
				name_str = plyr->get_name();

			if (g.esp.distance)
			{
				if (g.esp.name)
					name_str += " | ";
				name_str += std::to_string((int)distance);
				name_str += "m";
			}

			draw_list->AddText(name_pos, esp_color, name_str.c_str());
			const bool in_god    = ped_damage_bits & (uint32_t)eEntityProofs::GOD;
			std::string mode_str = "";
			if (g.esp.god)
			{
				if (in_god)
				{
					mode_str = "ESP_GOD"_T.data();
				}
				else
				{
					if (ped_damage_bits & (uint32_t)eEntityProofs::BULLET)
					{
						mode_str = "ESP_BULLET"_T.data();
					}
					if (ped_damage_bits & (uint32_t)eEntityProofs::EXPLOSION)
					{
						if (!mode_str.empty())
							mode_str += ", ";
						mode_str += "ESP_EXPLOSION"_T.data();
					}
				}

				if (auto player_vehicle = plyr->get_current_vehicle();
				    player_vehicle && (ped->m_ped_task_flag & (uint32_t)ePedTask::TASK_DRIVING) && (player_vehicle->m_damage_bits & (uint32_t)eEntityProofs::GOD))
				{
					if (!mode_str.empty())
						mode_str += ", ";
					mode_str += "VEHICLE_GOD"_T.data();
				}
			}

			if (!mode_str.empty())
			{
				draw_list->AddText({esp_x - (62.5f * multplr), esp_y - (175.f * multplr) - 40.f},
				    ImColor(1.f, 0.f, 0.f, 1.f),
				    mode_str.c_str());
			}

			if (!in_god)
			{
				if (g.esp.health)
				{
					if (g.esp.scale_health_from_dist)
					{
						draw_list->AddLine({esp_x - (62.5f * multplr), esp_y + (175.f * multplr) + 5.f},
						    {esp_x - (62.5f * multplr) + (125.f * multplr), esp_y + (175.f * multplr) + 5.f},
						    health_perc == 0.f      ? death_bg :
						        health_perc < 0.25f ? health_red_bg :
						        health_perc < 0.65f ? health_yellow_bg :
						                              health_green_bg,
						    4);
						draw_list->AddLine({esp_x - (62.5f * multplr), esp_y + (175.f * multplr) + 5.f},
						    {esp_x - (62.5f * multplr) + (125.f * multplr) * health_perc, esp_y + (175.f * multplr) + 5.f},
						    health_perc < 0.25f     ? health_red :
						        health_perc < 0.65f ? health_yellow :
						                              health_green,
						    4);
					}
					else
					{
						draw_list->AddLine({esp_x - (62.5f * multplr), esp_y + (175.f * multplr) + 5.f},
						    {esp_x - (62.5f * multplr) + (100.f), esp_y + (175.f * multplr) + 5.f},
						    health_perc == 0.f      ? death_bg :
						        health_perc < 0.25f ? health_red_bg :
						        health_perc < 0.65f ? health_yellow_bg :
						                              health_green_bg,
						    4);
						draw_list->AddLine({esp_x - (62.5f * multplr), esp_y + (175.f * multplr) + 5.f},
						    {esp_x - (62.5f * multplr) + (100.f * health_perc), esp_y + (175.f * multplr) + 5.f},
						    health_perc < 0.25f     ? health_red :
						        health_perc < 0.65f ? health_yellow :
						                              health_green,
						    4);
					}
				}
				if (g.esp.armor && ped->m_armor > 0)
				{
					float offset = 5.f;
					offset       = g.esp.health ? 10.f : 5.f;
					if (g.esp.scale_armor_from_dist)
					{
						draw_list->AddLine({esp_x - (62.5f * multplr), esp_y + (175.f * multplr) + offset}, {esp_x - (62.5f * multplr) + (125.f * multplr), esp_y + (175.f * multplr) + offset}, armor_blue_bg, 4);
						draw_list->AddLine({esp_x - (62.5f * multplr), esp_y + (175.f * multplr) + offset}, {esp_x - (62.5f * multplr) + (125.f * multplr) * armor_perc, esp_y + (175.f * multplr) + offset}, armor_blue, 4);
					}
					else
					{
						draw_list->AddLine({esp_x - (62.5f * multplr), esp_y + (175.f * multplr) + offset}, {esp_x - (62.5f * multplr) + (100.f), esp_y + (175.f * multplr) + offset}, armor_blue_bg, 4);
						draw_list->AddLine({esp_x - (62.5f * multplr), esp_y + (175.f * multplr) + offset}, {esp_x - (62.5f * multplr) + (100.f * armor_perc), esp_y + (175.f * multplr) + offset}, armor_blue, 4);
					}
				}
			}
		}
	}

	void esp::draw()
	{
		if (!g.esp.enabled)
			return;

		const auto draw_list = ImGui::GetBackgroundDrawList();

		g_player_service->iterate([draw_list](const player_entry& entry) {
			draw_player(entry.second, draw_list);
		});
	}
}