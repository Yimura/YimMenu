#include "hotkey_service.hpp"

#include "fiber_pool.hpp"
#include "gui.hpp"
#include "network/ChatData.hpp"
#include "pointers.hpp"
#include "renderer/renderer.hpp"

#include <script/GtaThread.hpp>

namespace big
{
	hotkey_service::hotkey_service()
	{
		// ordered alphabetically to more easily see if a certain hotkey is present
		register_hotkey("beastjump", g.settings.hotkeys.beastjump, "beastjump"_J);
		register_hotkey("bringpv", g.settings.hotkeys.bringvehicle, "bringpv"_J);
		register_hotkey("clearwantedself", g.settings.hotkeys.clear_wanted, "clearwantedself"_J);
		register_hotkey("cmdexecutor", g.settings.hotkeys.cmd_excecutor, "cmdexecutor"_J);
		register_hotkey("fastquit", g.settings.hotkeys.fast_quit, "fastquit"_J);
		register_hotkey("fastrun", g.settings.hotkeys.superrun, "fastrun"_J);
		register_hotkey("fillammo", g.settings.hotkeys.fill_ammo, "fillammo"_J);
		register_hotkey("fillsnacks", g.settings.hotkeys.fill_inventory, "fillsnacks"_J);
		register_hotkey("freecam", g.settings.hotkeys.freecam, "freecam"_J);
		register_hotkey("heal", g.settings.hotkeys.heal, "heal"_J);
		register_hotkey("invis", g.settings.hotkeys.invis, "invis"_J);
		register_hotkey("invisveh", g.settings.hotkeys.invisveh, "invisveh"_J);
		register_hotkey("localinvisveh", g.settings.hotkeys.localinvisveh, "localinvisveh"_J);
		register_hotkey("noclip", g.settings.hotkeys.noclip, "noclip"_J);
		register_hotkey("objective", g.settings.hotkeys.teleport_objective, "objectivetp"_J);
		register_hotkey("pvtp", g.settings.hotkeys.teleport_pv, "pvtp"_J);
		register_hotkey("passive", g.settings.hotkeys.passive, "passive"_J);
		register_hotkey("repairpv", g.settings.hotkeys.repairpv, "repairpv"_J);
		register_hotkey("skipcutscene", g.settings.hotkeys.skip_cutscene, "skipcutscene"_J);
		register_hotkey("superjump", g.settings.hotkeys.superjump, "superjump"_J);
		register_hotkey("vehiclecontroller", g.settings.hotkeys.open_vehicle_controller, "vehiclecontrol"_J);
		register_hotkey("vehiclefly", g.settings.hotkeys.vehicle_flymode, "vehiclefly"_J);
		register_hotkey("waypoint", g.settings.hotkeys.teleport_waypoint, "waypointtp"_J);
		register_hotkey("highlighttp", g.settings.hotkeys.teleport_selected, "highlighttp"_J);

		g_renderer.add_wndproc_callback([this](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
			wndproc(static_cast<eKeyState>(msg), wparam);
		});

		g_hotkey_service = this;
	}

	hotkey_service::~hotkey_service()
	{
		g_hotkey_service = nullptr;
	}

	void hotkey_service::register_hotkey(const std::string_view name, key_t key, rage::joaat_t command_hash, eKeyState state, std::optional<std::chrono::high_resolution_clock::duration> cooldown)
	{
		m_hotkeys[state == eKeyState::RELEASE].emplace(key, hotkey(rage::joaat(name), key, command_hash, cooldown));
	}

	bool hotkey_service::update_hotkey(const std::string_view name, const key_t key)
	{
		static auto update_hotkey_map = [](hotkey_map& hotkey_map, rage::joaat_t name_hash, key_t new_key) -> bool {
			bool processed = false;

			for (auto it = hotkey_map.begin(); it != hotkey_map.end(); ++it)
			{
				auto hotkey = it->second;
				if (hotkey.name_hash() != name_hash)
					continue;

				hotkey_map.erase(it);
				hotkey.set_key(new_key);
				hotkey_map.emplace(new_key, hotkey);

				processed = true;
			}
			return processed;
		};

		const auto name_hash = rage::joaat(name);
		return update_hotkey_map(m_hotkeys[1], name_hash, key)  // released
		    && update_hotkey_map(m_hotkeys[0], name_hash, key); // down
	}

	void hotkey_service::wndproc(eKeyState state, key_t key)
	{
		if (const auto chat_data = *g_pointers->m_gta.m_chat_data; chat_data && (chat_data->m_chat_open || chat_data->m_timer_two))
			return;

		//command executer is opened
		if (g.cmd_executor.enabled)
			return;

		bool is_using_cellphone = false;
		for (auto script : *g_pointers->m_gta.m_script_threads)
		{
			if (script && script->m_script_hash == "cellphone_flashhand"_J)
			{
				is_using_cellphone = script->m_context.m_state == rage::eThreadState::idle;
			}
		}

		if (g_gui->is_open() || *g_pointers->m_gta.m_is_social_club_overlay_active || is_using_cellphone
		    || g.settings.hotkeys.is_mp_chat_active)
			return;

		switch (state)
		{
		case WM_LBUTTONUP:
			state = eKeyState::RELEASE;
			key   = VK_LBUTTON;
			break;
		case WM_RBUTTONUP:
			state = eKeyState::RELEASE;
			key   = VK_RBUTTON;
			break;
		case WM_MBUTTONUP:
			state = eKeyState::RELEASE;
			key   = VK_MBUTTON;
			break;
		case WM_XBUTTONUP:
			state = eKeyState::RELEASE;
			key   = GET_XBUTTON_WPARAM(key) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2;
			break;
		case WM_LBUTTONDOWN:
			state = eKeyState::DOWN;
			key   = VK_LBUTTON;
			break;
		case WM_RBUTTONDOWN:
			state = eKeyState::DOWN;
			key   = VK_RBUTTON;
			break;
		case WM_MBUTTONDOWN:
			state = eKeyState::DOWN;
			key   = VK_MBUTTON;
			break;
		case WM_XBUTTONDOWN:
			state = eKeyState::DOWN;
			key   = GET_XBUTTON_WPARAM(key) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2;
			break;
		}

		if (state == eKeyState::RELEASE || state == eKeyState::DOWN)
		{
			auto& hotkey_map = m_hotkeys[state == eKeyState::RELEASE];
			for (auto [ it, end ] = hotkey_map.equal_range(key); it != end; ++it)
			{
				if (auto& hotkey = it->second; hotkey.can_exec())
				{
					g_fiber_pool->queue_job([&hotkey] {
						hotkey.exec();
					});
				}
			}
		}
	}
}
