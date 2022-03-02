#include "core/data/custom_weapons.hpp"
#include "fiber_pool.hpp"
#include "gta/Weapons.h"
#include "script.hpp"
#include "views/view.hpp"

namespace big
{
	void view::weapons() {
		if (ImGui::TreeNode("Ammo Options"))
		{
			ImGui::Checkbox("Infinite Ammo", &g->weapons.infinite_ammo);

			ImGui::SameLine();

			ImGui::Checkbox("Infinite Clip", &g->weapons.infinite_mag);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Misc"))
		{
			ImGui::Checkbox("Force Crosshairs", &g->weapons.force_crosshairs);

			ImGui::SameLine();

			ImGui::Checkbox("No Recoil", &g->weapons.no_recoil);

			ImGui::SameLine();

			ImGui::Checkbox("No Spread", &g->weapons.no_spread);

			if (ImGui::Button("Get All Weapons"))
			{
				QUEUE_JOB_BEGIN_CLAUSE()
				{
					for (auto const& weapon : weapon_list) {
						WEAPON::GIVE_DELAYED_WEAPON_TO_PED(PLAYER::PLAYER_PED_ID(), weapon, 9999, false);
					}
					WEAPON::GIVE_DELAYED_WEAPON_TO_PED(PLAYER::PLAYER_PED_ID(), -72657034, 0, true);
				}
				QUEUE_JOB_END_CLAUSE
			}

			ImGui::SliderFloat("Damage Multiplier", &g->weapons.increased_damage, 1.f, 10.f, "%.1f");
			
			ImGui::TreePop();
		}
		
		if (ImGui::TreeNode("Custom Weapons"))
		{
			CustomWeapon selected = g->weapons.custom_weapon;

			if (ImGui::BeginCombo("Weapon", custom_weapons[(int)selected].name))
			{
				for (const custom_weapon& weapon : custom_weapons)
				{
					if (ImGui::Selectable(weapon.name, weapon.id == selected))
					{
						g->weapons.custom_weapon = weapon.id;
					}

					if (weapon.id == selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			switch (selected)
			{
			case CustomWeapon::NONE:
				break;
			case CustomWeapon::CAGE_GUN:
				break;
			case CustomWeapon::DELETE_GUN:
				break;
			case CustomWeapon::GRAVITY_GUN:
				break;
			case CustomWeapon::REPAIR_GUN:
				break;
			case CustomWeapon::VEHICLE_GUN:
				ImGui::Text("Shooting Model:");
				ImGui::InputText("##vehicle_gun_model", g->weapons.vehicle_gun_model, 12);

				break;
			}

			ImGui::TreePop();
		}
	}
}