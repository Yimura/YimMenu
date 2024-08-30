#pragma once
#include "asmjit_helper.hpp"
#include "hooking/detour_hook.hpp"
#include "lua/bindings/type_info_t.hpp"

#include <asmjit/asmjit.h>

namespace lua::memory
{
	class runtime_func_t
	{
		std::vector<uint8_t> m_jit_function_buffer;
		asmjit::x86::Mem m_args_stack;

		std::unique_ptr<big::detour_hook> m_detour;

		uintptr_t m_target_func_ptr{};

	public:
		type_info_t m_return_type;
		std::vector<type_info_t> m_param_types;

		struct parameters_t
		{
			template<typename T>
			void set(const uint8_t idx, const T val) const
			{
				*(T*)get_arg_ptr(idx) = val;
			}

			template<typename T>
			T get(const uint8_t idx) const
			{
				return *(T*)get_arg_ptr(idx);
			}

			// asm depends on this specific type
			// we the runtime_func allocates stack space that is set to point here (check asmjit::compiler.newStack calls)
			volatile uintptr_t m_arguments;

			// must be char* for aliasing rules to work when reading back out
			char* get_arg_ptr(const uint8_t idx) const;
		};

		class return_value_t
		{
			uintptr_t m_return_value;

		public:
			unsigned char* get() const;
		};

		typedef bool (*user_pre_callback_t)(const parameters_t* params, const uint8_t parameters_count, return_value_t* return_value, const uintptr_t target_func_ptr);
		typedef void (*user_post_callback_t)(const parameters_t* params, const uint8_t parameters_count, return_value_t* return_value, const uintptr_t target_func_ptr);

		runtime_func_t();

		~runtime_func_t();

		uintptr_t get_target_func_ptr() const
		{
			return m_target_func_ptr;
		}

		void debug_print_args(const asmjit::FuncSignature& sig);

		// Construct a callback given the raw signature at runtime. 'Callback' param is the C stub to transfer to,
		// where parameters can be modified through a structure which is written back to the parameter slots depending
		// on calling convention.
		uintptr_t make_jit_func(const asmjit::FuncSignature& sig, const asmjit::Arch arch, const user_pre_callback_t pre_callback, const user_post_callback_t post_callback, const uintptr_t target_func_ptr);

		// Construct a callback given the typedef as a string. Types are any valid C/C++ data type (basic types), and pointers to
		// anything are just a uintptr_t. Calling convention is defaulted to whatever is typical for the compiler you use, you can override with
		// stdcall, fastcall, or cdecl (cdecl is default on x86). On x64 those map to the same thing.
		uintptr_t make_jit_func(const std::string& return_type, const std::vector<std::string>& param_types, const asmjit::Arch arch, const user_pre_callback_t pre_callback, const user_post_callback_t post_callback, const uintptr_t target_func_ptr, std::string call_convention = "");

		void create_and_enable_hook(const std::string& hook_name, uintptr_t target_func_ptr, uintptr_t jitted_func_ptr);
	};
}