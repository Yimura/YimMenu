#pragma once
#include "services/players/player.hpp"

namespace big
{
	class sync_list
	{
	public:
		std::array<std::string, 100> list;
		int size = 0;

		void add_sync_data(player_ptr sender, std::string details)
		{
			if (size == 100)
			{
				// Copy the last 50 elements to the first 50 positions
				std::copy(list.begin() + 50, list.end(), list.begin());
				// Add the new elements after the first 50 elements
				size = 50;
			}

			auto now       = std::chrono::system_clock::now();
			auto localTime = std::chrono::zoned_time{std::chrono::current_zone(), now};

			list[size] = std::format("[{:%H:%M:%S}] [{}] {}", localTime.get_local_time(), sender->get_name(), details);

			++size;
		}
	};

	struct g_sync_lists_t
	{
		sync_list peds;
		sync_list vehicles;
		sync_list objects;
		sync_list pickups;
		sync_list explosions;
		sync_list ptfxs;
	};

	inline g_sync_lists_t g_sync_lists{};
}