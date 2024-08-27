#include "type_info_t.hpp"

namespace lua::memory
{
	type_info_t get_type_info_from_string(const std::string& s)
	{
		if ((s.contains("const") && s.contains("char") && s.contains("*")) || s.contains("string"))
		{
			return type_info_t::string_;
		}
		else if (s.contains("bool"))
		{
			return type_info_t::boolean_;
		}
		else if (s.contains("ptr") || s.contains("pointer") || s.contains("*"))
		{
			// passing lua::memory::pointer
			return type_info_t::ptr_;
		}
		else if (s.contains("float"))
		{
			return type_info_t::float_;
		}
		else if (s.contains("double"))
		{
			return type_info_t::double_;
		}
		else if (s.contains("vector3"))
		{
			return type_info_t::vector3_;
		}
		else
		{
			return type_info_t::integer_;
		}
	}
}