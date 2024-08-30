#pragma once
#include "services/players/player.hpp"

namespace big
{
	class sync_list
	{
	public:
		std::array<std::string, 100> list;
		int size = 0;

		inline void add_sync_data_to_list(player_ptr sender, std::string details)
		{
			if (size == 100)
			{
				// Copy the last 50 elements to the first 50 positions
				std::copy(list.begin() + 50, list.end(), list.begin());
				size = 50;
			}

			// Add the new element after the first 50 elements
			auto now       = std::chrono::system_clock::now();
			auto localTime = std::chrono::zoned_time{std::chrono::current_zone(), now};

			list[size] = std::format("[{:%H:%M:%S}] [{}] {}",localTime.get_local_time(), sender->get_name(), details);

			++size;
		}
	};

	struct g_misc_data_t
	{
		sync_list ped_sync_list;
		sync_list vehicle_sync_list;
		sync_list object_sync_list;
		sync_list pickup_sync_list;
		sync_list explosion_sync_list;
		sync_list ptfx_sync_list;
	};

	inline g_misc_data_t g_misc_data{};
}