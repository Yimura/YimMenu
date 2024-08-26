#include "core/data/bullet_impact_types.hpp"
#include "core/data/special_ammo_types.hpp"
#include "gta/joaat.hpp"
#include "services/gta_data/gta_data_service.hpp"
#include "views/view.hpp"
#include "imgui_internal.h"

namespace big
{
	struct custom_weapon
	{
		big::CustomWeapon id;
		const char* name;
	};

	constexpr auto custom_weapons = std::to_array<custom_weapon>({
	    {big::CustomWeapon::NONE, "VIEW_SELF_WEAPONS_NONE"},
	    {big::CustomWeapon::CAGE_GUN, "VIEW_SELF_WEAPONS_CAGE_GUN"},
	    {big::CustomWeapon::DELETE_GUN, "VIEW_SELF_WEAPONS_DELETE_GUN"},
	    {big::CustomWeapon::GRAVITY_GUN, "VIEW_SELF_WEAPONS_GRAVITY_GUN"},
	    {big::CustomWeapon::STEAL_VEHICLE_GUN, "BACKEND_LOOPED_WEAPONS_STEAL_VEHICLE_GUN"},
	    {big::CustomWeapon::REPAIR_GUN, "BACKEND_LOOPED_WEAPONS_REPAIR_GUN"},
	    {big::CustomWeapon::VEHICLE_GUN, "BACKEND_LOOPED_WEAPONS_VEHICLE_GUN"},
	    {big::CustomWeapon::TP_GUN, "VIEW_SELF_WEAPONS_TP_GUN"},
	    {big::CustomWeapon::PAINT_GUN, "VIEW_SELF_WEAPONS_PAINT_GUN"},
	});

	void view::weapons()
	{
		ImGui::SeparatorText("AMMO"_T.data());

		ImGui::BeginGroup();

		components::command_checkbox<"infammo">();
		components::command_checkbox<"alwaysfullammo">();
		components::command_checkbox<"infclip">();
		components::command_checkbox<"infrange">();
		components::command_checkbox<"allowwepsinside">();

		ImGui::EndGroup();
		ImGui::SameLine();
		ImGui::BeginGroup();

		ImGui::Checkbox("VIEW_WEAPON_INCREASE_C4_LIMIT"_T.data(), &g.weapons.increased_c4_limit);
		ImGui::Checkbox("VIEW_WEAPON_INCREASE_FLARE_LIMIT"_T.data(), &g.weapons.increased_flare_limit);

		components::command_checkbox<"rapidfire">();

		ImGui::Checkbox("ENABLE_SPECIAL_AMMO"_T.data(), &g.weapons.ammo_special.toggle);
		components::options_modal("SPECIAL_AMMO"_T.data(), [] {
			eAmmoSpecialType selected_ammo   = g.weapons.ammo_special.type;
			eExplosionTag selected_explosion = g.weapons.ammo_special.explosion_tag;

			if (ImGui::BeginCombo("SPECIAL_AMMO"_T.data(), SPECIAL_AMMOS[(int)selected_ammo].name))
			{
				for (const auto& special_ammo : SPECIAL_AMMOS)
				{
					if (ImGui::Selectable(special_ammo.name, special_ammo.type == selected_ammo))
					{
						g.weapons.ammo_special.type = special_ammo.type;
					}

					if (special_ammo.type == selected_ammo)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			if (ImGui::BeginCombo("BULLET_IMPACT"_T.data(), BULLET_IMPACTS[selected_explosion]))
			{
				for (const auto& [type, name] : BULLET_IMPACTS)
				{
					if (ImGui::Selectable(name, type == selected_explosion))
					{
						g.weapons.ammo_special.explosion_tag = type;
					}

					if (type == selected_explosion)
					{
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}
		});

		components::command_checkbox<"enablemk1variants">();

		ImGui::EndGroup();

		ImGui::SeparatorText("DEBUG_TAB_MISC"_T.data());

		components::command_checkbox<"norecoil">();
		ImGui::SameLine();
		components::command_checkbox<"nospread">();
		ImGui::SameLine();
		components::command_checkbox<"nosway">();

		components::button("GET_ALL_WEAPONS"_T, [] {
			for (const auto& [_, weapon] : g_gta_data_service.weapons())
			{
				WEAPON::GIVE_DELAYED_WEAPON_TO_PED(self::ped, weapon.m_hash, 9999, false);
			}
			WEAPON::GIVE_DELAYED_WEAPON_TO_PED(self::ped, "GADGET_PARACHUTE"_J, 0, true);
		});
		ImGui::SameLine();
		components::button("REMOVE_ALL_WEAPONS"_T, [] {
			for (const auto& [_, weapon] : g_gta_data_service.weapons())
			{
				if (weapon.m_hash != "WEAPON_UNARMED"_J)
					WEAPON::REMOVE_WEAPON_FROM_PED(self::ped, weapon.m_hash);
			}
		});
		components::button("REMOVE_CUR_WEAPON"_T, [] {
			Hash weapon_hash;
			WEAPON::GET_CURRENT_PED_WEAPON(self::ped, &weapon_hash, 1);
			if (weapon_hash != "WEAPON_UNARMED"_J)
				WEAPON::REMOVE_WEAPON_FROM_PED(self::ped, weapon_hash);
		});
		ImGui::SameLine();
		components::command_button<"resetweaps">();

		components::command_checkbox<"incrdamage">();
		ImGui::InputFloat("VIEW_WEAPON_DAMAGE"_T.data(), &g.weapons.increased_damage, .1, 10, "%.1f");

		components::command_checkbox<"flyingaxe">();

		components::command_checkbox<"modifyexplosionradius">();
		ImGui::InputFloat("VIEW_WEAPON_EXPLOSION_RADIUS"_T.data(), &g.weapons.set_explosion_radius, .1, 200, "%.1f");

		ImGui::PushID("custom_weapon_view");
		ImGui::SeparatorText("CUSTOM_WEAPONS"_T.data());

		ImGui::Checkbox("VIEW_WEAPON_CUSTOM_GUN_ONLY_FIRES_WHEN_THE_WEAPON_IS_OUT"_T.data(), &g.self.custom_weapon_stop);
		CustomWeapon selected = g.weapons.custom_weapon;

		if (ImGui::BeginCombo("WEAPON"_T.data(),
		        g_translation_service.get_translation(custom_weapons[(int)selected].name).data()))
		{
			for (const custom_weapon& weapon : custom_weapons)
			{
				if (ImGui::Selectable(g_translation_service.get_translation(weapon.name).data(), weapon.id == selected))
				{
					g.weapons.custom_weapon = weapon.id;
				}

				if (weapon.id == selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}
		ImGui::PopID();

		switch (selected)
		{
		case CustomWeapon::GRAVITY_GUN:
			ImGui::Checkbox("VIEW_WEAPON_LAUNCH_ON_RELEASE"_T.data(), &g.weapons.gravity_gun.launch_on_release);
			break;
		case CustomWeapon::VEHICLE_GUN:
			components::input_text_with_hint("SHOOTING_MODEL"_T, "NAME_VEHICLE_MODEL"_T, g.weapons.vehicle_gun_model);
			break;
		case CustomWeapon::PAINT_GUN:
			ImGui::Checkbox("RAINBOW_PAINT"_T.data(), &g.weapons.paintgun.rainbow);
			ImGui::SliderFloat("VIEW_WEAPON_RAINBOW_SPEED"_T.data(), &g.weapons.paintgun.speed, 0.f, 10.f);
			if (!g.weapons.paintgun.rainbow)
			{
				ImGui::ColorEdit4("VIEW_WEAPON_PAINT_GUN_COLOR"_T.data(), g.weapons.paintgun.col);
			}
		}

		ImGui::SeparatorText("VIEW_WEAPON_AIM_ASSISTANCE"_T.data());
		components::command_checkbox<"triggerbot">();
		ImGui::SameLine();
		components::command_checkbox<"aimbot">();

		if (g.weapons.aimbot.enable || g.weapons.triggerbot)
		{
			components::command_checkbox<"aimonlyatplayer">();
			ImGui::SameLine();
			ImGui::Checkbox("TRUST_FRIENDS"_T.data(), &g.weapons.aimbot.exclude_friends);
			ImGui::SameLine();
			components::command_checkbox<"aimonlyatenemy">();
			ImGui::SameLine();
			components::command_checkbox<"aimonlyatthreats">();

			ImGui::BeginGroup();
			ImGui::CheckboxFlags("PLAYERS"_T.data(), &g.weapons.aimbot.only_on_ped_type, (int64_t)ePedTypeFlag::PED_TYPE_NETWORK_PLAYER);
			ImGui::CheckboxFlags("PED_TYPE_CIVMALE"_T.data(), &g.weapons.aimbot.only_on_ped_type, (int64_t)ePedTypeFlag::PED_TYPE_CIVMALE);
			ImGui::CheckboxFlags("PED_TYPE_CIVFEMALE"_T.data(), &g.weapons.aimbot.only_on_ped_type, (int64_t)ePedTypeFlag::PED_TYPE_CIVFEMALE);
			ImGui::EndGroup();

			ImGui::SameLine();

			ImGui::BeginGroup();
			ImGui::CheckboxFlags("PED_TYPE_DEALER"_T.data(), &g.weapons.aimbot.only_on_ped_type, (int64_t)ePedTypeFlag::PED_TYPE_DEALER);
			ImGui::CheckboxFlags("PED_TYPE_PROSTITUTE"_T.data(), &g.weapons.aimbot.only_on_ped_type, (int64_t)ePedTypeFlag::PED_TYPE_PROSTITUTE);
			ImGui::CheckboxFlags("PED_TYPE_BUM"_T.data(), &g.weapons.aimbot.only_on_ped_type, (int64_t)ePedTypeFlag::PED_TYPE_BUM);
			ImGui::EndGroup();

			ImGui::SameLine();

			ImGui::BeginGroup();
			ImGui::CheckboxFlags("PED_TYPE_MEDIC"_T.data(), &g.weapons.aimbot.only_on_ped_type, (int64_t)ePedTypeFlag::PED_TYPE_MEDIC);
			ImGui::CheckboxFlags("PED_TYPE_FIREMAN"_T.data(), &g.weapons.aimbot.only_on_ped_type, (int64_t)ePedTypeFlag::PED_TYPE_FIREMAN);
			ImGui::CheckboxFlags("PED_TYPE_ARMY"_T.data(), &g.weapons.aimbot.only_on_ped_type, (int64_t)ePedTypeFlag::PED_TYPE_ARMY);
			ImGui::EndGroup();

			ImGui::SameLine();

			ImGui::BeginGroup();
			ImGui::CheckboxFlags("GUI_TAB_MISSIONS"_T.data(), &g.weapons.aimbot.only_on_ped_type, (int64_t)ePedTypeFlag::PED_TYPE_MISSION);
			ImGui::CheckboxFlags("PED_TYPE_ANIMAL"_T.data(), &g.weapons.aimbot.only_on_ped_type, (int64_t)ePedTypeFlag::PED_TYPE_ANIMAL);
			ImGui::CheckboxFlags("PED_TYPE_SPECIAL"_T.data(), &g.weapons.aimbot.only_on_ped_type, (int64_t)ePedTypeFlag::PED_TYPE_SPECIAL);
			ImGui::EndGroup();

			ImGui::SameLine();

			ImGui::BeginGroup();
			ImGui::CheckboxFlags("POLICE"_T.data(), &g.weapons.aimbot.only_on_ped_type, (int64_t)ePedTypeFlag::PED_TYPE_COP);
			ImGui::CheckboxFlags("PED_TYPE_SWAT"_T.data(), &g.weapons.aimbot.only_on_ped_type, (int64_t)ePedTypeFlag::PED_TYPE_SWAT);
			ImGui::EndGroup();

			ImGui::PushItemWidth(350);
			ImGui::SliderFloat("VIEW_WEAPON_AIM_FOV"_T.data(), &g.weapons.aimbot.fov, 1.f, 360.f, "%.0f");
			if (g.weapons.aimbot.use_weapon_range)
				ImGui::BeginDisabled();
			ImGui::SliderFloat("VIEW_SELF_CUSTOM_TELEPORT_DISTANCE"_T.data(), &g.weapons.aimbot.distance, 1.f, 1000.f, "%.0f");
			ImGui::SameLine();
			if (g.weapons.aimbot.use_weapon_range)
				ImGui::EndDisabled();
			ImGui::Checkbox("BACKEND_LOOPED_WEAPONS_USE_MAX_RANGE"_T.data(), &g.weapons.aimbot.use_weapon_range);
			ImGui::PopItemWidth();
		}
	}
}
