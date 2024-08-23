#include "script_function.hpp"

namespace big
{
	script_function::script_function(const std::string& name, const rage::joaat_t script, const std::string& pattern) :
	    m_name(name),
	    m_script(script),
	    m_pattern(pattern),
	    m_ip(0)
	{
	}

	uint32_t script_function::get_ip(rage::scrProgram* program)
	{
		if (m_ip != 0)
			return m_ip;

		if (auto location = scripts::get_code_location_by_pattern(program, m_pattern))
		{
			m_ip = *location;
			LOG(VERBOSE) << "Found pattern " << m_name << " at " << HEX_TO_UPPER(m_ip) << " in script " << program->m_name;
		}
		else
		{
			LOG(FATAL) << "Failed to find pattern " << m_name << " in script " << program->m_name;
		}

		return m_ip;
	}
}