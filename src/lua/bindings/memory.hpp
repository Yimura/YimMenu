#pragma once
#include "lua/bindings/type_info_t.hpp"
#include "lua/lua_module.hpp"

namespace lua::memory
{
	// Lua API: Class
	// Name: pointer
	// Class representing a 64-bit memory address.

	struct pointer
	{
	private:
		uint64_t m_address;

	public:
		// Lua API: Constructor
		// Class: pointer
		// Param: address: integer: Address
		// Returns a memory instance, with the given address.
		explicit pointer(uint64_t address);

		explicit pointer();

		// Lua API: Function
		// Class: pointer
		// Name: add
		// Param: offset: integer: offset
		// Returns: pointer: new pointer object.
		// Adds an offset to the current memory address and returns a new pointer object.
		pointer add(uint64_t offset);

		// Lua API: Function
		// Class: pointer
		// Name: sub
		// Param: offset: integer: offset
		// Returns: pointer: new pointer object.
		// Subs an offset to the current memory address and returns a new pointer object.
		pointer sub(uint64_t offset);

		// Lua API: Function
		// Class: pointer
		// Name: rip
		// Param: offset: integer: offset
		// Returns: pointer: new pointer object.
		// Rips the current memory address and returns a new pointer object.
		pointer rip();

		// Lua API: Function
		// Class: pointer
		// Name: get_byte
		// Returns: number: the value stored at the memory address as the specified type.
		// Retrieves the value stored at the memory address as the specified type.

		// Lua API: Function
		// Class: pointer
		// Name: get_word
		// Returns: number: the value stored at the memory address as the specified type.
		// Retrieves the value stored at the memory address as the specified type.

		// Lua API: Function
		// Class: pointer
		// Name: get_int
		// Returns: number: the value stored at the memory address as the specified type.
		// Retrieves the value stored at the memory address as the specified type.

		// Lua API: Function
		// Class: pointer
		// Name: get_dword
		// Returns: number: the value stored at the memory address as the specified type.
		// Retrieves the value stored at the memory address as the specified type.

		// Lua API: Function
		// Class: pointer
		// Name: get_float
		// Returns: float: the value stored at the memory address as the specified type.
		// Retrieves the value stored at the memory address as the specified type.

		// Lua API: Function
		// Class: pointer
		// Name: get_qword
		// Returns: number: the value stored at the memory address as the specified type.
		// Retrieves the value stored at the memory address as the specified type.

		template<typename T>
		T get()
		{
			return *(T*)m_address;
		}

		// Lua API: Function
		// Class: pointer
		// Name: set_byte
		// Param: value: number: new value.
		// Sets the value at the memory address to the specified value of the given type.

		// Lua API: Function
		// Class: pointer
		// Name: set_word
		// Param: value: number: new value.
		// Sets the value at the memory address to the specified value of the given type.

		// Lua API: Function
		// Class: pointer
		// Name: set_int
		// Param: value: number: new value.
		// Sets the value at the memory address to the specified value of the given type.

		// Lua API: Function
		// Class: pointer
		// Name: set_dword
		// Param: value: number: new value.
		// Sets the value at the memory address to the specified value of the given type.

		// Lua API: Function
		// Class: pointer
		// Name: set_float
		// Param: value: float: new value.
		// Sets the value at the memory address to the specified value of the given type.

		// Lua API: Function
		// Class: pointer
		// Name: set_qword
		// Param: value: number: new value.
		// Sets the value at the memory address to the specified value of the given type.

		template<typename T>
		void set(T value)
		{
			*(T*)m_address = value;
		}

		// Lua API: Function
		// Class: pointer
		// Name: get_string
		// Returns: string: the value stored at the memory address as the specified type.
		// Retrieves the value stored at the memory address as the specified type.
		std::string get_string();

		// Lua API: Function
		// Class: pointer
		// Name: set_string
		// Param: value: string: new value.
		// Sets the value at the memory address to the specified value of the given type.
		void set_string(const std::string& string, int max_length);

		// Lua API: Function
		// Class: pointer
		// Name: patch_byte
		// Param: value: number: new value.
		// Returns: lua_patch: memory patch instance for modifying the value at the memory address with the specified value. Can call apply / restore on the object.
		// Creates a memory patch for modifying the value at the memory address with the specified value.
		// The modified value is applied when you call the apply function on the lua_patch object.
		// The original value is restored when you call the restore function on the lua_patch object.

		// Lua API: Function
		// Class: pointer
		// Name: patch_word
		// Param: value: number: new value.
		// Returns: lua_patch: memory patch instance for modifying the value at the memory address with the specified value. Can call apply / restore on the object.
		// Creates a memory patch for modifying the value at the memory address with the specified value.
		// The modified value is applied when you call the apply function on the lua_patch object.
		// The original value is restored when you call the restore function on the lua_patch object.

		// Lua API: Function
		// Class: pointer
		// Name: patch_dword
		// Param: value: number: new value.
		// Returns: lua_patch: memory patch instance for modifying the value at the memory address with the specified value. Can call apply / restore on the object.
		// Creates a memory patch for modifying the value at the memory address with the specified value.
		// The modified value is applied when you call the apply function on the lua_patch object.
		// The original value is restored when you call the restore function on the lua_patch object.

		// Lua API: Function
		// Class: pointer
		// Name: patch_qword
		// Param: value: number: new value.
		// Returns: lua_patch: memory patch instance for modifying the value at the memory address with the specified value. Can call apply / restore on the object.
		// Creates a memory patch for modifying the value at the memory address with the specified value.
		// The modified value is applied when you call the apply function on the lua_patch object.
		// The original value is restored when you call the restore function on the lua_patch object.

		template<typename T>
		big::lua_patch* patch(T value, sol::this_state state)
		{
			big::lua_module* module = sol::state_view(state)["!this"];

			auto patch = std::make_unique<big::lua_patch>(::memory::byte_patch::make((T*)m_address, value).get());
			auto raw   = patch.get();
			module->m_registered_patches.push_back(std::move(patch));
			return raw;
		}

		// Lua API: Function
		// Class: pointer
		// Name: is_null
		// Returns: boolean: Returns true if the address is null.
		bool is_null();

		// Lua API: Function
		// Class: pointer
		// Name: is_valid
		// Returns: boolean: Returns true if the address is not null.
		bool is_valid();

		// Lua API: Function
		// Class: pointer
		// Name: deref
		// Returns: pointer: A new pointer object pointing to the value at that address.
		// Dereferences the memory address and returns a new pointer object pointing to the value at that address.
		pointer deref();

		// Lua API: Function
		// Class: pointer
		// Name: get_address
		// Returns: number: The memory address stored in the pointer object as a number.
		// Retrieves the memory address stored in the pointer object.
		uint64_t get_address() const;

		// Lua API: Function
		// Class: pointer
		// Name: set_address
		// Param: address: integer: new address.
		// Sets the memory address stored in the pointer object.
		void set_address(uint64_t address);
	};

	// Lua API: Class
	// Name: value_wrapper
	// Class for wrapping parameters and return value of functions, used mostly by the dynamic_hook system.

	class value_wrapper_t
	{
		char* m_value      = nullptr;
		type_info_t m_type = type_info_t::none_;

	public:
		value_wrapper_t(char* val, type_info_t type);

		// Lua API: Function
		// Class: value_wrapper
		// Name: get
		// Returns: any: The current value.
		// Get the value currently contained by the wrapper.
		sol::object get(sol::this_state state_);

		// Lua API: Function
		// Class: value_wrapper
		// Name: set
		// Param: new_value: any: The new value.
		// Set the new value contained by the wrapper.
		void set(sol::object new_val, sol::this_state state_);
	};

	void bind(sol::state& state);
}