#include "memory.hpp"

#include "lua/lua_manager.hpp"
#include "memory/module.hpp"
#include "memory/pattern.hpp"
#include "pointers.hpp"

namespace lua::memory
{
	pointer::pointer(uint64_t address) :
	    m_address(address)
	{
	}

	pointer::pointer() :
	    m_address(0)
	{
	}

	pointer pointer::add(uint64_t offset)
	{
		return pointer(m_address + offset);
	}

	pointer pointer::sub(uint64_t offset)
	{
		return pointer(m_address - offset);
	}

	pointer pointer::rip()
	{
		return add(*(std::int32_t*)m_address).add(4);
	}

	std::string pointer::get_string()
	{
		return std::string((char*)m_address);
	}

	void pointer::set_string(const std::string& string, int max_length)
	{
		strncpy((char*)m_address, string.data(), max_length);
	}

	bool pointer::is_null()
	{
		return m_address == 0;
	}

	bool pointer::is_valid()
	{
		return !is_null();
	}

	pointer pointer::deref()
	{
		return pointer(*(uint64_t*)m_address);
	}

	uint64_t pointer::get_address() const
	{
		return m_address;
	}

	void pointer::set_address(uint64_t address)
	{
		m_address = address;
	}

	// Lua API: Table
	// Name: memory
	// Table containing helper functions related to process memory.

	// Lua API: Function
	// Table: memory
	// Name: scan_pattern
	// Param: pattern: string: byte pattern (IDA format)
	// Returns: pointer: A pointer to the found address.
	// Scans the specified memory pattern within the "GTA5.exe" module and returns a pointer to the found address.
	static pointer scan_pattern(const std::string& pattern)
	{
		return pointer(::memory::module("GTA5.exe").scan(::memory::pattern(pattern)).value().as<uint64_t>());
	}

	// Lua API: Function
	// Table: memory
	// Name: handle_to_ptr
	// Param: entity: number: script game entity handle
	// Returns: pointer: A rage::CDynamicEntity pointer to the script game entity handle
	static pointer handle_to_ptr(int entity)
	{
		auto ptr = big::g_pointers->m_gta.m_handle_to_ptr(entity);
		return pointer((uint64_t)ptr);
	}

	// Lua API: Function
	// Table: memory
	// Name: ptr_to_handle
	// Param: mem_addr: pointer: A rage::CDynamicEntity pointer.
	// Returns: number: The script game entity handle linked to the given rage::CDynamicEntity pointer.
	static int ptr_to_handle(pointer mem_addr)
	{
		if (mem_addr.is_null())
			return 0;

		return big::g_pointers->m_gta.m_ptr_to_handle((void*)mem_addr.get_address());
	}

	// Lua API: Function
	// Table: memory
	// Name: allocate
	// Param: size: integer: The number of bytes to allocate on the heap.
	// Returns: pointer: A pointer to the newly allocated memory.
	static pointer allocate(int size, sol::this_state state)
	{
		void* mem = new uint8_t[size]();

		big::lua_module* module = sol::state_view(state)["!this"];
		module->m_allocated_memory.push_back(mem);

		return pointer((uint64_t)mem);
	}

	// Lua API: Function
	// Table: memory
	// Name: free
	// Param: ptr: pointer: The pointer that must be freed.
	static void free(pointer ptr, sol::this_state state)
	{
		delete[] (void*)ptr.get_address();

		big::lua_module* module = sol::state_view(state)["!this"];

		std::erase_if(module->m_allocated_memory, [ptr](void* addr) {
			return ptr.get_address() == (uint64_t)addr;
		});
	}

	value_wrapper_t::value_wrapper_t(char* val, type_info_t type)
	{
		m_value = val;
		m_type  = type;
	}

	sol::object value_wrapper_t::get(sol::this_state state_)
	{
		if (m_type == type_info_t::boolean_)
		{
			return sol::make_object(state_, *(bool*)m_value);
		}
		else if (m_type == type_info_t::string_)
		{
			return sol::make_object(state_, *(const char**)m_value);
		}
		else if (m_type == type_info_t::integer_)
		{
			return sol::make_object(state_, *(int64_t*)m_value);
		}
		else if (m_type == type_info_t::float_)
		{
			return sol::make_object(state_, *(float*)m_value);
		}
		else if (m_type == type_info_t::double_)
		{
			return sol::make_object(state_, *(double*)m_value);
		}
		else
		{
			return sol::nil;
		}
	}

	void value_wrapper_t::set(sol::object new_val, sol::this_state state_)
	{
		if (m_type == type_info_t::boolean_ && new_val.is<bool>())
		{
			*(bool*)m_value = new_val.as<bool>();
		}
		else if (m_type == type_info_t::string_ && new_val.is<const char*>())
		{
			*(const char**)m_value = new_val.as<const char*>();
		}
		else if (m_type == type_info_t::integer_ && new_val.is<int64_t>())
		{
			*(int64_t*)m_value = new_val.as<int64_t>();
		}
		else if (m_type == type_info_t::float_ && new_val.is<float>())
		{
			*(float*)m_value = new_val.as<float>();
		}
		else if (m_type == type_info_t::double_ && new_val.is<double>())
		{
			*(double*)m_value = new_val.as<double>();
		}
	}

	static bool pre_callback(const runtime_func_t::parameters_t* params, const uint8_t param_count, runtime_func_t::return_value_t* return_value, const uintptr_t target_func_ptr)
	{
		const auto& dyn_hook = big::g_lua_manager->m_target_func_ptr_to_dynamic_hook[target_func_ptr];
		return big::g_lua_manager
		    ->dynamic_hook_pre_callbacks(target_func_ptr, dyn_hook->m_return_type, return_value, dyn_hook->m_param_types, params, param_count);
	}

	static void post_callback(const runtime_func_t::parameters_t* params, const uint8_t param_count, runtime_func_t::return_value_t* return_value, const uintptr_t target_func_ptr)
	{
		const auto& dyn_hook = big::g_lua_manager->m_target_func_ptr_to_dynamic_hook[target_func_ptr];
		big::g_lua_manager->dynamic_hook_post_callbacks(target_func_ptr, dyn_hook->m_return_type, return_value, dyn_hook->m_param_types, params, param_count);
	}

	// Lua API: Function
	// Table: memory
	// Name: dynamic_hook
	// Param: hook_name: string: The name of the hook.
	// Param: return_type: string: Type of the return value of the detoured function.
	// Param: param_types: table<string>: Types of the parameters of the detoured function.
	// Param: target_func_ptr: memory.pointer: The pointer to the function to detour.
	// Param: pre_callback: function: The function that will be called before the original function is about to be called. The callback must match the following signature: ( return_value (value_wrapper), arg1 (value_wrapper), arg2 (value_wrapper), ... ) -> Returns true or false (boolean) depending on whether you want the original function to be called.
	// Param: post_callback: function: The function that will be called after the original function is called (or just after the pre callback is called, if the original function was skipped). The callback must match the following signature: ( return_value (value_wrapper), arg1 (value_wrapper), arg2 (value_wrapper), ... ) -> void
	// **Example Usage:**
	// ```lua
	// local ptr = memory.scan_pattern("some ida sig")
	// -- Check the implementation of the asmjit::TypeId get_type_id function if you are unsure what to use for return type / parameters types
	// memory.dynamic_hook("test_hook", "float", {"const char*"}, ptr,
	// function(ret_val, str)
	//
	//     --str:set("replaced str")
	//     ret_val:set(69.69)
	//     log.info("pre callback from lua", ret_val:get(), str:get())
	//
	//     -- false for skipping the original function call
	//     return false
	// end,
	// function(ret_val, str)
	//     log.info("post callback from lua 1", ret_val:get(), str:get())
	//     ret_val:set(79.69)
	//     log.info("post callback from lua 2", ret_val:get(), str:get())
	// end)
	// ```
	static void dynamic_hook(const std::string& hook_name, const std::string& return_type, sol::table param_types_table, lua::memory::pointer& target_func_ptr_obj, sol::protected_function pre_lua_callback, sol::protected_function post_lua_callback, sol::this_state state_)
	{
		if (!target_func_ptr_obj.is_valid())
		{
			return;
		}

		big::lua_module* module = sol::state_view(state_)["!this"];
		if (!module)
		{
			return;
		}

		const auto target_func_ptr = target_func_ptr_obj.get_address();

		bool need_hook = false;
		if (pre_lua_callback.valid())
		{
			module->m_dynamic_hook_pre_callbacks[target_func_ptr].push_back(pre_lua_callback);
			need_hook = true;
		}
		if (post_lua_callback.valid())
		{
			module->m_dynamic_hook_post_callbacks[target_func_ptr].push_back(post_lua_callback);
			need_hook = true;
		}

		if (need_hook)
		{
			std::shared_ptr<runtime_func_t> runtime_func;

			if (!big::g_lua_manager->m_target_func_ptr_to_dynamic_hook.contains(target_func_ptr))
			{
				std::vector<std::string> param_types;
				for (const auto& [k, v] : param_types_table)
				{
					if (v.is<const char*>())
					{
						param_types.push_back(v.as<const char*>());
					}
				}

				runtime_func = std::make_shared<runtime_func_t>();
				const auto jitted_func = runtime_func->make_jit_func(return_type, param_types, asmjit::Arch::kHost, pre_callback, post_callback, target_func_ptr);

				big::g_lua_manager->m_target_func_ptr_to_dynamic_hook[target_func_ptr] = runtime_func.get();

				big::g_lua_manager->m_target_func_ptr_to_dynamic_hook[target_func_ptr]->create_and_enable_hook(hook_name, target_func_ptr, jitted_func);
			}
			else
			{
				// lua modules own and share the runtime_func_t object, such as when no module reference it anymore the hook detour get cleaned up.
				runtime_func = big::g_lua_manager->get_existing_dynamic_hook(target_func_ptr);
			}

			if (runtime_func)
			{
				module->m_dynamic_hooks.push_back(runtime_func);
			}
		}
	}

	static std::string get_jitted_lua_func_global_name(uintptr_t function_to_call_ptr)
	{
		return std::format("__dynamic_call_{}", function_to_call_ptr);
	}

	class asmjit_error_handler_t : public asmjit::ErrorHandler
	{
	public:
		void handleError(asmjit::Error err, const char* message, asmjit::BaseEmitter* origin) override
		{
			LOG(FATAL) << "asmjit error: " << message;
		}
	};

	static std::unique_ptr<uint8_t[]> jit_lua_binded_func(uintptr_t function_to_call_ptr, const asmjit::FuncSignature& function_to_call_sig, const asmjit::Arch& arch, std::vector<type_info_t> param_types, type_info_t return_type, lua_State* lua_state, const std::string& jitted_lua_func_global_name)
	{
		asmjit::CodeHolder code;
		auto env = asmjit::Environment::host();
		env.setArch(arch);
		auto asmjit_error = code.init(env);

		// initialize function
		asmjit::x86::Compiler cc(&code);
		// clang-format off
		asmjit::FuncNode* func = cc.addFunc(asmjit::FuncSignature(asmjit::CallConvId::kFastCall, asmjit::FuncSignature::kNoVarArgs,
			asmjit::TypeId::kInt32,
			asmjit::TypeId::kUIntPtr));
		// clang-format on

		asmjit::StringLogger log;
		// clang-format off
			const auto format_flags =
				asmjit::FormatFlags::kMachineCode | asmjit::FormatFlags::kExplainImms | asmjit::FormatFlags::kRegCasts |
				asmjit::FormatFlags::kHexImms     | asmjit::FormatFlags::kHexOffsets  | asmjit::FormatFlags::kPositions;
		// clang-format on

		log.addFlags(format_flags);
		code.setLogger(&log);
		asmjit_error_handler_t asmjit_error_handler;
		code.setErrorHandler(&asmjit_error_handler);

		// map argument slots to registers, following abi.
		std::vector<asmjit::x86::Reg> arg_registers;
		for (uint8_t arg_index = 0; arg_index < function_to_call_sig.argCount(); arg_index++)
		{
			const auto arg_type = function_to_call_sig.args()[arg_index];

			// for each "function to call" parameter
			// InvokeNode for the corresponding lua_toXXX func
			// result of the invokenode setRet goes to the arg Reg.
			// those Reg will then be setArg of the function_to_call final InvokeNode.

			asmjit::x86::Reg arg;

			const auto arg_type_info = param_types[arg_index];
			if (arg_type_info == type_info_t::integer_ || arg_type_info == type_info_t::float_ || arg_type_info == type_info_t::double_)
			{
				asmjit::InvokeNode* lua_tofunc;
				// clang-format off
				cc.invoke(&lua_tofunc, lua_tonumberx, asmjit::FuncSignature(asmjit::CallConvId::kCDecl, asmjit::FuncSignature::kNoVarArgs,
					asmjit::TypeId::kFloat64,
					asmjit::TypeId::kUIntPtr, asmjit::TypeId::kInt32, asmjit::TypeId::kUIntPtr));
				// clang-format on

				lua_tofunc->setArg(0, lua_state);
				lua_tofunc->setArg(1, arg_index + 1);
				lua_tofunc->setArg(2, NULL);

				const auto tmp = cc.newXmm();
				lua_tofunc->setRet(0, tmp);
				if (arg_type_info == type_info_t::integer_)
				{
					arg = cc.newUIntPtr();
					cc.cvttsd2si(arg.as<asmjit::x86::Gp>(), tmp);
				}
				else if (arg_type_info == type_info_t::float_)
				{
					arg = cc.newXmm();
					cc.cvtsd2ss(arg.as<asmjit::x86::Xmm>(), tmp);
				}
				else
				{
					arg = tmp;
				}
			}
			else if (arg_type_info == type_info_t::boolean_)
			{
				asmjit::InvokeNode* lua_tofunc;
				// clang-format off
				cc.invoke(&lua_tofunc, lua_toboolean, asmjit::FuncSignature(asmjit::CallConvId::kCDecl, asmjit::FuncSignature::kNoVarArgs,
					asmjit::TypeId::kInt32,
					asmjit::TypeId::kUIntPtr, asmjit::TypeId::kInt32));
				// clang-format on

				lua_tofunc->setArg(0, lua_state);
				lua_tofunc->setArg(1, arg_index + 1);

				arg = cc.newUIntPtr();
				lua_tofunc->setRet(0, arg);
			}
			else if (arg_type_info == type_info_t::string_)
			{
				asmjit::InvokeNode* lua_tofunc;
				// clang-format off
				cc.invoke(&lua_tofunc, lua_tolstring, asmjit::FuncSignature(asmjit::CallConvId::kCDecl, asmjit::FuncSignature::kNoVarArgs,
					asmjit::TypeId::kUIntPtr,
					asmjit::TypeId::kUIntPtr, asmjit::TypeId::kInt32, asmjit::TypeId::kUIntPtr));
				// clang-format on

				lua_tofunc->setArg(0, lua_state);
				lua_tofunc->setArg(1, arg_index + 1);
				lua_tofunc->setArg(2, NULL);

				arg = cc.newUIntPtr();
				lua_tofunc->setRet(0, arg);
			}
			else if (arg_type_info == type_info_t::ptr_)
			{
				asmjit::InvokeNode* lua_tofunc;
				// clang-format off
				cc.invoke(&lua_tofunc, lua_tonumberx, asmjit::FuncSignature(asmjit::CallConvId::kCDecl, asmjit::FuncSignature::kNoVarArgs,
					asmjit::TypeId::kFloat64,
					asmjit::TypeId::kUIntPtr, asmjit::TypeId::kInt32, asmjit::TypeId::kUIntPtr));
				// clang-format on

				lua_tofunc->setArg(0, lua_state);
				lua_tofunc->setArg(1, arg_index + 1);
				lua_tofunc->setArg(2, NULL);

				// lua_Number (double) to integer type
				const auto tmp = cc.newXmm();
				lua_tofunc->setRet(0, tmp);
				arg = cc.newUIntPtr();
				cc.cvttsd2si(arg.as<asmjit::x86::Gp>(), tmp);
			}

			arg_registers.push_back(arg);
		}

		asmjit::InvokeNode* function_to_call_invoke_node;
		cc.invoke(&function_to_call_invoke_node, function_to_call_ptr, function_to_call_sig);
		for (uint8_t arg_index = 0; arg_index < function_to_call_sig.argCount(); arg_index++)
		{
			function_to_call_invoke_node->setArg(arg_index, arg_registers.at(arg_index));
		}
		asmjit::x86::Reg function_to_call_return_val_reg;
		if (is_general_register(function_to_call_sig.ret()))
		{
			function_to_call_return_val_reg = cc.newUIntPtr();
		}
		else if (is_XMM_register(function_to_call_sig.ret()))
		{
			function_to_call_return_val_reg = cc.newXmm();
		}
		else
		{
			LOG(FATAL) << "Return val wider than 64bits not supported";
			return nullptr;
		}
		function_to_call_invoke_node->setRet(0, function_to_call_return_val_reg);

		if (return_type == type_info_t::integer_ || return_type == type_info_t::float_ || return_type == type_info_t::double_)
		{
			if (function_to_call_sig.ret() >= asmjit::TypeId::_kIntStart && function_to_call_sig.ret() <= asmjit::TypeId::_kIntEnd)
			{
				// the function returned to a Gp register, need to convert it to a lua_Number compatible register.
				const auto tmp = cc.newXmm();
				cc.cvtsi2sd(tmp, function_to_call_return_val_reg.as<asmjit::x86::Gp>());
				function_to_call_return_val_reg = tmp;
			}
			else if (function_to_call_sig.ret() == asmjit::TypeId::kFloat32)
			{
				// m128_f32 -> m128_f64 (lua_Number)
				cc.cvtss2sd(function_to_call_return_val_reg.as<asmjit::x86::Xmm>(), function_to_call_return_val_reg.as<asmjit::x86::Xmm>());
			}

			asmjit::InvokeNode* lua_pushfunc;
			// clang-format off
			cc.invoke(&lua_pushfunc, lua_pushnumber, asmjit::FuncSignature(asmjit::CallConvId::kCDecl, asmjit::FuncSignature::kNoVarArgs,
				asmjit::TypeId::kVoid,
				asmjit::TypeId::kUIntPtr, asmjit::TypeId::kFloat64));
			// clang-format on

			lua_pushfunc->setArg(0, lua_state);
			lua_pushfunc->setArg(1, function_to_call_return_val_reg);
		}
		else if (return_type == type_info_t::boolean_)
		{
			asmjit::InvokeNode* lua_pushfunc;
			// clang-format off
			cc.invoke(&lua_pushfunc, lua_pushboolean, asmjit::FuncSignature(asmjit::CallConvId::kCDecl, asmjit::FuncSignature::kNoVarArgs,
				asmjit::TypeId::kVoid,
				asmjit::TypeId::kUIntPtr, asmjit::TypeId::kInt8));
			// clang-format on

			lua_pushfunc->setArg(0, lua_state);
			lua_pushfunc->setArg(1, function_to_call_return_val_reg);
		}
		else if (return_type == type_info_t::string_)
		{
			asmjit::InvokeNode* lua_pushfunc;
			// clang-format off
			cc.invoke(&lua_pushfunc, lua_pushstring, asmjit::FuncSignature(asmjit::CallConvId::kCDecl, asmjit::FuncSignature::kNoVarArgs,
				asmjit::TypeId::kVoid,
				asmjit::TypeId::kUIntPtr, asmjit::TypeId::kUIntPtr));
			// clang-format on

			lua_pushfunc->setArg(0, lua_state);
			lua_pushfunc->setArg(1, function_to_call_return_val_reg);
		}
		else if (return_type == type_info_t::ptr_)
		{
			// integer type to lua_Number (double)
			asmjit::x86::Xmm tmp = cc.newXmm();
			cc.cvtsi2sd(tmp, function_to_call_return_val_reg.as<asmjit::x86::Gp>());

			asmjit::InvokeNode* lua_pushfunc;
			// clang-format off
			cc.invoke(&lua_pushfunc, lua_pushnumber, asmjit::FuncSignature(asmjit::CallConvId::kCDecl, asmjit::FuncSignature::kNoVarArgs,
				asmjit::TypeId::kVoid,
				asmjit::TypeId::kUIntPtr, asmjit::TypeId::kFloat64));
			// clang-format on

			lua_pushfunc->setArg(0, lua_state);
			lua_pushfunc->setArg(1, tmp);
		}

		// a lua binded c func always return an int which hold the number of returned vars.
		asmjit::x86::Gp retReg = cc.newUIntPtr();
		cc.mov(retReg, function_to_call_sig.hasRet() ? 1 : 0);
		cc.ret(retReg);

		cc.endFunc();

		// write to buffer
		cc.finalize();

		// worst case, overestimates for case trampolines needed
		code.flatten();
		size_t size = code.codeSize();

		// Allocate a virtual memory (executable).
		auto jit_function_buffer = std::make_unique<uint8_t[]>(size);
		DWORD old_protect;
		VirtualProtect(jit_function_buffer.get(), size, PAGE_EXECUTE_READWRITE, &old_protect);

		// if multiple sections, resolve linkage (1 atm)
		if (code.hasUnresolvedLinks())
		{
			code.resolveUnresolvedLinks();
		}

		// Relocate to the base-address of the allocated memory.
		code.relocateToBase((uintptr_t)jit_function_buffer.get());
		code.copyFlattenedData(jit_function_buffer.get(), size);

		LOG(VERBOSE) << "JIT Stub: " << log.data();

		lua_pushcfunction(lua_state, (lua_CFunction)jit_function_buffer.get());
		lua_setglobal(lua_state, jitted_lua_func_global_name.c_str());

		return jit_function_buffer;
	}

	// Lua API: Function
	// Table: memory
	// Name: dynamic_call
	// Param: return_type: string: Type of the return value of the function to call.
	// Param: param_types: table<string>: Types of the parameters of the function to call.
	// Param: target_func_ptr: memory.pointer: The pointer to the function to call.
	// Returns: string: Key name of the function that you can now call from lua.
	// **Example Usage:**
	// ```lua
	// -- the sig in this example leads to an implementation of memcpy_s
	// local ptr = memory.scan_pattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 49 8B D9 49 8B F0 48 8B FA")
	// if ptr:is_valid() then
	//     local dest_size = 8
	//     local dest_ptr = memory.allocate(dest_size)
	//     dest_ptr:set_qword(0)
	//
	//     local src_size = 8
	//     local src_ptr = memory.allocate(src_size)
	//     src_ptr:set_qword(123)
	//
	//     -- Check the implementation of the asmjit::TypeId get_type_id function if you are unsure what to use for return type / parameters types
	//     local func_to_call_test_global_name = memory.dynamic_call("int", {"void*", "uint64_t", "void*", "uint64_t"}, ptr)
	//     -- print zero.
	//     log.info(dest_ptr:get_qword())
	//     -- note: don't pass memory.pointer objects directly when you call the function, but use get_address() instead.
	//     local call_res_test = _G[func_to_call_test_global_name](dest_ptr:get_address(), dest_size, src_ptr:get_address(), src_size)
	//     -- print 123.
	//     log.info(dest_ptr:get_qword())
	// end
	// ```
	static std::string dynamic_call(const std::string& return_type, sol::table param_types_table, lua::memory::pointer& target_func_ptr_obj, sol::this_state state_)
	{
		big::lua_module* module = sol::state_view(state_)["!this"];
		if (!module)
		{
			return "";
		}

		if (!target_func_ptr_obj.is_valid())
		{
			return "";
		}

		const auto target_func_ptr = target_func_ptr_obj.get_address();

		const auto jitted_lua_func_global_name = get_jitted_lua_func_global_name(target_func_ptr);

		const auto already_jitted_func = sol::state_view(state_)[jitted_lua_func_global_name];
		if (already_jitted_func.is<sol::protected_function>())
		{
			return already_jitted_func;
		}

		std::vector<std::string> param_types_strings;
		for (const auto& [k, v] : param_types_table)
		{
			if (v.is<const char*>())
			{
				param_types_strings.push_back(v.as<const char*>());
			}
		}

		std::string call_convention = "";
		asmjit::FuncSignature sig(get_call_convention(call_convention), asmjit::FuncSignature::kNoVarArgs, get_type_id(return_type));

		std::vector<type_info_t> param_types;
		for (const std::string& s : param_types_strings)
		{
			sig.addArg(get_type_id(s));
			param_types.push_back(get_type_info_from_string(s));
		}

		if (!module->m_dynamic_call_jit_functions.contains(target_func_ptr))
		{
			auto jitted_func = jit_lua_binded_func(target_func_ptr,
			    sig,
			    asmjit::Arch::kHost,
			    param_types,
			    get_type_info_from_string(return_type),
			    state_.L,
			    jitted_lua_func_global_name);

			if (jitted_func)
			{
				module->m_dynamic_call_jit_functions.emplace(target_func_ptr, std::move(jitted_func));
			}
			else
			{
				return "";
			}
		}

		return jitted_lua_func_global_name;
	}

	void bind(sol::state& state)
	{
		auto ns = state["memory"].get_or_create<sol::table>();

		auto pointer_ut = ns.new_usertype<pointer>("pointer", sol::constructors<pointer(uint64_t)>());

		pointer_ut["add"]         = &pointer::add;
		pointer_ut["sub"]         = &pointer::sub;
		pointer_ut["rip"]         = &pointer::rip;
		pointer_ut["get_int"]     = &pointer::get<int32_t>;
		pointer_ut["get_byte"]    = &pointer::get<uint8_t>;
		pointer_ut["get_word"]    = &pointer::get<uint16_t>;
		pointer_ut["get_dword"]   = &pointer::get<uint32_t>;
		pointer_ut["get_qword"]   = &pointer::get<uint64_t>;
		pointer_ut["get_float"]   = &pointer::get<float>;
		pointer_ut["get_string"]  = &pointer::get_string;
		pointer_ut["set_int"]     = &pointer::set<int32_t>;
		pointer_ut["set_byte"]    = &pointer::set<uint8_t>;
		pointer_ut["set_word"]    = &pointer::set<uint16_t>;
		pointer_ut["set_dword"]   = &pointer::set<uint32_t>;
		pointer_ut["set_qword"]   = &pointer::set<uint64_t>;
		pointer_ut["set_float"]   = &pointer::set<float>;
		pointer_ut["set_string"]  = &pointer::set_string;
		pointer_ut["patch_byte"]  = &pointer::patch<uint8_t>;
		pointer_ut["patch_word"]  = &pointer::patch<uint16_t>;
		pointer_ut["patch_dword"] = &pointer::patch<uint32_t>;
		pointer_ut["patch_qword"] = &pointer::patch<uint64_t>;
		pointer_ut["is_null"]     = &pointer::is_null;
		pointer_ut["is_valid"]    = &pointer::is_valid;
		pointer_ut["deref"]       = &pointer::deref;
		pointer_ut["get_address"] = &pointer::get_address;
		pointer_ut["set_address"] = &pointer::set_address;

		auto patch_ut       = ns.new_usertype<big::lua_patch>("patch", sol::no_constructor);
		patch_ut["apply"]   = &big::lua_patch::apply;
		patch_ut["restore"] = &big::lua_patch::restore;

		ns["scan_pattern"]  = scan_pattern;
		ns["handle_to_ptr"] = handle_to_ptr;
		ns["ptr_to_handle"] = ptr_to_handle;
		ns["allocate"]      = allocate;
		ns["free"]          = free;

		ns.new_usertype<value_wrapper_t>("value_wrapper", "get", &value_wrapper_t::get, "set", &value_wrapper_t::set);
		ns["dynamic_hook"] = dynamic_hook;

		ns["dynamic_call"] = dynamic_call;
	}
}