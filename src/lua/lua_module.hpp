#pragma once
#include "../script.hpp"
#include "bindings/gui/gui_element.hpp"
#include "core/data/menu_event.hpp"
#include "lua/bindings/runtime_func_t.hpp"
#include "lua/bindings/scr_patch.hpp"
#include "lua/bindings/type_info_t.hpp"
#include "lua_patch.hpp"
#include "services/gui/gui_service.hpp"

namespace big
{
	class lua_module
	{
		sol::state m_state;

		sol::protected_function m_io_open;

		std::filesystem::path m_module_path;

		std::string m_module_name;
		rage::joaat_t m_module_id;

		std::chrono::time_point<std::chrono::file_clock> m_last_write_time;

		bool m_disabled;
		std::mutex m_registered_scripts_mutex;

	public:
		std::vector<std::unique_ptr<script>> m_registered_scripts;
		std::vector<std::unique_ptr<lua_patch>> m_registered_patches;
		std::vector<std::unique_ptr<lua::scr_patch::scr_patch>> m_registered_script_patches;

		std::vector<big::tabs> m_owned_tabs;

		std::unordered_map<big::tabs, std::vector<big::tabs>> m_tab_to_sub_tabs;

		std::vector<std::unique_ptr<lua::gui::gui_element>> m_independent_gui;
		std::vector<std::unique_ptr<lua::gui::gui_element>> m_always_draw_gui;
		std::unordered_map<rage::joaat_t, std::vector<std::unique_ptr<lua::gui::gui_element>>> m_gui;
		std::unordered_map<menu_event, std::vector<sol::protected_function>> m_event_callbacks;
		std::vector<void*> m_allocated_memory;

		// lua modules own and share the runtime_func_t object, such as when no module reference it anymore the hook detour get cleaned up.
		std::vector<std::shared_ptr<lua::memory::runtime_func_t>> m_dynamic_hooks;
		std::unordered_map<uintptr_t, std::vector<sol::protected_function>> m_dynamic_hook_pre_callbacks;
		std::unordered_map<uintptr_t, std::vector<sol::protected_function>> m_dynamic_hook_post_callbacks;

		std::unordered_map<uintptr_t, std::unique_ptr<uint8_t[]>> m_dynamic_call_jit_functions;

		lua_module(const std::filesystem::path& module_path, folder& scripts_folder, bool disabled = false);
		~lua_module();

		const std::filesystem::path& module_path() const;

		rage::joaat_t module_id() const;
		const std::string& module_name() const;
		const std::chrono::time_point<std::chrono::file_clock> last_write_time() const;
		const bool is_disabled() const;

		// used for sandboxing and limiting to only our custom search path for the lua require function
		void set_folder_for_lua_require(folder& scripts_folder);

		void sandbox_lua_os_library();
		void sandbox_lua_io_library();
		void sandbox_lua_loads(folder& scripts_folder);

		void init_lua_api(folder& scripts_folder);

		void load_and_call_script();

		inline void for_each_script(auto func)
		{
			std::lock_guard guard(m_registered_scripts_mutex);

			for (auto& script : m_registered_scripts)
			{
				func(script.get());
			}
		}

		void tick_scripts();
		void cleanup_done_scripts();

		sol::object to_lua(const lua::memory::runtime_func_t::parameters_t* params, const uint8_t i, const std::vector<lua::memory::type_info_t>& param_types);
		sol::object to_lua(lua::memory::runtime_func_t::return_value_t* return_value, const lua::memory::type_info_t return_value_type);
	};
}