#pragma once

namespace lua::scr_patch
{
	class scr_patch
	{
		rage::joaat_t m_script;
		std::string m_patch_name;
		std::string m_pattern;
		int m_offset;
		sol::table m_patch;
		bool m_enable;

	public:
		// Lua API: Constructor
		// Class: scr_patch
		// Param: script_name: string: The name of the script.
		// Param: patch_name: string: The name of the patch.
		// Param: pattern: string: The pattern to scan for within the script.
		// Param: offset: integer: The position within the pattern.
		// Param: patch_: table: The bytes to be written into the script's bytecode.
		// Adds a patch for the specified script.
		explicit scr_patch(const std::string& script_name, const std::string& patch_name, const std::string& pattern, const int offset, sol::table patch_, sol::this_state state);
		~scr_patch();

		// Lua API: Function
		// Class: scr_patch
		// Name: enable
		// Enables the script patch for the current instance. When a new instance is created, it will be enabled by default.
		void enable();
		
		// Lua API: Function
		// Class: scr_patch
		// Name: disable
		// Disables the script patch for the current instance.
		void disable();
	};

	void bind(sol::state& state);
}