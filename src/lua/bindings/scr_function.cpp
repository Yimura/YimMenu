#include "scr_function.hpp"

#include "gta_util.hpp"
#include "memory.hpp"
#include "memory/pattern.hpp"
#include "pointers.hpp"
#include "util/scripts.hpp"

namespace lua::scr_function
{
	template<typename Arg>
	void push_arg(uint64_t* stack, uint32_t& stack_pointer, Arg&& value)
	{
		*reinterpret_cast<std::remove_cv_t<std::remove_reference_t<Arg>>*>(reinterpret_cast<uint64_t*>(stack) + (stack_pointer++)) = std::forward<Arg>(value);
	}

	// Lua API: Table
	// Name: scr_function
	// Table for calling GTA script functions. Needs to be called in the fiber pool. Only call the function when necessary.

	// Lua API: function
	// Table: scr_function
	// Name: call_script_function
	// Param: script_name: string: Name of the script.
	// Param: function_name: string: Name of the function. This parameter needs to be unique.
	// Param: pattern: string: Pattern to scan for within the script.
	// Param: return_type_string: string: Return type of the function. Supported types are **"int"**, **"bool"**, **"const char\*/string"**, **"ptr/pointer/*"**, **"float"**, and **"vector3"**. Anything different will be rejected.
	// Param: args_: table: Arguments to pass to the function. Supported types are the same as return types.
	// Calls a script function with the given arguments. Returns the return value as the given type.
	// **Example Usage:**
	// ```lua
	// local value = scr_function.call_script_function("freemode", "wear_sunglasses_at_night", "69 42 06 66", "bool", {
	//   { "int", 69 },
	//   { "float", 4.20 },
	//   { "int", 666 }
	// })
	// ```

	static sol::object call_script_function_by_signature(const std::string& script_name, const std::string& function_name, const std::string& pattern, const std::string& return_type_string, sol::table args_, sol::this_state state_)
	{
		std::vector<lua::memory::type_info_t> param_types;
		std::vector<sol::object> actual_args;
		static std::unordered_map<std::string, int32_t> pattern_results;
		for (const auto& [k, v_] : args_)
		{
			if (v_.is<sol::table>())
			{
				auto v = v_.as<sol::table>();
				param_types.push_back(lua::memory::get_type_info_from_string(v[1].get<const char*>()));
				actual_args.push_back(v[2].get<sol::object>());
			}
		}

		const auto return_type = lua::memory::get_type_info_from_string(return_type_string);

		auto thread  = big::gta_util::find_script_thread(rage::joaat(script_name));
		auto program = big::gta_util::find_script_program(rage::joaat(script_name));

		if (!thread || !program)
		{
			LOG(FATAL) << "Failed to find " << script_name << " for " << function_name;
			return sol::lua_nil;
		}

		int32_t instruction_pointer;

		if (pattern_results.contains(function_name))
		{
			instruction_pointer = pattern_results[function_name];
		}
		else
		{
			const ::memory::pattern pattern_scan(pattern);

			auto location = big::scripts::get_code_location_by_pattern(program, pattern_scan);

			if (!location)
			{
				LOG(FATAL) << "Failed to find pattern " << function_name << " in script " << script_name;
				return sol::lua_nil;
			}
			else
			{
				LOG(VERBOSE) << "Found pattern for " << function_name << " at " << HEX_TO_UPPER(location.value()) << " in " << script_name;
			}

			pattern_results[function_name] = instruction_pointer = location.value();
		}

		auto tls_ctx                       = rage::tlsContext::get();
		auto stack                         = (uint64_t*)thread->m_stack;
		auto og_thread                     = tls_ctx->m_script_thread;
		tls_ctx->m_script_thread           = thread;
		tls_ctx->m_is_script_thread_active = true;
		rage::scrThreadContext ctx         = thread->m_context;
		auto top_stack                     = ctx.m_stack_pointer; // This will be the top item in the stack after the args and return address are cleaned off

		for (size_t i = 0; i < param_types.size(); i++)
		{
			switch (param_types[i])
			{
			case lua::memory::type_info_t::boolean_:
			{
				const auto val = actual_args[i].as<std::optional<bool>>();
				if (val)
				{
					push_arg(stack, ctx.m_stack_pointer, *val);
				}
				break;
			}
			case lua::memory::type_info_t::string_:
			{
				const auto val = actual_args[i].as<std::optional<const char*>>();
				if (val)
				{
					push_arg(stack, ctx.m_stack_pointer, *val);
				}
				break;
			}
			case lua::memory::type_info_t::integer_:
			{
				const auto val = actual_args[i].as<std::optional<int>>();
				if (val)
				{
					push_arg(stack, ctx.m_stack_pointer, *val);
				}
				break;
			}
			case lua::memory::type_info_t::ptr_:
			{
				const auto val = actual_args[i].as<std::optional<lua::memory::pointer>>();
				if (val)
				{
					push_arg(stack, ctx.m_stack_pointer, (*val).get_address());
				}
				break;
			}
			case lua::memory::type_info_t::float_:
			{
				const auto val = actual_args[i].as<std::optional<float>>();
				if (val)
				{
					push_arg(stack, ctx.m_stack_pointer, *val);
				}
				break;
			}
			case lua::memory::type_info_t::vector3_:
			{
				const auto val = actual_args[i].as<std::optional<Vector3>>();
				if (val)
				{
					push_arg(stack, ctx.m_stack_pointer, val.value().x);
					push_arg(stack, ctx.m_stack_pointer, val.value().y);
					push_arg(stack, ctx.m_stack_pointer, val.value().z);
				}
				break;
			}
			default: break;
			}
		}

		stack[ctx.m_stack_pointer++] = 0;
		ctx.m_instruction_pointer    = instruction_pointer;
		ctx.m_state                  = rage::eThreadState::running;

		big::g_pointers->m_gta.m_script_vm(stack, big::g_pointers->m_gta.m_script_globals, program, &ctx);

		tls_ctx->m_script_thread           = og_thread;
		tls_ctx->m_is_script_thread_active = og_thread != nullptr;

		if (return_type == lua::memory::type_info_t::boolean_)
		{
			return sol::make_object(state_, (bool)*reinterpret_cast<BOOL*>(stack + top_stack));
		}
		else if (return_type == lua::memory::type_info_t::string_)
		{
			return sol::make_object(state_, *reinterpret_cast<const char**>(stack + top_stack));
		}
		else if (return_type == lua::memory::type_info_t::integer_)
		{
			return sol::make_object(state_, *reinterpret_cast<int*>(stack + top_stack));
		}
		else if (return_type == lua::memory::type_info_t::ptr_)
		{
			return sol::make_object(state_, *reinterpret_cast<uint64_t*>(stack + top_stack));
		}
		else if (return_type == lua::memory::type_info_t::float_)
		{
			return sol::make_object(state_, *reinterpret_cast<float*>(stack + top_stack));
		}
		else if (return_type == lua::memory::type_info_t::vector3_)
		{
			return sol::make_object(state_, *reinterpret_cast<Vector3*>(stack + top_stack));
		}
		else
		{
			LOG(FATAL) << "Unimplemented return type " << return_type_string;
			return sol::lua_nil;
		}
	}

	// Lua API: function
	// Table: scr_function
	// Name: call_script_function
	// Param: script_name: string: Name of the script.
	// Param: instruction_pointer: integer: Position of the function within the script.
	// Param: return_type_string: string: Return type of the function. Supported types are **"int"**, **"bool"**, **"const char\*/string"**, **"ptr/pointer/*"**, **"float"**, and **"vector3"**. Anything different will be rejected.
	// Param: args_: table: Arguments to pass to the function. Supported types are the same as return types.
	// Calls a script function directly using the function position with the given arguments. Returns the return value as the given type.
	// **Example Usage:**
	// ```lua
	// local value = scr_function.call_script_function("freemode", 0xE792, "string", {
	//   { "int", 191 }
	// })
	// ```

	static sol::object call_script_function_by_instruction_pointer(const std::string& script_name, const int instruction_pointer, const std::string& return_type_string, sol::table args_, sol::this_state state_)
	{
		std::vector<lua::memory::type_info_t> param_types;
		std::vector<sol::object> actual_args;
		for (const auto& [k, v_] : args_)
		{
			if (v_.is<sol::table>())
			{
				auto v = v_.as<sol::table>();
				param_types.push_back(lua::memory::get_type_info_from_string(v[1].get<const char*>()));
				actual_args.push_back(v[2].get<sol::object>());
			}
		}

		const auto return_type = lua::memory::get_type_info_from_string(return_type_string);

		auto thread  = big::gta_util::find_script_thread(rage::joaat(script_name));
		auto program = big::gta_util::find_script_program(rage::joaat(script_name));

		if (!thread || !program || !instruction_pointer)
		{
			LOG(FATAL) << "Failed to run " << script_name << " script function at " << HEX_TO_UPPER(instruction_pointer);
			return sol::lua_nil;
		}

		auto tls_ctx                       = rage::tlsContext::get();
		auto stack                         = (uint64_t*)thread->m_stack;
		auto og_thread                     = tls_ctx->m_script_thread;
		tls_ctx->m_script_thread           = thread;
		tls_ctx->m_is_script_thread_active = true;
		rage::scrThreadContext ctx         = thread->m_context;
		auto top_stack                     = ctx.m_stack_pointer; // This will be the top item in the stack after the args and return address are cleaned off

		for (size_t i = 0; i < param_types.size(); i++)
		{
			switch (param_types[i])
			{
			case lua::memory::type_info_t::boolean_:
			{
				const auto val = actual_args[i].as<std::optional<bool>>();
				if (val)
				{
					push_arg(stack, ctx.m_stack_pointer, *val);
				}
				break;
			}
			case lua::memory::type_info_t::string_:
			{
				const auto val = actual_args[i].as<std::optional<const char*>>();
				if (val)
				{
					push_arg(stack, ctx.m_stack_pointer, *val);
				}
				break;
			}
			case lua::memory::type_info_t::integer_:
			{
				const auto val = actual_args[i].as<std::optional<int>>();
				if (val)
				{
					push_arg(stack, ctx.m_stack_pointer, *val);
				}
				break;
			}
			case lua::memory::type_info_t::ptr_:
			{
				const auto val = actual_args[i].as<std::optional<lua::memory::pointer>>();
				if (val)
				{
					push_arg(stack, ctx.m_stack_pointer, (*val).get_address());
				}
				break;
			}
			case lua::memory::type_info_t::float_:
			{
				const auto val = actual_args[i].as<std::optional<float>>();
				if (val)
				{
					push_arg(stack, ctx.m_stack_pointer, *val);
				}
				break;
			}
			case lua::memory::type_info_t::vector3_:
			{
				const auto val = actual_args[i].as<std::optional<Vector3>>();
				if (val)
				{
					push_arg(stack, ctx.m_stack_pointer, val.value().x);
					push_arg(stack, ctx.m_stack_pointer, val.value().y);
					push_arg(stack, ctx.m_stack_pointer, val.value().z);
				}
				break;
			}
			default: break;
			}
		}

		stack[ctx.m_stack_pointer++] = 0;
		ctx.m_instruction_pointer    = instruction_pointer;
		ctx.m_state                  = rage::eThreadState::running;

		big::g_pointers->m_gta.m_script_vm(stack, big::g_pointers->m_gta.m_script_globals, program, &ctx);

		tls_ctx->m_script_thread           = og_thread;
		tls_ctx->m_is_script_thread_active = og_thread != nullptr;

		if (return_type == lua::memory::type_info_t::boolean_)
		{
			return sol::make_object(state_, (bool)*reinterpret_cast<BOOL*>(stack + top_stack));
		}
		else if (return_type == lua::memory::type_info_t::string_)
		{
			return sol::make_object(state_, *reinterpret_cast<const char**>(stack + top_stack));
		}
		else if (return_type == lua::memory::type_info_t::integer_)
		{
			return sol::make_object(state_, *reinterpret_cast<int*>(stack + top_stack));
		}
		else if (return_type == lua::memory::type_info_t::ptr_)
		{
			return sol::make_object(state_, *reinterpret_cast<uint64_t*>(stack + top_stack));
		}
		else if (return_type == lua::memory::type_info_t::float_)
		{
			return sol::make_object(state_, *reinterpret_cast<float*>(stack + top_stack));
		}
		else if (return_type == lua::memory::type_info_t::vector3_)
		{
			return sol::make_object(state_, *reinterpret_cast<Vector3*>(stack + top_stack));
		}
		else
		{
			LOG(FATAL) << "Unimplemented return type " << return_type_string;
			return sol::lua_nil;
		}
	}

	void bind(sol::state& state)
	{
		auto ns = state["scr_function"].get_or_create<sol::table>();

		ns["call_script_function"] = sol::overload(call_script_function_by_signature, call_script_function_by_instruction_pointer);
	}
}