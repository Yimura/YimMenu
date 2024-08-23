#include "gui.hpp"

#include "../../gui.hpp"

namespace lua::gui
{
	static void add_independent_element(lua_State* state, std::unique_ptr<lua::gui::gui_element> element)
	{
		big::lua_module* module = sol::state_view(state)["!this"];

		module->m_independent_gui.push_back(std::move(element));
	}

	static void add_always_draw_element(lua_State* state, std::unique_ptr<lua::gui::gui_element> element)
	{
		big::lua_module* module = sol::state_view(state)["!this"];

		module->m_always_draw_gui.push_back(std::move(element));
	}

	static void add_element(lua_State* state, uint32_t hash, std::unique_ptr<lua::gui::gui_element> element)
	{
		big::lua_module* module = sol::state_view(state)["!this"];

		module->m_gui[hash].push_back(std::move(element));
	}

	big::tabs tab::id() const
	{
		return m_id;
	}

	rage::joaat_t tab::hash() const
	{
		return m_tab_hash;
	}

	bool tab::check_if_existing_tab_and_fill_id(const std::map<big::tabs, big::navigation_struct>& nav)
	{
		for (const auto& nav_item : nav)
		{
			if (nav_item.second.hash == m_tab_hash)
			{
				m_id = nav_item.first;
				return true;
			}

			if (check_if_existing_tab_and_fill_id(nav_item.second.sub_nav))
			{
				return true;
			}
		}

		return false;
	}

	static void add_to_existing_tab(std::map<big::tabs, big::navigation_struct>& nav, const rage::joaat_t existing_tab_hash, const std::pair<big::tabs, big::navigation_struct>& new_tab, const sol::this_state& state)
	{
		for (auto& nav_item : nav)
		{
			if (nav_item.second.hash == existing_tab_hash)
			{
				big::lua_module* module = sol::state_view(state)["!this"];


				module->m_tab_to_sub_tabs[nav_item.first].push_back(new_tab.first);


				nav_item.second.sub_nav.emplace(new_tab);
				return;
			}

			add_to_existing_tab(nav_item.second.sub_nav, existing_tab_hash, new_tab, state);
		}
	}

	std::pair<big::tabs, big::navigation_struct> tab::make_tab_nav(const std::string& name, const rage::joaat_t tab_hash, const sol::this_state& state)
	{
		static size_t custom_tab_count = size_t(big::tabs::RUNTIME_CUSTOM);
		m_id                           = big::tabs(custom_tab_count);

		custom_tab_count++;

		big::navigation_struct new_navigation_struct{};
		strcpy(new_navigation_struct.name, name.c_str());
		new_navigation_struct.hash = tab_hash;

		return std::make_pair(m_id, new_navigation_struct);
	}

	tab::tab(const std::string& name, const sol::this_state& state) :
	    m_tab_hash(rage::joaat(name))
	{
		auto& nav = big::g_gui_service->get_navigation();

		if (check_if_existing_tab_and_fill_id(nav))
		{
			return;
		}

		// add top tab
		nav.emplace(make_tab_nav(name, m_tab_hash, state));

		big::lua_module* module = sol::state_view(state)["!this"];
		module->m_owned_tabs.push_back(id());
	}

	tab::tab(const std::string& name, const rage::joaat_t parent_tab_hash, const sol::this_state& state) :
	    m_tab_hash(rage::joaat(name))
	{
		auto& nav = big::g_gui_service->get_navigation();

		if (check_if_existing_tab_and_fill_id(nav))
		{
			return;
		}

		const auto sub_tab = make_tab_nav(name, m_tab_hash, state);
		add_to_existing_tab(nav, parent_tab_hash, sub_tab, state);

		big::lua_module* module = sol::state_view(state)["!this"];
		module->m_owned_tabs.push_back(id());
	}

	bool tab::is_selected(sol::this_state state)
	{
		return big::g_gui_service->get_selected()->hash == m_tab_hash;
	}

	void tab::clear(sol::this_state state)
	{
		big::lua_module* module = sol::state_view(state)["!this"];

		module->m_gui[m_tab_hash].clear();

		for (auto sub_tab : module->m_tab_to_sub_tabs[id()])
		{
			for (const auto owned_tab : module->m_owned_tabs)
			{
				if (sub_tab == owned_tab)
				{
					big::g_gui_service->remove_from_nav(sub_tab);
				}
			}
		}
	}

	tab tab::add_tab(const std::string& name, sol::this_state state)
	{
		const auto sub_tab = tab(name, m_tab_hash, state);

		return sub_tab;
	}

	lua::gui::button* tab::add_button(const std::string& name, sol::protected_function callback, sol::this_state state)
	{
		auto element = std::make_unique<lua::gui::button>(name, callback);
		auto el_ptr  = element.get();
		add_element(state, m_tab_hash, std::move(element));
		return el_ptr;
	}

	lua::gui::text* tab::add_text(const std::string& name, sol::this_state state)
	{
		auto element = std::make_unique<lua::gui::text>(name);
		auto el_ptr  = element.get();
		add_element(state, m_tab_hash, std::move(element));
		return el_ptr;
	}

	lua::gui::checkbox* tab::add_checkbox(const std::string& name, sol::this_state state)
	{
		auto element = std::make_unique<lua::gui::checkbox>(name);
		auto el_ptr  = element.get();
		add_element(state, m_tab_hash, std::move(element));
		return el_ptr;
	}

	lua::gui::sameline* tab::add_sameline(sol::this_state state)
	{
		auto element = std::make_unique<lua::gui::sameline>();
		auto el_ptr  = element.get();
		add_element(state, m_tab_hash, std::move(element));
		return el_ptr;
	}

	lua::gui::separator* tab::add_separator(sol::this_state state)
	{
		auto element = std::make_unique<lua::gui::separator>();
		auto el_ptr  = element.get();
		add_element(state, m_tab_hash, std::move(element));
		return el_ptr;
	}

	lua::gui::input_int* tab::add_input_int(const std::string& name, sol::this_state state)
	{
		auto element = std::make_unique<lua::gui::input_int>(name);
		auto el_ptr  = element.get();
		add_element(state, m_tab_hash, std::move(element));
		return el_ptr;
	}

	lua::gui::input_float* tab::add_input_float(const std::string& name, sol::this_state state)
	{
		auto element = std::make_unique<lua::gui::input_float>(name);
		auto el_ptr  = element.get();
		add_element(state, m_tab_hash, std::move(element));
		return el_ptr;
	}

	lua::gui::input_string* tab::add_input_string(const std::string& name, sol::this_state state)
	{
		auto element = std::make_unique<lua::gui::input_string>(name);
		auto el_ptr  = element.get();
		add_element(state, m_tab_hash, std::move(element));
		return el_ptr;
	}

	lua::gui::raw_imgui_callback* tab::add_imgui(sol::protected_function imgui_rendering, sol::this_state state)
	{
		auto element = std::make_unique<lua::gui::raw_imgui_callback>(imgui_rendering);
		auto el_ptr  = element.get();
		add_element(state, m_tab_hash, std::move(element));
		return el_ptr;
	}

	// Lua API: Table
	// Name: gui
	// Table containing functions for modifying the menu GUI.

	// Lua API: Function
	// Table: gui
	// Name: get_tab
	// Param: tab_name: string: Name of the tab to get.
	// Returns: tab: A tab instance which corresponds to the tab in the GUI.
	static tab get_tab(const std::string& tab_name, sol::this_state state)
	{
		return tab(tab_name, state);
	}

	// Lua API: Function
	// Table: gui
	// Name: add_tab
	// Param: tab_name: string: Name of the tab to add.
	// Returns: tab: A tab instance which corresponds to the new tab in the GUI.
	static tab add_tab(const std::string& tab_name, sol::this_state state)
	{
		const auto new_tab = tab(tab_name, state);

		return new_tab;
	}

	// Lua API: Function
	// Table: gui
	// Name: show_success
	// Param: title: string
	// Param: message: string
	// Shows a success to the user with the given title and message.
	static void show_success(const std::string& title, const std::string& message)
	{
		big::g_notification_service.push_success(title, message);
	}

	// Lua API: Function
	// Table: gui
	// Name: show_message
	// Param: title: string
	// Param: message: string
	// Shows a message to the user with the given title and message.
	static void show_message(const std::string& title, const std::string& message)
	{
		big::g_notification_service.push(title, message);
	}

	// Lua API: Function
	// Table: gui
	// Name: show_warning
	// Param: title: string
	// Param: message: string
	// Shows a warning to the user with the given title and message.
	static void show_warning(const std::string& title, const std::string& message)
	{
		big::g_notification_service.push_warning(title, message);
	}

	// Lua API: Function
	// Table: gui
	// Name: show_error
	// Param: title: string
	// Param: message: string
	// Shows an error to the user with the given title and message.
	static void show_error(const std::string& title, const std::string& message)
	{
		big::g_notification_service.push_error(title, message);
	}

	// Lua API: Function
	// Table: gui
	// Name: is_open
	// Returns: bool: Returns true if the GUI is open.
	static bool is_open()
	{
		return big::g_gui->is_open();
	}

	// Lua API: Function
	// Table: gui
	// Name: toggle
	// Param: toggle: boolean
	// Opens and closes the gui.
	static void toggle(bool toggle)
	{
		big::g_gui->toggle(toggle);
	}


	// Lua API: Function
	// Table: gui
	// Name: mouse_override
	// Returns: bool: Returns true if the mouse is overridden.
	static bool mouse_override()
	{
		return big::g_gui->mouse_override();
	}

	// Lua API: Function
	// Table: gui
	// Name: override_mouse
	// Param: override: boolean
	static void override_mouse(bool override)
	{
		big::g_gui->override_mouse(override);
	}

	// Lua API: Function
	// Table: gui
	// Name: add_imgui
	// Param: imgui_rendering: function: Function that will be called every rendering frame, you can call ImGui functions in it, please check the ImGui.md documentation file for more info.
	// Registers a function that will be called every rendering frame, you can call ImGui functions in it, please check the ImGui.md documentation file for more info.
	// **Example Usage:**
	// ```lua
	// gui.add_imgui(function()
	//   if ImGui.Begin("My Custom Window") then
	//       if ImGui.Button("Label") then
	//         script.run_in_fiber(function(script)
	//           -- call natives in there
	//         end)
	//       end
	//
	//       ImGui.End()
	//   end
	// end)
	// ```
	static lua::gui::raw_imgui_callback* add_imgui(sol::protected_function imgui_rendering, sol::this_state state)
	{
		auto element = std::make_unique<lua::gui::raw_imgui_callback>(imgui_rendering);
		auto el_ptr  = element.get();
		add_independent_element(state, std::move(element));
		return el_ptr;
	}

	// Lua API: Function
	// Table: gui
	// Name: add_always_draw_imgui
	// Param: imgui_rendering: function: Function that will be called every rendering frame, you can call ImGui functions in it, please check the ImGui.md documentation file for more info.
	// Registers a function that will be called every rendering frame, you can call ImGui functions in it, please check the ImGui.md documentation file for more info. This function will be called even when the menu is closed.
	// **Example Usage:**
	// ```lua
	// gui.add_always_draw_imgui(function()
	//   if ImGui.Begin("My Custom Window") then
	//       if ImGui.Button("Label") then
	//         script.run_in_fiber(function(script)
	//           -- call natives in there
	//         end)
	//       end
	//
	//       ImGui.End()
	//   end
	// end)
	// ``
	static lua::gui::raw_imgui_callback* add_always_draw_imgui(sol::protected_function imgui_rendering, sol::this_state state)
	{
		auto element = std::make_unique<lua::gui::raw_imgui_callback>(imgui_rendering);
		auto el_ptr  = element.get();
		add_always_draw_element(state, std::move(element));
		return el_ptr;
	}

	void bind(sol::state& state)
	{
		auto ns                     = state["gui"].get_or_create<sol::table>();
		ns["get_tab"]               = get_tab;
		ns["add_tab"]               = add_tab;
		ns["show_success"]          = show_success;
		ns["show_message"]          = show_message;
		ns["show_warning"]          = show_warning;
		ns["show_error"]            = show_error;
		ns["is_open"]               = is_open;
		ns["toggle"]                = toggle;
		ns["mouse_override"]        = mouse_override;
		ns["override_mouse"]        = override_mouse;
		ns["add_imgui"]             = add_imgui;
		ns["add_always_draw_imgui"] = add_always_draw_imgui;

		auto button_ut        = ns.new_usertype<lua::gui::button>("button");
		button_ut["get_text"] = &lua::gui::button::get_text;
		button_ut["set_text"] = &lua::gui::button::set_text;

		auto text_ut        = ns.new_usertype<lua::gui::text>("text");
		text_ut["get_text"] = &lua::gui::text::get_text;
		text_ut["set_text"] = &lua::gui::text::set_text;
		text_ut["set_font"] = &lua::gui::text::set_font;

		auto checkbox_ut           = ns.new_usertype<lua::gui::checkbox>("checkbox");
		checkbox_ut["get_text"]    = &lua::gui::checkbox::get_text;
		checkbox_ut["set_text"]    = &lua::gui::checkbox::set_text;
		checkbox_ut["is_enabled"]  = &lua::gui::checkbox::is_enabled;
		checkbox_ut["set_enabled"] = &lua::gui::checkbox::set_enabled;

		ns.new_usertype<lua::gui::sameline>("sameline");
		ns.new_usertype<lua::gui::separator>("separator");

		auto input_int_ut         = ns.new_usertype<lua::gui::input_int>("input_int");
		input_int_ut["get_text"]  = &lua::gui::input_int::get_text;
		input_int_ut["set_text"]  = &lua::gui::input_int::set_text;
		input_int_ut["get_value"] = &lua::gui::input_int::get_value;
		input_int_ut["set_value"] = &lua::gui::input_int::set_value;

		auto input_float_ut         = ns.new_usertype<lua::gui::input_float>("input_float");
		input_float_ut["get_text"]  = &lua::gui::input_float::get_text;
		input_float_ut["set_text"]  = &lua::gui::input_float::set_text;
		input_float_ut["get_value"] = &lua::gui::input_float::get_value;
		input_float_ut["set_value"] = &lua::gui::input_float::set_value;

		auto input_string_ut         = ns.new_usertype<lua::gui::input_string>("input_string");
		input_string_ut["get_text"]  = &lua::gui::input_string::get_text;
		input_string_ut["set_text"]  = &lua::gui::input_string::set_text;
		input_string_ut["get_value"] = &lua::gui::input_string::get_value;
		input_string_ut["set_value"] = &lua::gui::input_string::set_value;

		auto tab_ut                = ns.new_usertype<tab>("tab");
		tab_ut["is_selected"]      = &tab::is_selected;
		tab_ut["clear"]            = &tab::clear;
		tab_ut["add_tab"]          = &tab::add_tab;
		tab_ut["add_button"]       = &tab::add_button;
		tab_ut["add_text"]         = &tab::add_text;
		tab_ut["add_checkbox"]     = &tab::add_checkbox;
		tab_ut["add_sameline"]     = &tab::add_sameline;
		tab_ut["add_separator"]    = &tab::add_separator;
		tab_ut["add_input_int"]    = &tab::add_input_int;
		tab_ut["add_input_float"]  = &tab::add_input_float;
		tab_ut["add_input_string"] = &tab::add_input_string;
		tab_ut["add_imgui"]        = &tab::add_imgui;
	}
}
