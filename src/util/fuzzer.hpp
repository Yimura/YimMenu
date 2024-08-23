#pragma once
#include "pointers.hpp"
#include "sync_trees.hpp"
#include "util/math.hpp"
#include "util/model_info.hpp"
#include "util/pools.hpp"

#include <entities/CDynamicEntity.hpp>

namespace big::fuzzer
{
	inline bool is_fuzzer_enabled()
	{
		return g.debug.fuzzer.active && g.debug.fuzzer.enabled && g.debug.fuzzer.thread_id == GetCurrentThreadId();
	}

	inline bool is_model_hash(rage::joaat_t hash)
	{
		return model_info::does_model_exist(hash);
	}

	inline bool is_object_model(rage::joaat_t hash)
	{
		return model_info::is_model_of_type(hash, eModelType::Object, eModelType::Time, eModelType::Weapon, eModelType::Destructable);
	}

	inline rage::joaat_t get_mismatched_model(rage::joaat_t original)
	{
		// TODO: trailer models?
		std::vector<rage::joaat_t> models = {
		    "player_zero"_J,           // ped
		    "kosatka"_J,               // submarine
		    "akula"_J,                 // heli
		    "alkonost"_J,              // plane
		    "jetmax"_J,                // boat
		    "faggio"_J,                // bike
		    "zentorno"_J,              // automobile
		    "apa_heist_apart2_door"_J, // object, pickup, pickup placement, door (?)
		    "freight"_J,               // train
		};

		auto info      = model_info::get_model(original);
		auto veh_model = model_info::get_vehicle_model(original);

		if (is_object_model(original))
			std::erase(models, "apa_heist_apart2_door"_J);
		else if (info && info->get_model_type() == eModelType::Vehicle)
		{
			if (veh_model->m_vehicle_type == eVehicleType::VEHICLE_TYPE_BIKE)
				std::erase(models, "faggio"_J);
			else if (veh_model->m_vehicle_type == eVehicleType::VEHICLE_TYPE_SUBMARINE)
				std::erase(models, "kosatka"_J);
			else if (veh_model->m_vehicle_type == eVehicleType::VEHICLE_TYPE_BOAT)
				std::erase(models, "jetmax"_J);
			else if (veh_model->m_vehicle_type == eVehicleType::VEHICLE_TYPE_HELI)
				std::erase(models, "akula"_J);
			else if (veh_model->m_vehicle_type == eVehicleType::VEHICLE_TYPE_PLANE)
				std::erase(models, "alkonost"_J);
			else if (veh_model->m_vehicle_type == eVehicleType::VEHICLE_TYPE_TRAIN)
				std::erase(models, "freight"_J);
			else
				std::erase(models, "zentorno"_J);
		}
		else if (info && (info->get_model_type() == eModelType::Ped))
			std::erase(models, "player_zero"_J);

		return models[math::rand(models.size())];
	}

	inline rage::joaat_t get_crash_model(rage::joaat_t original)
	{
		auto info = model_info::get_model(original);

		if (is_object_model(original))
			return "urbanweeds01"_J;
		else if (info && (info->get_model_type() == eModelType::Ped))
			return "slod_human"_J;
		else if (info && info->get_model_type() == eModelType::Vehicle)
			return "arbitergt"_J;
		else
			return math::rand(2) ? "urbanweeds01"_J : "slod_human"_J;
	}

	inline std::int16_t get_first_ped_id()
	{
		for (auto ped : pools::get_all_peds())
			if (auto net = static_cast<rage::CDynamicEntity*>(ped)->m_net_object; net && ped != g_local_player && (net->m_owner_id == -1 || net->m_owner_id == self::id))
				return net->m_object_id;

		LOG(INFO) << "nope";
		return -1;
	}

	inline std::int16_t get_first_veh_id()
	{
		if (g_local_player && g_local_player->m_vehicle && g_local_player->m_vehicle->m_net_object)
			return g_local_player->m_vehicle->m_net_object->m_object_id;

		for (auto veh : pools::get_all_vehicles())
			if (auto net = static_cast<rage::CDynamicEntity*>(veh)->m_net_object; net && (net->m_owner_id == -1 || net->m_owner_id == self::id))
				return net->m_object_id;

		return -1;
	}

	inline std::int16_t get_first_obj_id()
	{
		for (auto obj : pools::get_all_props())
			if (auto net = static_cast<rage::CDynamicEntity*>(obj)->m_net_object; net && (net->m_owner_id == -1 || net->m_owner_id == self::id))
				return net->m_object_id;

		return -1;
	}

	inline std::int16_t get_mismatched_net_id(std::int16_t original)
	{
		auto net_obj = g_pointers->m_gta.m_get_net_object(*g_pointers->m_gta.m_network_object_mgr, original, false);

		if (!net_obj)
			return -1;

		auto entity = reinterpret_cast<rage::CDynamicEntity*>(net_obj->GetGameObject());

		if (!entity)
			return -1;

		if (is_object_model(entity->m_model_info->m_hash))
			return math::rand(2) ? get_first_ped_id() : get_first_veh_id();
		else if (entity->m_model_info->get_model_type() == eModelType::Ped)
			return math::rand(2) ? get_first_obj_id() : get_first_veh_id();
		else if (entity->m_model_info->get_model_type() == eModelType::Vehicle)
			return math::rand(2) ? get_first_obj_id() : get_first_ped_id();

		return math::rand(2) ? get_first_ped_id() : get_first_veh_id();
	}

	inline rage::fvector3 get_fuzz_vector()
	{
		if (math::rand(2) == 0)
		{
			return {10800.0f, 10800.0f, 10.0f}; // host crash coords
		}
		else
		{
			if (g_local_player)
				return *g_local_player->get_position();
		}

		return {0.0f, 0.0f, 0.0f};
	}

	inline int truncate(int value, int bits)
	{
		return value & (((int)pow(2, bits)) - 1);
	}

	inline float truncate_float(float value, int length, float divisor)
	{
		float max   = (1 << length) - 1;
		int integer = truncate((int)((value / divisor) * max), length);
		return ((float)integer / max) * divisor;
	}

	inline float truncate_float_signed(float value, int length, float divisor)
	{
		float max   = (1 << (length - 1)) - 1;
		int integer = truncate((int)((value / divisor) * max), length);
		return ((float)integer / max) * divisor;
	}

	inline rage::fvector3 truncate_vector(rage::fvector3 value, int length, float divisor)
	{
		rage::fvector3 result{};
		result.x = truncate_float_signed(value.x, length, divisor);
		result.y = truncate_float_signed(value.y, length, divisor);
		result.z = truncate_float(value.z, length, divisor);
		return result;
	}

	inline rage::fvector3 truncate_vector_signed_z(rage::fvector3 value, int length, float divisor)
	{
		rage::fvector3 result{};
		result.x = truncate_float_signed(value.x, length, divisor);
		result.y = truncate_float_signed(value.y, length, divisor);
		result.z = truncate_float_signed(value.z, length, divisor);
		return result;
	}

	inline void handle_vtable_write(void* data, int size_in_bytes, const char* fuzzer_name)
	{
		auto address = reinterpret_cast<uintptr_t>(data);

		if (sync_node_finder::is_initialized())
		{
			for (auto& node : sync_node_finder::get_object_nodes(g.m_syncing_object_type))
			{
				if (node.first >= address && node.first < address + size_in_bytes)
				{
					LOG(FATAL) << "VFT OVERWRITE: " << node.second.name << " has been overwritten! size_in_bytes: " << size_in_bytes << ", fuzzer name: " << fuzzer_name;
					*(int*)0 = 0; // trigger stack trace
					break;
				}
			}
		}
	}

	// ===============================================================================

	inline std::int16_t fuzz_network_id(std::int16_t net_id)
	{
		if (!is_fuzzer_enabled())
			return net_id;

		int option = math::rand(0, 3);

		switch (option)
		{
		case 0: break; // do nothing
		case 1:        // mismatched IDs
		{
			auto _new = get_mismatched_net_id(net_id);
			if (_new != -1)
				return _new;

			break;
		}
		case 2: // set to player
		{
			if (auto net_obj = g_local_player->m_net_object)
				return net_obj->m_object_id;

			break;
		}
		case 3: // set to itself
		{
			return g.debug.fuzzer.syncing_object_id;
		}
		}

		return net_id;
	}

	inline std::uint32_t fuzz_potential_hash(std::uint32_t hash)
	{
		if (!is_fuzzer_enabled())
			return hash;

		if (hash == 0 || is_model_hash(hash))
		{
			int option = math::rand(0, 4);

			switch (option)
			{
			case 0:
			case 1:
			case 2: break;                                    // do nothing
			case 3: hash = get_mismatched_model(hash); break; // mismatched model
			case 4: hash = get_crash_model(hash); break;      // crash model
			}
		}
		else
		{
			if (math::rand(4))
				hash = math::rand(0, UINT_MAX); // not much we can do here
		}

		return hash;
	}

	inline bool fuzz_bool(bool value)
	{
		if (!is_fuzzer_enabled())
			return value;

		if (math::rand(4) == 0)
			return value; // flip bools
		else
			return true; // set to true
	}

	inline std::uint32_t fuzz_bits(std::uint32_t bits, int count)
	{
		if (!is_fuzzer_enabled())
			return bits;

		if (count == 13)
			return fuzz_network_id(bits);
		else if (count == 32)
			return fuzz_potential_hash(bits);
		else
		{
			if (math::rand(5 - (count == 1)) == 0)
			{
				if (count == 1)
				{
					return fuzz_bool(bits);
				}
				else
				{
					if (math::rand(2) == 0)
						return math::rand(((int)pow(2, count))); // random
					else
						return ((int)pow(2, count)) - 1; // max possible value
				}
			}
		}

		return bits;
	}

	inline std::int32_t fuzz_signed_bits(std::int32_t bits, int count)
	{
		auto n = fuzz_bits(bits, count - 1);

		if (math::rand(5) == 0)
			n = -n;

		return n;
	}

	inline void fuzz_data(void* data, int size)
	{
		if (!is_fuzzer_enabled())
			return;

		// well not much to do here I suppose

		for (int i = 0; i < size; i++)
			*(char*)((__int64)data + i) = math::rand(0, 255);
	}

	inline float fuzz_float(float orig, int size, float divisor, bool _signed)
	{
		auto truncate_func = _signed ? &truncate_float_signed : &truncate_float;

		if (!is_fuzzer_enabled())
			return orig;

		if (math::rand(3) == 0)
		{
			int option = math::rand(0, 2);
			switch (option)
			{
			case 0: return truncate_func(9999.9f, size, divisor);
			case 1: return truncate_func(orig + 200.0f, size, divisor);
			case 2: return truncate_func(orig - 200.0f, size, divisor);
			}
		}

		return orig;
	}

	inline rage::fvector3 fuzz_vector(rage::fvector3 orig, int size, float divisor, bool _signed_z)
	{
		auto truncate_func = _signed_z ? &truncate_vector_signed_z : &truncate_vector;

		if (math::rand(3) == 0)
		{
			return truncate_func(get_fuzz_vector(), size, divisor);
		}

		return orig;
	}
}
