#pragma once
#include "gta/enums.hpp"
#include "natives.hpp"
#include "pointers.hpp"
#include "script.hpp"
#include "ui/CBlipList.hpp"

namespace big::blip
{
	bool get_blip_location(Vector3& location, int sprite, int color = -1);

	bool get_objective_location_iteration(Vector3& location, const std::unordered_set<BlipIcons> sprites, const std::unordered_set<BlipColors> blip_colors);

	bool get_blip_location_from_offset(Vector3& location, int sprite);

	bool get_objective_location(Vector3& location);

	rage::CBlip* get_selected_blip();

	rage::CBlip* get_blip_from_blip_id(Blip);
}