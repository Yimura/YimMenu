#include "backend/player_command.hpp"
#include "natives.hpp"
#include "pointers.hpp"
#include "services/gta_data/gta_data_service.hpp"
#include "util/string_operations.hpp"
#include "util/teleport.hpp"
#include "util/vehicle.hpp"

namespace big
{
	enum class eKamikazeType
	{
		REGULAR,
		SELF,
		TOPDOWN
	};

	const std::map<eKamikazeType, std::string> kamikaze_types = {{eKamikazeType::REGULAR, "Regular"}, {eKamikazeType::SELF, "Self"}, {eKamikazeType::TOPDOWN, "Topdown"}};

	class kamikaze : player_command
	{
		using player_command::player_command;

		virtual std::optional<std::vector<std::string>> get_argument_suggestions(int arg) override
		{
			std::vector<std::string> suggestions;
			if (arg == 1)
			{
				for (auto& [_, player] : g_player_service->players())
				{
					suggestions.push_back(player->get_name());
				}
				return suggestions;
			}
			else if (arg == 2)
			{
				for (auto& [type, name] : kamikaze_types)
				{
					suggestions.push_back(name);
				}
				return suggestions;
			}
			else if (arg == 3)
			{
				for (auto& item : g_gta_data_service.vehicles())
				{
					suggestions.push_back(item.second.m_name);
				}
				return suggestions;
			}

			return std::nullopt;
		}

		virtual std::optional<command_arguments> parse_args(const std::vector<std::string>& args, const std::shared_ptr<command_context> ctx) override
		{
			command_arguments result(3);

			std::string self_name       = g_player_service->get_self()->get_name();
			std::string args_name_lower = args[0];
			string::operations::to_lower(self_name);
			string::operations::to_lower(args_name_lower);

			player_ptr target{};
			if (args[0] == "me" || args[0] == "self" || self_name.find(args_name_lower) != std::string::npos)
				target = g_player_service->get_self();
			else
				target = g_player_service->get_by_name_closest(args[0]);

			eKamikazeType type = (eKamikazeType)-1;

			for (auto it = kamikaze_types.begin(); it != kamikaze_types.end(); ++it)
				if (it->second == args[1])
					type = it->first;

			if (!target)
			{
				g_notification_service.push_error("Teleport", "Invalid player name(s).");
				return std::nullopt;
			}

			if (type == (eKamikazeType)-1)
			{
				g_notification_service.push_error("Teleport", "Invalid type.");
				return std::nullopt;
			}

			result.push(target->id());
			result.push((int)type);


			std::string item_name_lower, args_lower;
			args_lower = args[2];
			string::operations::to_lower(args_lower);
			for (auto& item : g_gta_data_service.vehicles())
			{
				item_name_lower = item.second.m_name;
				string::operations::to_lower(item_name_lower);
				if (item_name_lower.find(args_lower) != std::string::npos)
				{
					result.push(rage::joaat(item.second.m_name));
					return result;
				}
			}

			return result;
		}

		virtual CommandAccessLevel get_access_level() override
		{
			return CommandAccessLevel::AGGRESSIVE;
		}

		float get_speed_for_plane(Entity target)
		{
			auto ent_speed = ENTITY::GET_ENTITY_SPEED(target);
			if (ent_speed < 100)
				return 100;
			else
				return ent_speed + 20;
		}

		virtual void execute(player_ptr player, const command_arguments& _args, const std::shared_ptr<command_context> ctx) override
		{
			player             = g_player_service->get_by_id(_args.get<uint8_t>(0));
			auto type          = (eKamikazeType)_args.get<int>(1);
			auto vehicle_model = _args.get<rage::joaat_t>(2);

			if (!player || !player->get_ped()->get_position())
				return;

			auto position   = player->get_ped()->get_position();
			auto ped_handle = g_pointers->m_gta.m_ptr_to_handle(player->get_ped());
			float height    = 20;
			float distance  = 30;

			if (type == eKamikazeType::SELF)
			{
				height   = 100;
				distance = 100;
			}
			else if (type == eKamikazeType::TOPDOWN)
			{
				height   = 100;
				distance = 0;
			}

			auto spawn_position = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(ped_handle, 0.0, -distance, height);

			Entity plane{};
			if (type == eKamikazeType::SELF && ENTITY::DOES_ENTITY_EXIST(self::veh))
			{
				plane = self::veh;
			}
			else
				plane = vehicle::spawn(vehicle_model, spawn_position, ENTITY::GET_ENTITY_HEADING(ped_handle));

			if (ENTITY::DOES_ENTITY_EXIST(plane) && entity::take_control_of(plane))
			{
				float angle = type == eKamikazeType::TOPDOWN ? 90 : 30;

				ENTITY::SET_ENTITY_COORDS_NO_OFFSET(plane, spawn_position.x, spawn_position.y, spawn_position.z, 0, 0, 0);
				auto current_rot = ENTITY::GET_ENTITY_ROTATION(plane, 0);
				ENTITY::SET_ENTITY_ROTATION(plane, current_rot.x - angle, current_rot.y, current_rot.z, 0, true);
				VEHICLE::SET_VEHICLE_FORWARD_SPEED(plane, get_speed_for_plane(ped_handle));
				VEHICLE::SET_VEHICLE_ENGINE_ON(plane, true, true, true);

				if (type != eKamikazeType::SELF)
					VEHICLE::SET_VEHICLE_OUT_OF_CONTROL(plane, false, true);

				if (type == eKamikazeType::SELF && !ENTITY::DOES_ENTITY_EXIST(self::veh))
				{
					teleport::into_vehicle(plane);
					entity::take_control_of(plane);
				}
			}
		}
	};

	kamikaze g_kamikaze("kamikaze", "KAMIKAZE", "KAMIKAZE_DESC", 2);
}
