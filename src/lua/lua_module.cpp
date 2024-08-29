#include "lua_module.hpp"

#include "bindings/command.hpp"
#include "bindings/entities.hpp"
#include "bindings/event.hpp"
#include "bindings/global_table.hpp"
#include "bindings/globals.hpp"
#include "bindings/gui.hpp"
#include "bindings/imgui.hpp"
#include "bindings/locals.hpp"
#include "bindings/log.hpp"
#include "bindings/memory.hpp"
#include "bindings/native.hpp"
#include "bindings/network.hpp"
#include "bindings/script.hpp"
#include "bindings/scr_function.hpp"
#include "bindings/scr_patch.hpp"
#include "bindings/self.hpp"
#include "bindings/stats.hpp"
#include "bindings/tunables.hpp"
#include "bindings/vector.hpp"
#include "bindings/vehicles.hpp"
#include "bindings/weapons.hpp"
#include "file_manager.hpp"
#include "script_mgr.hpp"

namespace big
{
	// https://sol2.readthedocs.io/en/latest/exceptions.html
	int exception_handler(lua_State* L, sol::optional<const std::exception&> maybe_exception, sol::string_view description)
	{
		// L is the lua state, which you can wrap in a state_view if necessary
		// maybe_exception will contain exception, if it exists
		// description will either be the what() of the exception or a description saying that we hit the general-case catch(...)
		if (maybe_exception)
		{
			const std::exception& ex = *maybe_exception;
			LOG(FATAL) << ex.what();
		}
		else
		{
			LOG(FATAL) << description;
		}
		Logger::FlushQueue();

		// you must push 1 element onto the stack to be
		// transported through as the error object in Lua
		// note that Lua -- and 99.5% of all Lua users and libraries -- expects a string
		// so we push a single string (in our case, the description of the error)
		return sol::stack::push(L, description);
	}

	inline void panic_handler(sol::optional<std::string> maybe_msg)
	{
		LOG(FATAL) << "Lua is in a panic state and will now abort() the application";
		if (maybe_msg)
		{
			const std::string& msg = maybe_msg.value();
			LOG(FATAL) << "error message: " << msg;
		}
		Logger::FlushQueue();

		// When this function exits, Lua will exhibit default behavior and abort()
	}

	static int traceback_error_handler(lua_State* L)
	{
		std::string msg = "An unknown error has triggered the error handler";
		sol::optional<sol::string_view> maybetopmsg = sol::stack::unqualified_check_get<sol::string_view>(L, 1, &sol::no_panic);
		if (maybetopmsg)
		{
			const sol::string_view& topmsg = maybetopmsg.value();
			msg.assign(topmsg.data(), topmsg.size());
		}
		luaL_traceback(L, L, msg.c_str(), 1);
		sol::optional<sol::string_view> maybetraceback = sol::stack::unqualified_check_get<sol::string_view>(L, -1, &sol::no_panic);
		if (maybetraceback)
		{
			const sol::string_view& traceback = maybetraceback.value();
			msg.assign(traceback.data(), traceback.size());
		}
		LOG(FATAL) << msg;
		return sol::stack::push(L, msg);
	}

	lua_module::lua_module(const std::filesystem::path& module_path, folder& scripts_folder, bool disabled) :
	    m_state(),
	    m_module_path(module_path),
	    m_module_name(module_path.filename().string()),
	    m_module_id(rage::joaat(m_module_name)),
	    m_disabled(disabled)
	{
		if (!m_disabled)
		{
			// clang-format off
			m_state.open_libraries(
				sol::lib::base,
				sol::lib::package,
				sol::lib::coroutine,
				sol::lib::string,
				sol::lib::os,
				sol::lib::math,
				sol::lib::table,
				sol::lib::bit32,
				sol::lib::io,
				sol::lib::utf8
			);
			// clang-format on

			init_lua_api(scripts_folder);

			m_state["!module_name"] = m_module_name;
			m_state["!this"]        = this;

			m_state.set_exception_handler(exception_handler);
			m_state.set_panic(sol::c_call<decltype(&panic_handler), &panic_handler>);
			lua_CFunction traceback_function = sol::c_call<decltype(&traceback_error_handler), &traceback_error_handler>;
			sol::protected_function::set_default_handler(sol::object(m_state.lua_state(), sol::in_place, traceback_function));

			m_last_write_time = std::filesystem::last_write_time(m_module_path);
		}
	}

	lua_module::~lua_module()
	{
		{
			std::lock_guard guard(m_registered_scripts_mutex);
			m_registered_scripts.clear();
			m_registered_script_patches.clear();
		}

		for (const auto owned_tab : m_owned_tabs)
		{
			big::g_gui_service->remove_from_nav(owned_tab);
		}

		for (auto memory : m_allocated_memory)
			delete[] memory;
	}

	rage::joaat_t lua_module::module_id() const
	{
		return m_module_id;
	}

	const std::string& lua_module::module_name() const
	{
		return m_module_name;
	}

	const std::filesystem::path& lua_module::module_path() const
	{
		return m_module_path;
	}

	const std::chrono::time_point<std::chrono::file_clock> lua_module::last_write_time() const
	{
		return m_last_write_time;
	}

	const bool lua_module::is_disabled() const
	{
		return m_disabled;
	}

	void lua_module::set_folder_for_lua_require(folder& scripts_folder)
	{
		std::string scripts_search_path = scripts_folder.get_path().string() + "/?.lua;";

		for (const auto& entry : std::filesystem::recursive_directory_iterator(scripts_folder.get_path(), std::filesystem::directory_options::skip_permission_denied))
		{
			if (!entry.is_directory())
				continue;

			scripts_search_path += entry.path().string() + "/?.lua;";
		}
		// Remove final ';'
		scripts_search_path.pop_back();

		m_state["package"]["path"] = scripts_search_path;
	}

	void lua_module::sandbox_lua_os_library()
	{
		const auto& os = m_state["os"];
		sol::table sandbox_os(m_state, sol::create);

		sandbox_os["clock"]    = os["clock"];
		sandbox_os["date"]     = os["date"];
		sandbox_os["difftime"] = os["difftime"];
		sandbox_os["time"]     = os["time"];

		m_state["os"] = sandbox_os;
	}

	static std::optional<std::filesystem::path> make_absolute(const std::filesystem::path& root, const std::filesystem::path& user_path)
	{
		auto final_path = std::filesystem::weakly_canonical(root / user_path);

		auto [root_end, nothing] = std::mismatch(root.begin(), root.end(), final_path.begin());

		if (root_end != root.end())
			return std::nullopt;

		return final_path;
	};

	void lua_module::sandbox_lua_io_library()
	{
		auto io = m_state["io"];
		sol::table sandbox_io(m_state, sol::create);

		m_io_open = io["open"];

		// Lua API: Table
		// Name: io
		// Table for file manipulation. Modified for security purposes.

		// Lua API: Function
		// Table: io
		// Name: open
		// Returns: file_handle: file handle or nil if can't read / write to the given path.
		sandbox_io["open"] = [this](const std::string& filename, const std::string& mode) {
			const auto scripts_config_sub_path = make_absolute(g_lua_manager->get_scripts_config_folder().get_path(), filename);
			if (!scripts_config_sub_path)
			{
				LOG(WARNING) << "io.open is restricted to the scripts_config folder, and the filename provided (" << filename << ") is outside of it.";

				return sol::reference(sol::lua_nil);
			}

			const auto res = m_io_open(scripts_config_sub_path.value().u8string().c_str(), mode).get<sol::reference>();

			if (res.get_type() == sol::type::lua_nil)
			{
				LOG(WARNING) << "Couldn't io.open a file called " << filename << " mode (" << mode << "). Note that io.open is restricted to the scripts_config folder. If you want to check if a file exists, use io.exists";
			}

			return res;
		};

		// Lua API: Function
		// Table: io
		// Name: exists
		// Returns: boolean: True if the passed file path exists
		sandbox_io["exists"] = [](const std::string& filename) -> bool {
			const auto scripts_config_sub_path = make_absolute(g_lua_manager->get_scripts_config_folder().get_path(), filename);
			if (!scripts_config_sub_path)
			{
				LOG(WARNING) << "io.open is restricted to the scripts_config folder, and the filename provided (" << filename << ") is outside of it.";

				return false;
			}

			return std::filesystem::exists(*scripts_config_sub_path);
		};

		m_state["io"] = sandbox_io;
	}

	template<size_t N>
	static constexpr auto not_supported_lua_function(const char (&function_name)[N])
	{
		return [function_name](sol::this_state state, sol::variadic_args args) {
			big::lua_module* module = sol::state_view(state)["!this"];

			LOG(FATAL) << module->module_name() << " tried calling a currently not supported lua function: " << function_name;
			Logger::FlushQueue();
		};
	}

	void lua_module::sandbox_lua_loads(folder& scripts_folder)
	{
		// That's from lua base lib, luaB
		m_state["load"]       = not_supported_lua_function("load");
		m_state["loadstring"] = not_supported_lua_function("loadstring");
		m_state["loadfile"]   = not_supported_lua_function("loadfile");
		m_state["dofile"]     = not_supported_lua_function("dofile");

		// That's from lua package lib.
		// We only allow dependencies between .lua files, no DLLs.
		m_state["package"]["loadlib"] = not_supported_lua_function("package.loadlib");
		m_state["package"]["cpath"]   = "";

		// 1                   2               3            4
		// {searcher_preload, searcher_Lua, searcher_C, searcher_Croot, NULL};
		m_state["package"]["searchers"][3] = not_supported_lua_function("package.searcher C");
		m_state["package"]["searchers"][4] = not_supported_lua_function("package.searcher Croot");

		set_folder_for_lua_require(scripts_folder);
	}

	void lua_module::init_lua_api(folder& scripts_folder)
	{
		// https://blog.rubenwardy.com/2020/07/26/sol3-script-sandbox/
		// https://www.lua.org/manual/5.4/manual.html#pdf-require
		sandbox_lua_os_library();
		sandbox_lua_io_library();
		sandbox_lua_loads(scripts_folder);

		lua::log::bind(m_state);
		lua::globals::bind(m_state);
		lua::script::bind(m_state);
		lua::scr_function::bind(m_state);
		lua::scr_patch::bind(m_state);
		lua::native::bind(m_state);
		lua::memory::bind(m_state);
		lua::gui::bind(m_state);
		lua::network::bind(m_state);
		lua::command::bind(m_state);
		lua::tunables::bind(m_state);
		lua::locals::bind(m_state);
		lua::event::bind(m_state);
		lua::vector::bind(m_state);
		lua::global_table::bind(m_state);
		lua::imgui::bind(m_state, m_state.globals());
		lua::entities::bind(m_state);
		lua::self::bind(m_state);
		lua::stats::bind(m_state);
		lua::weapons::bind(m_state);
		lua::vehicles::bind(m_state);
	}

	void lua_module::load_and_call_script()
	{
		auto result = m_state.safe_script_file(m_module_path.string(), &sol::script_pass_on_error, sol::load_mode::text);

		if (!result.valid())
		{
			LOG(FATAL) << m_module_name << " failed to load: " << result.get<sol::error>().what();
			Logger::FlushQueue();
		}
		else
		{
			LOG(INFO) << "Loaded " << m_module_name;
		}
	}

	void lua_module::tick_scripts()
	{
		std::lock_guard guard(m_registered_scripts_mutex);

		const auto script_count = m_registered_scripts.size();
		for (size_t i = 0; i < script_count; i++)
		{
			const auto script = m_registered_scripts[i].get();

			if (script->is_enabled())
			{
				script->tick();
			}
		}
	}

	void lua_module::cleanup_done_scripts()
	{
		std::lock_guard guard(m_registered_scripts_mutex);

		std::erase_if(m_registered_scripts, [](auto& script) {
			return script->is_done();
		});
	}

	sol::object lua_module::to_lua(const lua::memory::runtime_func_t::parameters_t* params, const uint8_t i, const std::vector<lua::memory::type_info_t>& param_types)
	{
		if (param_types[i] == lua::memory::type_info_t::none_)
		{
			return sol::nil;
		}
		else if (param_types[i] == lua::memory::type_info_t::ptr_)
		{
			return sol::make_object(m_state, lua::memory::pointer(params->get<uintptr_t>(i)));
		}
		else
		{
			return sol::make_object(m_state, lua::memory::value_wrapper_t(params->get_arg_ptr(i), param_types[i]));
		}

		return sol::nil;
	}

	sol::object lua_module::to_lua(lua::memory::runtime_func_t::return_value_t* return_value, const lua::memory::type_info_t return_value_type)
	{
		if (return_value_type == lua::memory::type_info_t::none_)
		{
			return sol::nil;
		}
		else if (return_value_type == lua::memory::type_info_t::ptr_)
		{
			return sol::make_object(m_state, lua::memory::pointer((uintptr_t)return_value->get()));
		}
		else
		{
			return sol::make_object(m_state, lua::memory::value_wrapper_t((char*)return_value->get(), return_value_type));
		}

		return sol::nil;
	}
}
