#include "context_menu_service.hpp"

#include "fiber_pool.hpp"
#include "gui.hpp"
#include "natives.hpp"
#include "pointers.hpp"
#include "util/misc.hpp"

#include <network/ChatData.hpp>

namespace big
{
	context_menu_service::context_menu_service()
	{
		g_context_menu_service = this;
		load_shared();
	}

	context_menu_service::~context_menu_service()
	{
		g_context_menu_service = nullptr;
	}

	void context_menu_service::fill_model_bounding_box_screen_space()
	{
		Vector3 forward, right, up, pos;
		ENTITY::GET_ENTITY_MATRIX(m_handle, &forward, &right, &up, &pos);

		const auto hash = ENTITY::GET_ENTITY_MODEL(m_handle);
		Vector3 min, max;
		MISC::GET_MODEL_DIMENSIONS(hash, &min, &max);
		const auto dimensions = (max - min) * 0.5f;

		// sanity check
		if (!m_pointer || !m_pointer->m_navigation)
			return;
		// make copy just in case
		const auto position = *m_pointer->m_navigation->get_position();

		rage::fvector3 front_upper_right, back_lower_left;
		front_upper_right.x = position.x + dimensions.y * forward.x + dimensions.x * right.x + dimensions.z * up.x;
		front_upper_right.y = position.y + dimensions.y * forward.y + dimensions.x * right.y + dimensions.z * up.y;
		front_upper_right.z = position.z + dimensions.y * forward.z + dimensions.x * right.z + dimensions.z * up.z;

		back_lower_left.x = position.x - dimensions.y * forward.x - dimensions.x * right.x - dimensions.z * up.x;
		back_lower_left.y = position.y - dimensions.y * forward.y - dimensions.x * right.y - dimensions.z * up.y;
		back_lower_left.z = position.z - dimensions.y * forward.z - dimensions.x * right.z - dimensions.z * up.z;

		rage::fvector3 edge1 = back_lower_left;
		rage::fvector3 edge2, edge3, edge4;

		rage::fvector3 edge5 = front_upper_right;
		rage::fvector3 edge6, edge7, edge8;

		edge2.x = edge1.x + 2 * dimensions.y * forward.x;
		edge2.y = edge1.y + 2 * dimensions.y * forward.y;
		edge2.z = edge1.z + 2 * dimensions.y * forward.z;

		edge3.x = edge2.x + 2 * dimensions.z * up.x;
		edge3.y = edge2.y + 2 * dimensions.z * up.y;
		edge3.z = edge2.z + 2 * dimensions.z * up.z;

		edge4.x = edge1.x + 2 * dimensions.z * up.x;
		edge4.y = edge1.y + 2 * dimensions.z * up.y;
		edge4.z = edge1.z + 2 * dimensions.z * up.z;

		edge6.x = edge5.x - 2 * dimensions.y * forward.x;
		edge6.y = edge5.y - 2 * dimensions.y * forward.y;
		edge6.z = edge5.z - 2 * dimensions.y * forward.z;

		edge7.x = edge6.x - 2 * dimensions.z * up.x;
		edge7.y = edge6.y - 2 * dimensions.z * up.y;
		edge7.z = edge6.z - 2 * dimensions.z * up.z;

		edge8.x = edge5.x - 2 * dimensions.z * up.x;
		edge8.y = edge5.y - 2 * dimensions.z * up.y;
		edge8.z = edge5.z - 2 * dimensions.z * up.z;

		auto any_fail                     = false;
		static auto imgui_world_to_screen = [&any_fail](rage::fvector3& world_input, ImVec2& screen_result) {
			if (any_fail)
			{
				return;
			}

			const auto success = GRAPHICS::GET_SCREEN_COORD_FROM_WORLD_COORD(world_input.x,
			    world_input.y,
			    world_input.z,
			    &screen_result.x,
			    &screen_result.y);
			if (success)
			{
				screen_result.x = static_cast<float>(*g_pointers->m_gta.m_resolution_x) * screen_result.x;
				screen_result.y = static_cast<float>(*g_pointers->m_gta.m_resolution_y) * screen_result.y;
			}
			else
			{
				any_fail = true;
			}
		};

		auto& box = m_model_bounding_box_screen_space;
		imgui_world_to_screen(edge1, box.edge1);
		imgui_world_to_screen(edge2, box.edge2);
		imgui_world_to_screen(edge3, box.edge3);
		imgui_world_to_screen(edge4, box.edge4);

		imgui_world_to_screen(edge5, box.edge5);
		imgui_world_to_screen(edge6, box.edge6);
		imgui_world_to_screen(edge7, box.edge7);
		imgui_world_to_screen(edge8, box.edge8);

		if (any_fail)
		{
			box = {};
		}
	}

	double context_menu_service::distance_to_middle_of_screen(const rage::fvector2& screen_pos)
	{
		double cum_dist{};

		if (screen_pos.x > 0.5)
			cum_dist += screen_pos.x - 0.5;
		else
			cum_dist += 0.5 - screen_pos.x;

		if (screen_pos.y > 0.5)
			cum_dist += screen_pos.y - 0.5;
		else
			cum_dist += 0.5 - screen_pos.y;

		return cum_dist;
	}

	s_context_menu* context_menu_service::get_context_menu()
	{
		if (m_pointer && m_pointer->m_model_info)
		{
			switch (m_pointer->m_model_info->get_model_type())
			{
			case eModelType::Object:
			{
				if (!misc::has_bits_set(&g.context_menu.allowed_entity_types, static_cast<uint8_t>(ContextEntityType::OBJECT)))
				{
					break;
				}
				return &options.at(ContextEntityType::OBJECT);
			}
			case eModelType::Ped:
			{
				if (const auto ped = reinterpret_cast<CPed*>(m_pointer); ped)
				{
					if (ped->m_ped_task_flag & static_cast<uint8_t>(ePedTask::TASK_DRIVING) && ped->m_vehicle)
					{
						if (!misc::has_bits_set(&g.context_menu.allowed_entity_types, static_cast<uint8_t>(ContextEntityType::VEHICLE)))
						{
							break;
						}

						m_pointer = ped->m_vehicle;
						return &options.at(ContextEntityType::VEHICLE);
					}
					if (ped->m_player_info)
					{
						if (!misc::has_bits_set(&g.context_menu.allowed_entity_types, static_cast<uint8_t>(ContextEntityType::PLAYER)))
						{
							break;
						}

						return &options.at(ContextEntityType::PLAYER);
					}
				}

				if (!misc::has_bits_set(&g.context_menu.allowed_entity_types, static_cast<uint8_t>(ContextEntityType::PED)))
				{
					break;
				}

				return &options.at(ContextEntityType::PED);
			}
			case eModelType::Vehicle:
			{
				if (!misc::has_bits_set(&g.context_menu.allowed_entity_types, static_cast<uint8_t>(ContextEntityType::VEHICLE)))
				{
					break;
				}

				return &options.at(ContextEntityType::VEHICLE);
			}
			default: break;
			}
		}
		return nullptr;
	}

	void context_menu_service::get_entity_closest_to_screen_center()
	{
		m_handle = entity::get_entity_closest_to_middle_of_screen(&m_pointer);
		if (ENTITY::DOES_ENTITY_EXIST(m_handle) && m_pointer)
			fill_model_bounding_box_screen_space();
	}

	void context_menu_service::load_shared()
	{
		for (auto& [type, menu] : options)
		{
			if (type == ContextEntityType::SHARED)
				continue;
			menu.options.insert(menu.options.end(),
			    options.at(ContextEntityType::SHARED).options.begin(),
			    options.at(ContextEntityType::SHARED).options.end());

			uint32_t max_size = 0;
			for (auto& [name, _] : menu.options)
			{
				max_size = static_cast<int>(max_size < name.length() ? name.length() : max_size);
			}

			menu.menu_size = {(10.f * static_cast<float>(max_size)) + 10.f, 2 * (10.f * static_cast<float>(menu.options.size())) + 10.f};
		}
	}

	static const ControllerInputs controls[] = {
	    ControllerInputs::INPUT_NEXT_WEAPON,
	    ControllerInputs::INPUT_PREV_WEAPON,
	    ControllerInputs::INPUT_VEH_NEXT_RADIO,
	    ControllerInputs::INPUT_VEH_SELECT_NEXT_WEAPON,
	    ControllerInputs::INPUT_SELECT_NEXT_WEAPON,
	    ControllerInputs::INPUT_SELECT_PREV_WEAPON,
	    ControllerInputs::INPUT_ATTACK,
	    ControllerInputs::INPUT_ATTACK2,
	    ControllerInputs::INPUT_SPECIAL_ABILITY,
	    ControllerInputs::INPUT_VEH_MOUSE_CONTROL_OVERRIDE,
	    ControllerInputs::INPUT_SNIPER_ZOOM,
	};

	void context_menu_service::disable_control_action_loop()
	{
		if (g_context_menu_service->enabled)
		{
			for (const auto& control : controls)
				PAD::DISABLE_CONTROL_ACTION(0, static_cast<int>(control), true);
		}
	}

	void context_menu_service::context_menu()
	{
		while (g_running)
		{
			if (g_gui->is_open() || HUD::IS_PAUSE_MENU_ACTIVE()
			    || (*g_pointers->m_gta.m_chat_data && (*g_pointers->m_gta.m_chat_data)->m_chat_open) || g.cmd_executor.enabled)
			{
				script::get_current()->yield();
				continue;
			}

			if (!g.context_menu.enabled)
			{
				g_context_menu_service->enabled = false;

				script::get_current()->yield();
				continue;
			}

			if (PAD::IS_USING_KEYBOARD_AND_MOUSE(0))
			{
				if (PAD::IS_DISABLED_CONTROL_JUST_RELEASED(0, (int)ControllerInputs::INPUT_VEH_DUCK))
				{
					g_context_menu_service->enabled = !g_context_menu_service->enabled;
				}
			}
			else
			{
				if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)ControllerInputs::INPUT_AIM) && PAD::IS_DISABLED_CONTROL_JUST_RELEASED(0, (int)ControllerInputs::INPUT_FRONTEND_Y))
				{
					g_context_menu_service->enabled = !g_context_menu_service->enabled;
				}
			}

			if (g_context_menu_service->enabled)
			{
				HUD::SHOW_HUD_COMPONENT_THIS_FRAME(static_cast<int>(HudComponents::RETICLE));

				g_context_menu_service->get_entity_closest_to_screen_center();

				const auto cm = g_context_menu_service->get_context_menu();
				if (cm == nullptr)
				{
					script::get_current()->yield();
					continue;
				}

				ControllerInputs next_key = PAD::IS_USING_KEYBOARD_AND_MOUSE(0) ? ControllerInputs::INPUT_WEAPON_WHEEL_NEXT : ControllerInputs::INPUT_SCRIPT_PAD_DOWN;
				ControllerInputs prev_key = PAD::IS_USING_KEYBOARD_AND_MOUSE(0) ? ControllerInputs::INPUT_WEAPON_WHEEL_PREV : ControllerInputs::INPUT_SCRIPT_PAD_UP;
				ControllerInputs execute_key = PAD::IS_USING_KEYBOARD_AND_MOUSE(0) ? ControllerInputs::INPUT_ATTACK : ControllerInputs::INPUT_FRONTEND_ACCEPT;

				PAD::DISABLE_CONTROL_ACTION(0, static_cast<int>(next_key), true);
				PAD::DISABLE_CONTROL_ACTION(0, static_cast<int>(prev_key), true);
				PAD::DISABLE_CONTROL_ACTION(0, static_cast<int>(execute_key), true);

				if (PAD::IS_DISABLED_CONTROL_JUST_PRESSED(0, (int)next_key))
					cm->current_option = cm->options.size() <= cm->current_option + 1 ? 0 : cm->current_option + 1;
				if (PAD::IS_DISABLED_CONTROL_JUST_PRESSED(0, (int)prev_key))
					cm->current_option = 0 > cm->current_option - 1 ? static_cast<int>(cm->options.size()) - 1 : cm->current_option - 1;

				if (PAD::IS_DISABLED_CONTROL_JUST_PRESSED(0, (int)execute_key))
				{
					if (!g_context_menu_service->m_pointer)
					{
						script::get_current()->yield();
						continue;
					}

					g_fiber_pool->queue_job([cm] {
						cm->options.at(cm->current_option).command();
					});
				}
			}

			script::get_current()->yield();
		}
	}
}
