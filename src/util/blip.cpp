#pragma once
#include "blip.hpp"

namespace big::blip
{
	bool get_blip_location(Vector3& location, int sprite, int color)
	{
		Blip blip;
		for (blip = HUD::GET_CLOSEST_BLIP_INFO_ID(sprite); HUD::DOES_BLIP_EXIST(blip) && color != -1 && HUD::GET_BLIP_COLOUR(blip) != color; blip = HUD::GET_NEXT_BLIP_INFO_ID(sprite))
			;

		if (!HUD::DOES_BLIP_EXIST(blip) || (color != -1 && HUD::GET_BLIP_COLOUR(blip) != color))
			return false;

		location = HUD::GET_BLIP_COORDS(blip);

		return true;
	}

	bool get_objective_location_iteration(Vector3& location, const std::unordered_set<BlipIcons> sprites, const std::unordered_set<BlipColors> blip_colors = {})
	{
		for (int i = 0; i < 1500; i++)
		{
			auto blip = g_pointers->m_gta.m_blip_list->m_Blips[i].m_pBlip;
			if (blip != nullptr
			    && ((sprites.contains((BlipIcons)blip->m_icon)
			            && (blip_colors.empty() || blip_colors.contains((BlipColors)blip->m_color)))
			        || (blip->m_display_bits & BlipIsGPSRoute)))
			{
				location.x = blip->m_position.x;
				location.y = blip->m_position.y;
				location.z = blip->m_position.z;
				return true;
			}
		}
		location = {};
		return false;
	}

	bool get_blip_location_from_offset(Vector3& location, int sprite)
	{
		Blip blip = HUD::GET_CLOSEST_BLIP_INFO_ID(sprite);
		if (HUD::DOES_BLIP_EXIST(blip))
		{
			location = HUD::GET_BLIP_COORDS(blip);
			return true;
		}
		return false;
	}

	bool get_objective_location(Vector3& location)
	{
		if (get_objective_location_iteration(location, {BlipIcons::RADAR_LEVEL}, {BlipColors::YellowMission, BlipColors::YellowMission2, BlipColors::Mission}))
			return true;
		if (get_objective_location_iteration(location, {BlipIcons::RADAR_RACEFLAG}, {BlipColors::None}))
			return true;
		if (get_objective_location_iteration(location, {BlipIcons::RADAR_LEVEL}, {BlipColors::Green, BlipColors::Blue}))
			return true;

		static const auto blip_icons = {BlipIcons::RADAR_OBJECTIVE_BLUE, BlipIcons::RADAR_OBJECTIVE_GREEN, BlipIcons::RADAR_OBJECTIVE_RED, BlipIcons::RADAR_OBJECTIVE_YELLOW, BlipIcons::RADAR_CRATEDROP, BlipIcons::RADAR_TARGET_A, BlipIcons::RADAR_TARGET_B, BlipIcons::RADAR_TARGET_C, BlipIcons::RADAR_TARGET_D, BlipIcons::RADAR_TARGET_E, BlipIcons::RADAR_TARGET_F, BlipIcons::RADAR_TARGET_G, BlipIcons::RADAR_TARGET_H, BlipIcons::RADAR_SM_CARGO, BlipIcons::RADAR_BAT_CARGO};
		for (const auto icon : blip_icons)
		{
			if (get_blip_location_from_offset(location, (int)icon))
				return true;
		}

		return false;
	}

	rage::CBlip* get_selected_blip()
	{
		for (int i = 0; i < 1500; i++)
		{
			auto blip = g_pointers->m_gta.m_blip_list->m_Blips[i].m_pBlip;
			if (blip && (blip->m_display_bits & BlipIsSelected))
			{
				return blip;
			}
		}
		return nullptr;
	}

	rage::CBlip* get_blip_from_blip_id(Blip blip_id)
	{
		for (int i = 0; i < 1500; i++)
		{
			auto blip = g_pointers->m_gta.m_blip_list->m_Blips[i].m_pBlip;
			if (blip && (blip->m_blip_array_index == blip_id))
			{
				return blip;
			}
		}
		return nullptr;
	}
}