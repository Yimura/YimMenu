#include "script_local.hpp"
#include "util/math.hpp"
#include "util/scripts.hpp"
#include "util/teleport.hpp"
#include "views/view.hpp"

namespace big
{
	int get_land_mark_beast_is_closest_to(player_ptr player, script_local land_mark_list)
	{
		if (!player->get_ped() || !player->get_ped()->m_navigation)
			return -1;

		int closest_index          = 0;
		Vector3 transformed_vector = Vector3(player->get_ped()->m_navigation->get_position()->x,
		    player->get_ped()->m_navigation->get_position()->y,
		    player->get_ped()->m_navigation->get_position()->z);
		float distance = math::distance_between_vectors(transformed_vector, *land_mark_list.at(0, 3).as<Vector3*>());

		for (int i = 1; i < *script_global(262145).at(11711).as<int*>(); i++)
		{
			float new_distance = math::distance_between_vectors(transformed_vector, *land_mark_list.at(i, 3).as<Vector3*>());
			if (new_distance < distance)
			{
				distance      = new_distance;
				closest_index = i;
			}
		}

		return closest_index;
	}

	inline void render_hunt_the_beast_ui()
	{
		if (auto hunt_the_beast_script_thread = gta_util::find_script_thread(RAGE_JOAAT("am_hunt_the_beast")))
		{
			auto beast_player_index = *script_local(hunt_the_beast_script_thread, 599).at(1).at(6).as<uint32_t*>();
			if (g_player_service->get_by_id(beast_player_index))
			{
				ImGui::Text("%s is the beast", g_player_service->get_by_id(beast_player_index).get()->get_name());
				ImGui::SameLine();
				components::button("Set as selected", [beast_player_index] {
					g_player_service->set_selected(g_player_service->get_by_id(beast_player_index));
				});

				ImGui::Spacing();

				auto beast_land_marks = *script_global(262145).at(11711).as<int*>();

				if (ImGui::ListBoxHeader("##beastlandmarks", ImVec2(400, 300)))
				{
					script_local beast_land_mark_list = script_local(hunt_the_beast_script_thread, 599).at(1).at(19);

					for (int i = 0; i < beast_land_marks; i++)
					{
						auto script_local_land_mark = *beast_land_mark_list.at(i, 3).as<Vector3*>();
						std::string label           = std::format("Tp to landmark {} at {}  {}  {}",
                            i,
                            script_local_land_mark.x,
                            script_local_land_mark.y,
                            script_local_land_mark.z);

						if (ImGui::Selectable(label.data(), i == get_land_mark_beast_is_closest_to(g_player_service->get_by_id(beast_player_index), beast_land_mark_list)))
							teleport::to_coords(script_local_land_mark);
					}
					ImGui::ListBoxFooter();
				}
			}
			else
			{
				ImGui::Text("Hunt the beast event is active...");
			}
		}
	}
}