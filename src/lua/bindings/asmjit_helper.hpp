#pragma once
#include <asmjit/asmjit.h>

namespace lua::memory
{
	// does a given type fit in a general purpose register (i.e. is it integer type)
	bool is_general_register(const asmjit::TypeId type_id);

	// float, double, simd128
	bool is_XMM_register(const asmjit::TypeId type_id);

	asmjit::CallConvId get_call_convention(const std::string& conv);

	asmjit::TypeId get_type_id(const std::string& type);
}