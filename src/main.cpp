#include "backend/backend.hpp"
#include "byte_patch_manager.hpp"
#include "common.hpp"
#include "fiber_pool.hpp"
#include "gui.hpp"
#include "hooking/hooking.hpp"
#include "http_client/http_client.hpp"
#include "logger/exception_handler.hpp"
#include "lua/lua_manager.hpp"
#include "native_hooks/native_hooks.hpp"
#include "pointers.hpp"
#include "rage/gameSkeleton.hpp"
#include "renderer/renderer.hpp"
#include "script_mgr.hpp"
#include "services/api/api_service.hpp"
#include "services/context_menu/context_menu_service.hpp"
#include "services/custom_text/custom_text_service.hpp"
#include "services/gta_data/gta_data_service.hpp"
#include "services/gui/gui_service.hpp"
#include "services/hotkey/hotkey_service.hpp"
#include "services/matchmaking/matchmaking_service.hpp"
#include "services/mobile/mobile_service.hpp"
#include "services/model_preview/model_preview_service.hpp"
#include "services/notifications/notification_service.hpp"
#include "services/pickups/pickup_service.hpp"
#include "services/player_database/player_database_service.hpp"
#include "services/players/player_service.hpp"
#include "services/script_connection/script_connection_service.hpp"
#include "services/script_patcher/script_patcher_service.hpp"
#include "services/tunables/tunables_service.hpp"
#include "services/vehicle/handling_service.hpp"
#include "services/vehicle/xml_vehicles_service.hpp"
#include "services/xml_maps/xml_map_service.hpp"
#include "thread_pool.hpp"
#include "util/is_proton.hpp"
#include "version.hpp"

namespace big
{
	bool disable_anticheat_skeleton()
	{
		bool patched = false;
		for (rage::game_skeleton_update_mode* mode = g_pointers->m_gta.m_game_skeleton->m_update_modes; mode; mode = mode->m_next)
		{
			for (rage::game_skeleton_update_base* update_node = mode->m_head; update_node; update_node = update_node->m_next)
			{
				if (update_node->m_hash != "Common Main"_J)
					continue;
				rage::game_skeleton_update_group* group = reinterpret_cast<rage::game_skeleton_update_group*>(update_node);
				for (rage::game_skeleton_update_base* group_child_node = group->m_head; group_child_node;
				     group_child_node                                  = group_child_node->m_next)
				{
					// TamperActions is a leftover from the old AC, but still useful to block anyway
					if (group_child_node->m_hash != 0xA0F39FB6 && group_child_node->m_hash != "TamperActions"_J)
						continue;
					patched = true;
					//LOG(INFO) << "Patching problematic skeleton update";
					reinterpret_cast<rage::game_skeleton_update_element*>(group_child_node)->m_function =
					    g_pointers->m_gta.m_nullsub;
				}
				break;
			}
		}

		for (rage::skeleton_data& i : g_pointers->m_gta.m_game_skeleton->m_sys_data)
		{
			if (i.m_hash != 0xA0F39FB6 && i.m_hash != "TamperActions"_J)
				continue;
			i.m_init_func     = reinterpret_cast<uint64_t>(g_pointers->m_gta.m_nullsub);
			i.m_shutdown_func = reinterpret_cast<uint64_t>(g_pointers->m_gta.m_nullsub);
		}
		return patched;
	}

	std::string ReadRegistryKeySZ(HKEY hKeyParent, std::string subkey, std::string valueName)
	{
		HKEY hKey;
		char value[1024];
		DWORD value_length = 1024;
		LONG ret           = RegOpenKeyEx(hKeyParent, subkey.c_str(), 0, KEY_READ, &hKey);
		if (ret != ERROR_SUCCESS)
		{
			LOG(INFO) << "Unable to read registry key " << subkey;
			return "";
		}
		ret = RegQueryValueEx(hKey, valueName.c_str(), NULL, NULL, (LPBYTE)&value, &value_length);
		RegCloseKey(hKey);
		if (ret != ERROR_SUCCESS)
		{
			LOG(INFO) << "Unable to read registry key " << valueName;
			return "";
		}
		return std::string(value);
	}

	DWORD ReadRegistryKeyDWORD(HKEY hKeyParent, std::string subkey, std::string valueName)
	{
		HKEY hKey;
		DWORD value;
		DWORD value_length = sizeof(DWORD);
		LONG ret           = RegOpenKeyEx(hKeyParent, subkey.c_str(), 0, KEY_READ, &hKey);
		if (ret != ERROR_SUCCESS)
		{
			LOG(INFO) << "Unable to read registry key " << subkey;
			return NULL;
		}
		ret = RegQueryValueEx(hKey, valueName.c_str(), NULL, NULL, (LPBYTE)&value, &value_length);
		RegCloseKey(hKey);
		if (ret != ERROR_SUCCESS)
		{
			LOG(INFO) << "Unable to read registry key " << valueName;
			return NULL;
		}
		return value;
	}

	std::unique_ptr<char[]> GetWindowsVersion()
	{
		typedef LPWSTR(WINAPI * BFS)(LPCWSTR);
		LPWSTR UTF16   = BFS(GetProcAddress(LoadLibrary("winbrand.dll"), "BrandingFormatString"))(L"%WINDOWS_LONG%");
		int BufferSize = WideCharToMultiByte(CP_UTF8, 0, UTF16, -1, NULL, 0, NULL, NULL);
		std::unique_ptr<char[]> UTF8(new char[BufferSize]);
		WideCharToMultiByte(CP_UTF8, 0, UTF16, -1, UTF8.get(), BufferSize, NULL, NULL);
		// BrandingFormatString requires a GlobalFree.
		GlobalFree(UTF16);
		return UTF8;
	}
}

BOOL APIENTRY DllMain(HMODULE hmod, DWORD reason, PVOID)
{
	using namespace big;
	if (reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hmod);
		g_hmodule     = hmod;
		g_main_thread = CreateThread(
		    nullptr,
		    0,
		    [](PVOID) -> DWORD {
			    auto handler = exception_handler();
			    std::srand(std::chrono::system_clock::now().time_since_epoch().count());

			    while (!FindWindow("grcWindow", nullptr))
				    std::this_thread::sleep_for(100ms);

			    std::filesystem::path base_dir = std::getenv("appdata");
			    base_dir /= "YimMenu";
			    g_file_manager.init(base_dir);

			    g.init(g_file_manager.get_project_file("./settings.json"));
			    g_log.initialize("YimMenu", g_file_manager.get_project_file("./cout.log"), g.debug.external_console);
			    LOG(INFO) << "Settings Loaded and logger initialized.";

			    LOG(INFO) << "Yim's Menu Initializing";
			    LOGF(INFO, "Git Info\n\tBranch:\t{}\n\tHash:\t{}\n\tDate:\t{}", version::GIT_BRANCH, version::GIT_SHA1, version::GIT_DATE);

			    // more tech debt, YAY!
			    if (is_proton())
			    {
				    LOG(INFO) << "Running on proton!";
			    }
			    else
			    {
				    auto display_version = ReadRegistryKeySZ(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "DisplayVersion");
				    auto current_build = ReadRegistryKeySZ(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "CurrentBuild");
				    auto UBR = ReadRegistryKeyDWORD(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "UBR");
				    LOG(INFO) << GetWindowsVersion() << " Version " << display_version << " (OS Build " << current_build << "." << UBR << ")";
			    }

#ifndef NDEBUG
			    LOG(WARNING) << "Debug Build. Switch to RelWithDebInfo or Release Build for a more stable experience";
#endif

			    auto thread_pool_instance = std::make_unique<thread_pool>();
			    LOG(INFO) << "Thread pool initialized.";

			    auto pointers_instance = std::make_unique<pointers>();
			    LOG(INFO) << "Pointers initialized.";

			    while (!disable_anticheat_skeleton())
			    {
				    LOG(WARNING) << "Failed patching anticheat gameskeleton (injected too early?). Waiting 100ms and trying again";
				    std::this_thread::sleep_for(100ms);
			    }
			    LOG(INFO) << "Disabled anticheat gameskeleton.";

			    auto byte_patch_manager_instance = std::make_unique<byte_patch_manager>();
			    LOG(INFO) << "Byte Patch Manager initialized.";

			    g_renderer.init();
			    LOG(INFO) << "Renderer initialized.";
			    auto gui_instance = std::make_unique<gui>();

			    auto fiber_pool_instance = std::make_unique<fiber_pool>(11);
			    LOG(INFO) << "Fiber pool initialized.";

			    g_http_client.init(g_file_manager.get_project_file("./proxy_settings.json"));
			    LOG(INFO) << "HTTP Client initialized.";

			    g_translation_service.init();
			    LOG(INFO) << "Translation Service initialized.";

			    auto hooking_instance = std::make_unique<hooking>();
			    LOG(INFO) << "Hooking initialized.";

			    g_gta_data_service.init();

			    auto context_menu_service_instance      = std::make_unique<context_menu_service>();
			    auto custom_text_service_instance       = std::make_unique<custom_text_service>();
			    auto mobile_service_instance            = std::make_unique<mobile_service>();
			    auto pickup_service_instance            = std::make_unique<pickup_service>();
			    auto player_service_instance            = std::make_unique<player_service>();
			    auto model_preview_service_instance     = std::make_unique<model_preview_service>();
			    auto handling_service_instance          = std::make_unique<handling_service>();
			    auto gui_service_instance               = std::make_unique<gui_service>();
			    auto script_patcher_service_instance    = std::make_unique<script_patcher_service>();
			    auto player_database_service_instance   = std::make_unique<player_database_service>();
			    auto hotkey_service_instance            = std::make_unique<hotkey_service>();
			    auto matchmaking_service_instance       = std::make_unique<matchmaking_service>();
			    auto api_service_instance               = std::make_unique<api_service>();
			    auto tunables_service_instance          = std::make_unique<tunables_service>();
			    auto script_connection_service_instance = std::make_unique<script_connection_service>();
			    auto xml_vehicles_service_instance      = std::make_unique<xml_vehicles_service>();
			    auto xml_maps_service_instance          = std::make_unique<xml_map_service>();
			    LOG(INFO) << "Registered service instances...";

			    g_notification_service.initialise();
			    LOG(INFO) << "Finished initialising services.";

			    g_script_mgr.add_script(std::make_unique<script>(&gui::script_func, "GUI", false));

			    g_script_mgr.add_script(std::make_unique<script>(&backend::loop, "Backend Loop", false));
			    g_script_mgr.add_script(std::make_unique<script>(&backend::self_loop, "Self"));
			    g_script_mgr.add_script(std::make_unique<script>(&backend::weapons_loop, "Weapon"));
			    g_script_mgr.add_script(std::make_unique<script>(&backend::vehicles_loop, "Vehicle"));
			    g_script_mgr.add_script(std::make_unique<script>(&backend::misc_loop, "Miscellaneous"));
			    g_script_mgr.add_script(std::make_unique<script>(&backend::remote_loop, "Remote"));
			    g_script_mgr.add_script(std::make_unique<script>(&backend::rainbowpaint_loop, "Rainbow Paint"));
			    g_script_mgr.add_script(std::make_unique<script>(&backend::disable_control_action_loop, "Disable Controls"));
			    g_script_mgr.add_script(std::make_unique<script>(&backend::world_loop, "World"));
			    g_script_mgr.add_script(std::make_unique<script>(&backend::orbital_drone, "Orbital Drone"));
			    g_script_mgr.add_script(std::make_unique<script>(&backend::vehicle_control, "Vehicle Control"));
			    g_script_mgr.add_script(std::make_unique<script>(&context_menu_service::context_menu, "Context Menu"));
			    g_script_mgr.add_script(std::make_unique<script>(&backend::tunables_script, "Tunables"));
			    g_script_mgr.add_script(std::make_unique<script>(&backend::squad_spawner, "Squad Spawner"));
			    g_script_mgr.add_script(std::make_unique<script>(&backend::ambient_animations_loop, "Ambient Animations"));

			    LOG(INFO) << "Scripts registered.";

			    g_hooking->enable();
			    LOG(INFO) << "Hooking enabled.";

			    auto native_hooks_instance = std::make_unique<native_hooks>();
			    LOG(INFO) << "Dynamic native hooker initialized.";

			    auto lua_manager_instance =
			        std::make_unique<lua_manager>(g_file_manager.get_project_folder("scripts"), g_file_manager.get_project_folder("scripts_config"));
			    LOG(INFO) << "Lua manager initialized.";

			    g_running = true;

			    while (g_running)
			    {
				    g.attempt_save();

				    std::this_thread::sleep_for(500ms);
			    }

			    g_script_mgr.remove_all_scripts();
			    LOG(INFO) << "Scripts unregistered.";

			    lua_manager_instance.reset();
			    LOG(INFO) << "Lua manager uninitialized.";

			    g_hooking->disable();
			    LOG(INFO) << "Hooking disabled.";

			    native_hooks_instance.reset();
			    LOG(INFO) << "Dynamic native hooker uninitialized.";

			    // Make sure that all threads created don't have any blocking loops
			    // otherwise make sure that they have stopped executing
			    thread_pool_instance->destroy();
			    LOG(INFO) << "Destroyed thread pool.";

			    script_connection_service_instance.reset();
			    LOG(INFO) << "Script Connection Service reset.";
			    tunables_service_instance.reset();
			    LOG(INFO) << "Tunables Service reset.";
			    hotkey_service_instance.reset();
			    LOG(INFO) << "Hotkey Service reset.";
			    matchmaking_service_instance.reset();
			    LOG(INFO) << "Matchmaking Service reset.";
			    player_database_service_instance.reset();
			    LOG(INFO) << "Player Database Service reset.";
			    api_service_instance.reset();
			    LOG(INFO) << "API Service reset.";
			    script_patcher_service_instance.reset();
			    LOG(INFO) << "Script Patcher Service reset.";
			    gui_service_instance.reset();
			    LOG(INFO) << "Gui Service reset.";
			    handling_service_instance.reset();
			    LOG(INFO) << "Vehicle Service reset.";
			    model_preview_service_instance.reset();
			    LOG(INFO) << "Model Preview Service reset.";
			    mobile_service_instance.reset();
			    LOG(INFO) << "Mobile Service reset.";
			    player_service_instance.reset();
			    LOG(INFO) << "Player Service reset.";
			    pickup_service_instance.reset();
			    LOG(INFO) << "Pickup Service reset.";
			    custom_text_service_instance.reset();
			    LOG(INFO) << "Custom Text Service reset.";
			    context_menu_service_instance.reset();
			    LOG(INFO) << "Context Service reset.";
			    xml_vehicles_service_instance.reset();
			    LOG(INFO) << "Xml Vehicles Service reset.";
			    LOG(INFO) << "Services uninitialized.";

			    hooking_instance.reset();
			    LOG(INFO) << "Hooking uninitialized.";

			    fiber_pool_instance.reset();
			    LOG(INFO) << "Fiber pool uninitialized.";

			    g_renderer.destroy();
			    LOG(INFO) << "Renderer uninitialized.";

			    byte_patch_manager_instance.reset();
			    LOG(INFO) << "Byte Patch Manager uninitialized.";

			    pointers_instance.reset();
			    LOG(INFO) << "Pointers uninitialized.";

			    thread_pool_instance.reset();
			    LOG(INFO) << "Thread pool uninitialized.";

			    LOG(INFO) << "Farewell!";
			    g_log.destroy();

			    CloseHandle(g_main_thread);
			    FreeLibraryAndExitThread(g_hmodule, 0);
		    },
		    nullptr,
		    0,
		    &g_main_thread_id);
	}

	return true;
}
