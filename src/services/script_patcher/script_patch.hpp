#pragma once
#include "memory/pattern.hpp"

namespace big
{
	struct script_data;

	class script_patch
	{
		rage::joaat_t m_script;
		const memory::pattern m_pattern;
		std::string m_name;
		int32_t m_offset;
		std::vector<uint8_t> m_patch;
		std::vector<uint8_t> m_original;
		bool* m_bool;
		int32_t m_ip;

		static uint8_t* get_code_address(script_data* data, uint32_t index);
		static const std::optional<uint32_t> get_code_location_by_pattern(script_data* data, const memory::pattern& pattern);

	public:
		void enable(script_data* data);
		void disable(script_data* data);

		inline rage::joaat_t get_script()
		{
			return m_script;
		}

		inline std::string get_name()
		{
			return m_name;
		}

		script_patch(rage::joaat_t script, std::string name, const memory::pattern pattern, int32_t offset, std::vector<uint8_t> patch, bool* enable_bool);
		void update(script_data* data);
	};
}