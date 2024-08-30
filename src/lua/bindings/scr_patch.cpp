#include "scr_patch.hpp"

#include "gta_util.hpp"
#include "lua/lua_module.hpp"
#include "lua/bindings/network.hpp" // For convert_sequence
#include "services/script_patcher/script_patcher_service.hpp"

namespace lua::scr_patch
{
	scr_patch::scr_patch(const std::string& script_name, const std::string& patch_name, const std::string& pattern, const int offset, sol::table patch_, sol::this_state state) :
	    m_script(rage::joaat(script_name)),
	    m_patch_name(patch_name),
	    m_pattern(pattern),
	    m_offset(offset),
	    m_patch(patch_),
	    m_enable(true)
	{
		auto patch = convert_sequence<uint8_t>(m_patch);

		big::g_script_patcher_service->add_patch({m_script, m_patch_name, ::memory::pattern(m_pattern), m_offset, patch, &m_enable});
		if (auto program = big::gta_util::find_script_program(m_script))
			big::g_script_patcher_service->on_script_load(program);

		big::lua_module* module = sol::state_view(state)["!this"];
		module->m_registered_script_patches.push_back(std::make_unique<scr_patch>(*this));
	}

	scr_patch::~scr_patch()
	{
		big::g_script_patcher_service->remove_patch(m_patch_name);
	}

	void scr_patch::enable()
	{
		if (!m_enable)
		{
			m_enable = true;
			big::g_script_patcher_service->update();
		}
	}

	void scr_patch::disable()
	{
		if (m_enable)
		{
			m_enable = false;
			big::g_script_patcher_service->update();
		}
	}

	void bind(sol::state& state)
	{
		auto scr_patch_ut = state.new_usertype<scr_patch>("scr_patch", sol::constructors<scr_patch(const std::string&, const std::string&, const std::string&, const int, sol::table, sol::this_state)>());

		scr_patch_ut["enable_patch"]  = &scr_patch::enable;
		scr_patch_ut["disable_patch"] = &scr_patch::disable;
	}
}