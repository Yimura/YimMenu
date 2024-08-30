#pragma once
#include "cache_file.hpp"
#include "ped_item.hpp"
#include "vehicle_item.hpp"
#include "weapon_file.hpp"

namespace big
{
	enum class eGtaDataUpdateState
	{
		IDLE,
		NEEDS_UPDATE,
		WAITING_FOR_SINGLE_PLAYER,
		UPDATING
	};

	using ped_map     = std::map<uint32_t, ped_item>;
	using vehicle_map = std::map<uint32_t, vehicle_item>;
	using string_vec  = std::vector<std::string>;

	class gta_data_service final
	{
	public:
		gta_data_service();
		~gta_data_service() = default;

		bool init();

		bool cache_needs_update() const;
		eGtaDataUpdateState state() const;
		void set_state(eGtaDataUpdateState state);
		void update_now();

		const ped_item& ped_by_hash(uint32_t hash);
		const vehicle_item& vehicle_by_hash(uint32_t hash);
		const weapon_item& weapon_by_hash(uint32_t hash);
		const weapon_component& weapon_component_by_hash(uint32_t hash);
		const weapon_component& weapon_component_by_name(std::string name);

		string_vec& ped_types();
		string_vec& vehicle_classes();
		string_vec& weapon_types();

		ped_map& peds()
		{
			return m_peds;
		}
		vehicle_map& vehicles()
		{
			return m_vehicles;
		}
		std::map<std::string, weapon_item>& weapons()
		{
			return m_weapons_cache.weapon_map;
		}
		std::map<std::string, weapon_component>& weapon_components()
		{
			return m_weapons_cache.weapon_components;
		}

		std::string get_vehicle_full_name(vehicle_item veh);
		{
			return std::format("{} {} {}", veh.m_vehicle_class, veh.m_display_manufacturer, veh.m_display_name);
		}

	private:
		bool is_cache_up_to_date();

		void load_data();
		void load_peds();
		void load_vehicles();
		void load_weapons();

		void rebuild_cache();

	private:
		cache_file m_peds_cache;
		cache_file m_vehicles_cache;
		weapon_file m_weapons_cache;

		// std::map is free sorting algo
		ped_map m_peds;
		vehicle_map m_vehicles;

		string_vec m_ped_types;
		string_vec m_vehicle_classes;
		string_vec m_weapon_types;

		eGtaDataUpdateState m_update_state;

	private:
		static constexpr ped_item empty_ped{};
		static constexpr vehicle_item empty_vehicle{};
		static constexpr weapon_item empty_weapon{};
		static constexpr weapon_component empty_component{};
	};

	inline gta_data_service g_gta_data_service{};
}
