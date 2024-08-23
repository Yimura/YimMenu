#include "backend/looped/looped.hpp"
#include "core/enums.hpp"
#include "gta/enums.hpp"
#include "util/entity.hpp"
#include "util/math.hpp"

namespace big
{
	static std::vector<Entity> ents = {};
	static Vector3 location;
	static Vector3 other;
	static float dist;
	static constexpr double multiplier = 3.0;

	static const int scroll = 0;

	void apply_velocity(Entity e)
	{
		if (ENTITY::DOES_ENTITY_EXIST(e))
		{
			if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)ControllerInputs::INPUT_WEAPON_WHEEL_NEXT))
				dist -= 5;
			if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)ControllerInputs::INPUT_WEAPON_WHEEL_PREV))
				dist += 5;

			if (!entity::take_control_of(e, 0))
				return; // TODO: remove from vector

			ENTITY::SET_ENTITY_COLLISION(e, false, false);

			other = ENTITY::GET_ENTITY_COORDS(e, true);

			Vector3 rot = CAM::GET_GAMEPLAY_CAM_ROT(2);
			float pitch = math::deg_to_rad(rot.x); // vertical
			// float roll = rot.y;
			float yaw = math::deg_to_rad(rot.z + 90); // horizontal

			Vector3 velocity;

			velocity.x = location.x + (dist * cos(pitch) * cos(yaw)) - other.x;
			velocity.y = location.y + (dist * sin(yaw) * cos(pitch)) - other.y;
			velocity.z = location.z + (dist * sin(pitch)) - other.z;

			ENTITY::SET_ENTITY_VELOCITY(e, velocity.x * (float)multiplier, velocity.y * (float)multiplier, velocity.z * (float)multiplier);
			ENTITY::SET_ENTITY_ALPHA(e, 105, 0);
		}
	}

	void looped::weapons_gravity_gun()
	{
		bool is_gravity_gun_selected = g.weapons.custom_weapon == CustomWeapon::GRAVITY_GUN;

		auto is_zoomed_in = is_gravity_gun_selected && PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)ControllerInputs::INPUT_AIM);
		if (is_zoomed_in && (!g.self.custom_weapon_stop || WEAPON::IS_PED_ARMED(self::ped, 4 | 2)))
		{
			location = self::pos;

			auto is_attack_just_pressed = PAD::IS_DISABLED_CONTROL_JUST_PRESSED(0, (int)ControllerInputs::INPUT_ATTACK);

			if (is_attack_just_pressed)
			{
				Entity ent_to_add;

				if (entity::raycast(&ent_to_add))
				{
					if (ENTITY::IS_ENTITY_A_PED(ent_to_add) && PED::IS_PED_A_PLAYER(ent_to_add))
					{
						g_notification_service.push_warning("CUSTOM_WEAPONS"_T.data(), "BACKEND_LOOPED_WEAPONS_GRAVITY_GUN_PLAYER"_T.data());
					}
					else
					{
						other               = ENTITY::GET_ENTITY_COORDS(ent_to_add, true);
						const int temp_dist = (float)math::distance_between_vectors(location, other);

						if (ents.size() == 0)
						{
							dist = temp_dist;
						}

						if (temp_dist > 500)
						{
							g_notification_service.push_warning("CUSTOM_WEAPONS"_T.data(), "BACKEND_LOOPED_WEAPONS_DELETE_GUN_TOO_FAR"_T.data());
						}
						else
						{
							if (entity::take_control_of(ent_to_add, 0) && ENTITY::IS_ENTITY_A_PED(ent_to_add) && !PED::IS_PED_RAGDOLL(ent_to_add))
							{
								TASK::SET_HIGH_FALL_TASK(ent_to_add, 0, 0, 0);

								g_notification_service.push_warning("CUSTOM_WEAPONS"_T.data(), "BACKEND_LOOPED_WEAPONS_GRAVITY_GUN_SET"_T.data());
							}

							ents.push_back(ent_to_add);
						}
					}
				}
			}
			if (ents.size() > 0)
			{
				for (const auto& e : ents)
				{
					apply_velocity(e);
				}
			}
		}
		else if (ents.size() > 0)
		{
			for (const Entity& e : ents)
			{
				if (entity::take_control_of(e, 0))
				{
					if (g.weapons.gravity_gun.launch_on_release)
					{
						dist += 100;
						apply_velocity(e);
					}
					ENTITY::SET_ENTITY_COLLISION(e, true, true);
					ENTITY::RESET_ENTITY_ALPHA(e);
				}
			}

			ents.clear();

			g_notification_service.push_success("CUSTOM_WEAPONS"_T.data(), "BACKEND_LOOPED_WEAPONS_GRAVITY_GUN_UNSET"_T.data());
		}
	}
}
