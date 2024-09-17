#include "backend/looped_command.hpp"
#include "pointers.hpp"
#include <game_files/GameDataHash.hpp>

namespace
{
	static std::array<std::uint32_t, 16> orig_hash;

	void store_data(std::array<std::uint32_t, 16>& data)
	{
		for (int i = 0; i < 16; i++)
		{
			data[i] = (*big::g_pointers->m_gta.m_game_data_hash)->m_data[i];
		}
	}

	void load_data(const std::array<std::uint32_t, 16>& data)
	{
		for (int i = 0; i < 16; i++)
		{
			(*big::g_pointers->m_gta.m_game_data_hash)->m_data[i] = data[i];
		}
	}
}

namespace big
{
	class spoof_game_data_hash : looped_command
	{
		using looped_command::looped_command;

		virtual void on_enable() override
		{
			store_data(orig_hash);
		}

		virtual void on_tick() override
		{
			if (g.spoofing.game_data_hash_dirty)
			{
				load_data(g.spoofing.game_data_hash);
				g.spoofing.game_data_hash_dirty = false;
			}
		}

		virtual void on_disable() override
		{
			load_data(orig_hash);
		}
	};

	class copy_current_game_data_hash : command
	{
		using command::command;

		virtual void execute(const command_arguments&, const std::shared_ptr<command_context> ctx) override
		{
			if (g.spoofing.spoof_game_data_hash)
				g.spoofing.game_data_hash = orig_hash;
			else
				store_data(g.spoofing.game_data_hash);

			g.spoofing.game_data_hash_dirty = true;
		}
	};

	spoof_game_data_hash g_spoof_game_data_hash("spoofdatahash", "SPOOF_GAME_DATA_HASH", "SPOOF_GAME_DATA_HASH_DESC", g.spoofing.spoof_game_data_hash);
	copy_current_game_data_hash g_copy_current_game_data_hash("storecurrenthash", "COPY_CURRENT_GAME_DATA_HASH", "COPY_CURRENT_GAME_DATA_HASH_DESC", 0);
}
