#include "persist_car_service.hpp"

#include "base/CObject.hpp"
#include "pointers.hpp"
#include "script_function.hpp"
#include "util/misc.hpp"
#include "util/vehicle.hpp"
#include "util/pools.hpp"
#include "util/world_model.hpp"
#include "vehicle/CVehicle.hpp"

namespace big
{
	void persist_car_service::save_vehicle(Vehicle vehicle, std::string_view file_name, std::string folder_name)
	{
		if (!ENTITY::DOES_ENTITY_EXIST(vehicle) || !ENTITY::IS_ENTITY_A_VEHICLE(vehicle))
		{
			g_notification_service.push_warning("PERSIST_CAR_TITLE"_T.data(),
			    "PERSIST_CAR_INVALID_VEHICLE_SAVE_ATTEMPT"_T.data());
			return;
		}

		const auto file = check_vehicle_folder(folder_name).get_file(file_name);

		std::ofstream file_stream(file.get_path(), std::ios::out | std::ios::trunc);

		file_stream << get_full_vehicle_json(vehicle).dump(4);

		file_stream.close();
	}

	Vehicle persist_car_service::preview_vehicle(std::string_view file_name, std::string folder_name, const std::optional<Vector3>& spawn_coords)
	{
		const auto file = check_vehicle_folder(folder_name).get_file(file_name);

		std::ifstream file_stream(file.get_path());

		nlohmann::json vehicle_json;

		try
		{
			file_stream >> vehicle_json;
			file_stream.close();
		}
		catch (std::exception& e)
		{
			g_notification_service.push_warning("PERSIST_CAR_TITLE"_T.data(), "Failed to load JSON file");
			return NULL;
		}

		return spawn_vehicle_json(vehicle_json, self::ped, spawn_coords, true);
	}

	Vehicle persist_car_service::load_vehicle(std::string_view file_name, std::string folder_name, const std::optional<Vector3>& spawn_coords)
	{
		const auto file = check_vehicle_folder(folder_name).get_file(file_name);

		std::ifstream file_stream(file.get_path());

		nlohmann::json vehicle_json;

		try
		{
			file_stream >> vehicle_json;
			file_stream.close();
		}
		catch (std::exception& e)
		{
			g_notification_service.push_warning("PERSIST_CAR_TITLE"_T.data(), "Failed to load JSON file");
			return NULL;
		}

		return spawn_vehicle_full(vehicle_json, self::ped, spawn_coords);
	}

	void persist_car_service::delete_vehicle(std::string_view file_name, std::string folder_name)
	{
		const auto file = check_vehicle_folder(folder_name).get_file(file_name);

		if (file.exists())
		{
			std::filesystem::remove(file.get_path());
		}
	}

	std::vector<std::string> persist_car_service::list_files(std::string folder_name)
	{
		std::vector<std::string> file_paths;

		const auto file_path = check_vehicle_folder(folder_name);
		for (const auto& directory_entry : std::filesystem::directory_iterator(file_path.get_path()))
			if (directory_entry.path().extension() == ".json")
				file_paths.push_back(directory_entry.path().filename().generic_string());

		return file_paths;
	}

	std::vector<std::string> persist_car_service::list_sub_folders()
	{
		std::vector<std::string> folders;

		const auto file_path = check_vehicle_folder();
		for (const auto& directory_entry : std::filesystem::directory_iterator(file_path.get_path()))
			if (directory_entry.is_directory())
				folders.push_back(directory_entry.path().filename().generic_string());

		return folders;
	}



	Vehicle persist_car_service::clone_ped_car(Ped ped, Vehicle vehicle)
	{
		return spawn_vehicle_full(get_full_vehicle_json(vehicle), ped);
	}

	Vehicle persist_car_service::spawn_vehicle_full(nlohmann::json vehicle_json, Ped ped, const std::optional<Vector3>& spawn_coords)
	{
		const auto vehicle = spawn_vehicle(vehicle_json, ped, spawn_coords);

		if (!vehicle_json[tow_key].is_null())
		{
			const auto tow = spawn_vehicle(vehicle_json[tow_key], ped, spawn_coords);

			auto pos = ENTITY::GET_ENTITY_COORDS(tow, true);
			pos.x -= 10;

			ENTITY::SET_ENTITY_COORDS_NO_OFFSET(tow, pos.x, pos.y, 0.f, true, true, false);

			VEHICLE::ATTACH_VEHICLE_TO_TOW_TRUCK(vehicle, tow, true, 0.f, 0.5f, 0.f);
			VEHICLE::SET_VEHICLE_TOW_TRUCK_ARM_POSITION(vehicle, 1.f);

			const auto rotation = ENTITY::GET_ENTITY_ROTATION(tow, 2);
			ENTITY::SET_ENTITY_ROTATION(tow, 0, 0, rotation.z, 2, true);
		}
		else if (!vehicle_json[trailer_key].is_null())
		{
			const auto trailer = spawn_vehicle(vehicle_json[trailer_key], ped, spawn_coords);
			VEHICLE::ATTACH_VEHICLE_TO_TRAILER(vehicle, trailer, 1.0f);

			const auto rotation = ENTITY::GET_ENTITY_ROTATION(trailer, 2);
			ENTITY::SET_ENTITY_ROTATION(trailer, 0, 0, rotation.z, 2, true);
		}

		const auto rotation = ENTITY::GET_ENTITY_ROTATION(vehicle, 2);
		ENTITY::SET_ENTITY_ROTATION(vehicle, rotation.x, 0, rotation.z, 2, true);

		return vehicle;
	}

	Vehicle persist_car_service::spawn_vehicle(nlohmann::json vehicle_json, Ped ped, const std::optional<Vector3>& spawn_coords)
	{
		const auto vehicle = spawn_vehicle_json(vehicle_json, ped, spawn_coords);

		std::vector<nlohmann::json> model_attachments = vehicle_json[model_attachments_key];
		for (const auto& j : model_attachments)
		{
			const auto attachment = j.get<model_attachment>();
			const auto object     = world_model::spawn(attachment.model_hash);
			if (object)
			{
				ENTITY::ATTACH_ENTITY_TO_ENTITY(object,
				    vehicle,
				    0,
				    attachment.position.x,
				    attachment.position.y,
				    attachment.position.z,
				    attachment.rotation.x,
				    attachment.rotation.y,
				    attachment.rotation.z,
				    false,
				    false,
				    false,
				    false,
				    0,
				    true,
				    0);

				ENTITY::SET_ENTITY_VISIBLE(object, attachment.is_visible, 0);
				ENTITY::SET_ENTITY_COLLISION(object, attachment.has_collision, true);
			}
		}

		std::vector<nlohmann::json> vehicle_attachments = vehicle_json[vehicle_attachments_key];
		for (const auto& j : vehicle_attachments)
		{
			const auto vehicle_to_attach = spawn_vehicle_json(j[vehicle_key], ped);
			auto attachment              = j[model_attachment_key].get<big::model_attachment>();
			ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicle_to_attach,
			    vehicle,
			    0,
			    attachment.position.x,
			    attachment.position.y,
			    attachment.position.z,
			    attachment.rotation.x,
			    attachment.rotation.y,
			    attachment.rotation.z,

			    false,
			    false,
			    false,
			    false,
			    0,
			    true,
			    0);

			ENTITY::SET_ENTITY_VISIBLE(vehicle_to_attach, attachment.is_visible, 0);
			ENTITY::SET_ENTITY_COLLISION(vehicle_to_attach, attachment.has_collision, true);
			VEHICLE::SET_VEHICLE_IS_CONSIDERED_BY_PLAYER(vehicle_to_attach, false);
		}

		return vehicle;
	}

	Vehicle persist_car_service::spawn_vehicle_json(nlohmann::json vehicle_json, Ped ped, const std::optional<Vector3>& spawn_coords, bool is_preview)
	{
		const Hash vehicle_hash = vehicle_json[vehicle_model_hash_key];
		const Vector3& spawn_location = spawn_coords.has_value() ? spawn_coords.value() : vehicle::get_spawn_location(g.persist_car.spawn_inside, vehicle_hash);
		const float spawn_heading = ENTITY::GET_ENTITY_HEADING(self::ped);

		Vehicle vehicle = self::veh;
		if (is_preview || (!is_preview && ENTITY::GET_ENTITY_MODEL(vehicle) != vehicle_hash))
		{
			vehicle = big::vehicle::spawn(vehicle_hash, spawn_location, spawn_heading, !is_preview);

			if (spawn_location.x + spawn_location.y + spawn_location.z != 0)
				script::get_current()->yield(); //This is needed to wait for the engine to instantiate things like the radio station so it won't overwrite it on the next frame.
		}

		VEHICLE::SET_VEHICLE_DIRT_LEVEL(vehicle, 0.0f);
		VEHICLE::SET_VEHICLE_MOD_KIT(vehicle, 0);

		if (!vehicle_json[tire_can_burst].is_null())
			VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(vehicle, vehicle_json[tire_can_burst]);
		else
			VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(vehicle, false);

		if (!vehicle_json[drift_tires].is_null())
			VEHICLE::SET_DRIFT_TYRES(vehicle, vehicle_json[drift_tires]);

		VEHICLE::SET_VEHICLE_COLOURS(vehicle, vehicle_json[primary_color_key], vehicle_json[secondary_color_key]);

		if (!vehicle_json[custom_primary_color_key].is_null())
		{
			std::vector<int> primary_custom_color = vehicle_json[custom_primary_color_key];
			VEHICLE::SET_VEHICLE_CUSTOM_PRIMARY_COLOUR(vehicle, primary_custom_color[0], primary_custom_color[1], primary_custom_color[2]);
		}

		if (!vehicle_json[is_visible_key].is_null())
		{
			bool is_visible = vehicle_json[is_visible_key];
			ENTITY::SET_ENTITY_VISIBLE(vehicle, is_visible, 0);
		}

		if (!vehicle_json[has_collision_key].is_null())
		{
			bool has_collision = vehicle_json[has_collision_key];
			ENTITY::SET_ENTITY_COLLISION(vehicle, has_collision, true);
		}

		if (!vehicle_json[custom_secondary_color_key].is_null())
		{
			std::vector<int> secondary_custom_color = vehicle_json[custom_secondary_color_key];
			VEHICLE::SET_VEHICLE_CUSTOM_SECONDARY_COLOUR(vehicle, secondary_custom_color[0], secondary_custom_color[1], secondary_custom_color[2]);
		}

		VEHICLE::SET_VEHICLE_WINDOW_TINT(vehicle, vehicle_json[vehicle_window_tint_key]);

		if (!vehicle_json[radio_station_key].is_null())
			AUDIO::SET_VEH_RADIO_STATION(vehicle, vehicle_json[radio_station_key].get<std::string>().c_str());

		VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(vehicle, vehicle_json[plate_text_key].get<std::string>().c_str());
		VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(vehicle, vehicle_json[plate_text_index_key]);
		VEHICLE::SET_VEHICLE_EXTRA_COLOURS(vehicle, vehicle_json[pearlescent_color_key], vehicle_json[wheel_color_key]);

		std::map<int, bool> vehicle_extras = vehicle_json[vehicle_extras_key];
		for (const auto& [extra, extra_enabled] : vehicle_extras)
		{
			VEHICLE::SET_VEHICLE_EXTRA(vehicle, extra, extra_enabled);
		}

		if (!vehicle_json[vehicle_livery_key].is_null())
		{
			VEHICLE::SET_VEHICLE_LIVERY(vehicle, vehicle_json[vehicle_livery_key]);
		}

		if (VEHICLE::IS_THIS_MODEL_A_CAR(ENTITY::GET_ENTITY_MODEL(vehicle)) || VEHICLE::IS_THIS_MODEL_A_BIKE(ENTITY::GET_ENTITY_MODEL(vehicle)))
		{
			VEHICLE::SET_VEHICLE_WHEEL_TYPE(vehicle, vehicle_json[wheel_type_key]);
			for (int i = MOD_SPOILERS; i < MOD_LIGHTBAR; i++)
			{
				const bool has_mod = !vehicle_json[mod_names[i]].is_null();
				if (has_mod)
				{
					if (i == MOD_TYRE_SMOKE)
					{
						std::vector<int> tire_smoke_color = vehicle_json[tire_smoke_color_key];
						VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(vehicle, tire_smoke_color[0], tire_smoke_color[1], tire_smoke_color[2]);
						VEHICLE::TOGGLE_VEHICLE_MOD(vehicle, MOD_TYRE_SMOKE, true);
					}
					else if (vehicle_json[mod_names[i]].is_array())
					{
						std::vector<int> mod = vehicle_json[mod_names[i]];
						VEHICLE::SET_VEHICLE_MOD(vehicle, i, mod[0], mod[1]);
					}
					else
					{
						VEHICLE::TOGGLE_VEHICLE_MOD(vehicle, i, true);
					}
				}
			}
			std::vector<bool> neon_lights = vehicle_json[neon_lights_key];
			for (int i = NEON_LEFT; i <= NEON_BACK; i++)
				VEHICLE::SET_VEHICLE_NEON_ENABLED(vehicle, i, neon_lights[i]);
			std::vector<int> neon_color = vehicle_json[neon_color_key];
			VEHICLE::SET_VEHICLE_NEON_COLOUR(vehicle, neon_color[0], neon_color[1], neon_color[2]);

			if (VEHICLE::IS_VEHICLE_A_CONVERTIBLE(vehicle, 0))
			{
				int convertable_state = vehicle_json[convertable_state_key];
				if (convertable_state == 0 || convertable_state == 3 || convertable_state == 5)
					VEHICLE::RAISE_CONVERTIBLE_ROOF(vehicle, true);
				else
					VEHICLE::LOWER_CONVERTIBLE_ROOF(vehicle, true);
			}

			VEHICLE::SET_VEHICLE_EXTRA_COLOUR_5(vehicle, vehicle_json[interior_color_key]);

			VEHICLE::SET_VEHICLE_EXTRA_COLOUR_6(vehicle, vehicle_json[dash_color_key]);

			const BOOL needs_clan_logo = vehicle_json[clan_logo_key];
			if (needs_clan_logo)
			{
				scr_functions::add_clan_logo_to_vehicle.call<bool>(&vehicle, NETWORK::NETWORK_GET_PLAYER_INDEX_FROM_PED(ped));
			}

			VEHICLE::SET_VEHICLE_XENON_LIGHT_COLOR_INDEX(vehicle, vehicle_json[headlight_color_key]);
		}

		return vehicle;
	}

	nlohmann::json persist_car_service::get_full_vehicle_json(Vehicle vehicle)
	{
		// The car needs to be rotated at (0, 0, 0) for the relative offset calculations to be accurate.
		ENTITY::SET_ENTITY_ROTATION(vehicle, 0, 0, 0, 0, true);
		script::get_current()->yield();
		ENTITY::SET_ENTITY_ROTATION(vehicle, 0, 0, 0, 0, true);

		nlohmann::json vehicle_json = get_vehicle_json(vehicle);

		vehicle_json[model_attachments_key]   = get_model_attachments(vehicle);
		vehicle_json[vehicle_attachments_key] = get_vehicle_attachents(vehicle);

		Vehicle tow = VEHICLE::GET_ENTITY_ATTACHED_TO_TOW_TRUCK(vehicle);
		if (ENTITY::DOES_ENTITY_EXIST(tow))
		{
			vehicle_json[tow_key]                          = get_vehicle_json(tow);
			vehicle_json[tow_key][model_attachments_key]   = get_model_attachments(tow, true);
			vehicle_json[tow_key][vehicle_attachments_key] = get_vehicle_attachents(tow);
		}
		if (VEHICLE::IS_VEHICLE_ATTACHED_TO_TRAILER(vehicle))
		{
			Vehicle trailer;
			VEHICLE::GET_VEHICLE_TRAILER_VEHICLE(vehicle, &trailer);

			vehicle_json[trailer_key]                          = get_vehicle_json(trailer);
			vehicle_json[trailer_key][model_attachments_key]   = get_model_attachments(trailer);
			vehicle_json[trailer_key][vehicle_attachments_key] = get_vehicle_attachents(trailer);
		}

		return vehicle_json;
	}

	model_attachment persist_car_service::get_model_attachment(Vehicle vehicle, Object object)
	{
		const auto object_location = ENTITY::GET_ENTITY_COORDS(object, 0);
		const auto location        = ENTITY::GET_OFFSET_FROM_ENTITY_GIVEN_WORLD_COORDS(vehicle,
            object_location.x,
            object_location.y,
            object_location.z);

		const auto object_rotation  = ENTITY::GET_ENTITY_ROTATION(object, 0);
		const auto vehicle_rotation = ENTITY::GET_ENTITY_ROTATION(vehicle, 0);
		bool has_collision          = ENTITY::GET_ENTITY_COLLISION_DISABLED(object);
		bool is_visible             = ENTITY::IS_ENTITY_VISIBLE(object);
		CObject* cobject            = (CObject*)g_pointers->m_gta.m_handle_to_ptr(vehicle);

		Vector3 rotation;
		rotation.x = (object_rotation.x - vehicle_rotation.x);
		rotation.y = (object_rotation.y - vehicle_rotation.y);
		rotation.z = (object_rotation.z - vehicle_rotation.z);

		model_attachment attachment = {ENTITY::GET_ENTITY_MODEL(object), location, rotation, !has_collision, is_visible};

		return attachment;
	}

	nlohmann::json persist_car_service::get_model_attachments(Vehicle vehicle, bool is_towed_vehicle)
	{
		std::vector<nlohmann::json> attached_objects;

		for (auto obj : pools::get_all_props())
		{
			const auto object = g_pointers->m_gta.m_ptr_to_handle(obj);
			if (!object)
				break;

			if (!ENTITY::IS_ENTITY_ATTACHED_TO_ENTITY(vehicle, object))
				continue;

			// Don't save tow hook.
			if (is_towed_vehicle && ENTITY::GET_ENTITY_MODEL(object) == "prop_v_hook_s"_J)
				continue;

			attached_objects.push_back(get_model_attachment(vehicle, object));
		};

		return attached_objects;
	}

	nlohmann::json persist_car_service::get_vehicle_attachents(Vehicle vehicle)
	{
		std::vector<nlohmann::json> attached_vehicles;

		Vehicle trailer;
		VEHICLE::GET_VEHICLE_TRAILER_VEHICLE(vehicle, &trailer);

		for (auto veh : pools::get_all_vehicles())
		{
			const auto object = g_pointers->m_gta.m_ptr_to_handle(veh);
			if (!object)
				break;

			if (!ENTITY::IS_ENTITY_ATTACHED_TO_ENTITY(vehicle, object))
				continue;

			if (object == VEHICLE::GET_ENTITY_ATTACHED_TO_TOW_TRUCK(vehicle) || VEHICLE::IS_VEHICLE_ATTACHED_TO_TOW_TRUCK(object, vehicle))
				continue;

			if (object == trailer || VEHICLE::IS_VEHICLE_ATTACHED_TO_TRAILER(object))
				continue;

			nlohmann::json model_attachment;
			model_attachment[vehicle_key]          = get_vehicle_json(object);
			model_attachment[model_attachment_key] = get_model_attachment(vehicle, object);

			attached_vehicles.push_back(model_attachment);
		}

		return attached_vehicles;
	}

	nlohmann::json persist_car_service::get_vehicle_json(Vehicle vehicle)
	{
		nlohmann::json vehicle_json;

		vehicle_json[vehicle_model_hash_key] = ENTITY::GET_ENTITY_MODEL(vehicle);

		int primary_color, secondary_color;
		VEHICLE::GET_VEHICLE_COLOURS(vehicle, &primary_color, &secondary_color);
		vehicle_json[primary_color_key]   = primary_color;
		vehicle_json[secondary_color_key] = secondary_color;
		if (VEHICLE::GET_IS_VEHICLE_PRIMARY_COLOUR_CUSTOM(vehicle))
		{
			int custom_primary_color[3]{};
			VEHICLE::GET_VEHICLE_CUSTOM_PRIMARY_COLOUR(vehicle, &custom_primary_color[0], &custom_primary_color[1], &custom_primary_color[2]);
			vehicle_json[custom_primary_color_key] = custom_primary_color;
		}

		if (VEHICLE::GET_IS_VEHICLE_SECONDARY_COLOUR_CUSTOM(vehicle))
		{
			int custom_secondary_color[3]{};
			VEHICLE::GET_VEHICLE_CUSTOM_SECONDARY_COLOUR(vehicle, &custom_secondary_color[0], &custom_secondary_color[1], &custom_secondary_color[2]);
			vehicle_json[custom_secondary_color_key] = custom_secondary_color;
		}

		vehicle_json[vehicle_window_tint_key] = VEHICLE::GET_VEHICLE_WINDOW_TINT(vehicle);

		auto radio_station = AUDIO::GET_PLAYER_RADIO_STATION_NAME();

		if (!radio_station)
			radio_station = "OFF";

		vehicle_json[radio_station_key] = radio_station;

		vehicle_json[plate_text_key]       = VEHICLE::GET_VEHICLE_NUMBER_PLATE_TEXT(vehicle);
		vehicle_json[plate_text_index_key] = VEHICLE::GET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(vehicle);

		int pearlescent_color, wheel_color;
		VEHICLE::GET_VEHICLE_EXTRA_COLOURS(vehicle, &pearlescent_color, &wheel_color);

		vehicle_json[pearlescent_color_key] = pearlescent_color;
		bool has_collision                  = ENTITY::GET_ENTITY_COLLISION_DISABLED(vehicle);
		bool is_visible                     = ENTITY::IS_ENTITY_VISIBLE(vehicle);
		CVehicle* cvehicle                  = (CVehicle*)g_pointers->m_gta.m_handle_to_ptr(vehicle);
		vehicle_json[has_collision_key]     = !has_collision;
		vehicle_json[is_visible_key]        = is_visible;
		vehicle_json[wheel_color_key]       = wheel_color;
		vehicle_json[tire_can_burst]        = VEHICLE::GET_VEHICLE_TYRES_CAN_BURST(vehicle);
		vehicle_json[drift_tires]           = VEHICLE::GET_DRIFT_TYRES_SET(vehicle);

		std::map<int, bool> vehicle_extras;
		for (int extra_iterator = 0; extra_iterator <= 14; extra_iterator++)
		{
			if (VEHICLE::DOES_EXTRA_EXIST(vehicle, extra_iterator))
			{
				vehicle_extras[extra_iterator] = !VEHICLE::IS_VEHICLE_EXTRA_TURNED_ON(vehicle, extra_iterator);
			}
		}

		vehicle_json[vehicle_extras_key] = vehicle_extras;

		if ((VEHICLE::GET_VEHICLE_LIVERY_COUNT(vehicle) > 1) && VEHICLE::GET_VEHICLE_LIVERY(vehicle) >= 0)
		{
			vehicle_json[vehicle_livery_key] = VEHICLE::GET_VEHICLE_LIVERY(vehicle);
		}

		if (VEHICLE::IS_THIS_MODEL_A_CAR(ENTITY::GET_ENTITY_MODEL(vehicle)) || VEHICLE::IS_THIS_MODEL_A_BIKE(ENTITY::GET_ENTITY_MODEL(vehicle)))
		{
			vehicle_json[wheel_type_key] = VEHICLE::GET_VEHICLE_WHEEL_TYPE(vehicle);

			for (int i = MOD_SPOILERS; i <= MOD_LIVERY; i++)
			{
				const auto is_mod_on = VEHICLE::IS_TOGGLE_MOD_ON(vehicle, i);
				if (is_mod_on)
				{
					if (i == MOD_TYRE_SMOKE)
					{
						int tire_smoke_color[3]{};
						VEHICLE::GET_VEHICLE_TYRE_SMOKE_COLOR(vehicle, &tire_smoke_color[0], &tire_smoke_color[1], &tire_smoke_color[2]);
						vehicle_json[tire_smoke_color_key] = tire_smoke_color;
					}
					vehicle_json[mod_names[i]] = "TOGGLE";
				}

				if (VEHICLE::GET_VEHICLE_MOD(vehicle, i) != -1)
				{
					int vehicle_mod[2] = {VEHICLE::GET_VEHICLE_MOD(vehicle, i), VEHICLE::GET_VEHICLE_MOD_VARIATION(vehicle, i)};
					vehicle_json[mod_names[i]] = vehicle_mod;
				}
			}

			bool neon_lights[4]{};
			for (int i = NEON_LEFT; i <= NEON_BACK; i++)
				neon_lights[i] = VEHICLE::GET_VEHICLE_NEON_ENABLED(vehicle, i);

			int neon_color[3]{};
			VEHICLE::GET_VEHICLE_NEON_COLOUR(vehicle, &neon_color[0], &neon_color[1], &neon_color[2]);
			vehicle_json[neon_color_key]  = neon_color;
			vehicle_json[neon_lights_key] = neon_lights;

			if (VEHICLE::IS_VEHICLE_A_CONVERTIBLE(vehicle, 0))
				vehicle_json[convertable_state_key] = VEHICLE::GET_CONVERTIBLE_ROOF_STATE(vehicle);

			int interior_color, dashboard_color;
			VEHICLE::GET_VEHICLE_EXTRA_COLOUR_5(vehicle, &interior_color);
			VEHICLE::GET_VEHICLE_EXTRA_COLOUR_6(vehicle, &dashboard_color);
			vehicle_json[interior_color_key] = interior_color;
			vehicle_json[dash_color_key]     = dashboard_color;

			vehicle_json[clan_logo_key]       = GRAPHICS::DOES_VEHICLE_HAVE_CREW_EMBLEM(vehicle, 0);
			vehicle_json[headlight_color_key] = VEHICLE::GET_VEHICLE_XENON_LIGHT_COLOR_INDEX(vehicle);
		}

		return vehicle_json;
	}

	big::folder persist_car_service::check_vehicle_folder(std::string folder_name)
	{
		return g_file_manager.get_project_folder("./saved_json_vehicles/" + folder_name);
	}
}
