#pragma once

namespace lua::memory
{
	enum type_info_t
	{
		none_,
		boolean_,
		string_,
		integer_,
		ptr_,
		float_,
		double_,
		vector3_
	};

	type_info_t get_type_info_from_string(const std::string& s);
}