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
}