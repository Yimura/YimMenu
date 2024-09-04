#include "services/gta_data/gta_data_service.hpp"
#include "views/view.hpp"

namespace big
{
	void view::gta_cache()
	{
		auto ped_count      = g_gta_data_service.peds().size();
		auto veh_count      = g_gta_data_service.vehicles().size();
		auto wep_count      = g_gta_data_service.weapons().size();
		auto wep_comp_count = g_gta_data_service.weapon_components().size();

		ImGui::Text(std::format("{}: {}\n{}: {}\n{}: {}\n{}: {}", "VIEW_GTA_CACHE_PEDS_CACHED"_T, ped_count, "VIEW_GTA_CACHE_VEHICLES_CACHED"_T, veh_count, "VIEW_GTA_CACHE_WEAPONS_CACHED"_T, wep_count, "VIEW_GTA_CACHE_WEAPON_COMPONENTS_CACHED"_T, wep_comp_count)
		                .c_str());

		if (!*g_pointers->m_gta.m_is_session_started && components::button("VIEW_GTA_CACHE_REBUILD_CACHE"_T))
		{
			g_gta_data_service.set_state(eGtaDataUpdateState::NEEDS_UPDATE);
			g_gta_data_service.update_now();
		}
	}
}
