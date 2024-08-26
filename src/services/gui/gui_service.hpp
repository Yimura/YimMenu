#pragma once
#include "views/view.hpp"

namespace big
{
	// Lua API: Tabs
	enum class tabs
	{
		NONE,

		SELF,
		WEAPONS,
		WEAPONS_AMMUNATION,
		TELEPORT,
		CUSTOM_TELEPORT,
		MOBILE,
		OUTFIT_EDITOR,
		OUTFIT_SLOTS,
		ANIMATIONS,

		VEHICLE,
		HANDLING,
		HANDLING_SEARCH,
		HANDLING_SAVED_PROFILE,
		HANDLING_MY_PROFILES,
		HANDLING_CURRENT_PROFILE,
		LSC,
		SPAWN_VEHICLE,
		FUN_VEHICLE,

		WORLD,
		SPAWN_PED,
		SQUAD_SPAWNER,
		CREATOR,
		TRAIN,
		BLACKHOLE,
		MODEL_SWAPPER,
		VFX,
		XML_MAPS,

		NETWORK,
		CHAT,
		NETWORK_CONTROLS,
		MISSIONS,
		SPOOFING,
		PLAYER_DATABASE,
		SESSION_BROWSER,
		STAT_EDITOR,

		SETTINGS,
		LUA_SCRIPTS,
		CONTEXT_MENU_SETTINGS,
		ESP_SETTINGS,
		GTA_CACHE_SETTINGS,
		GUI_SETTINGS,
		HOTKEY_SETTINGS,
		REACTION_SETTINGS,
		PROTECTION_SETTINGS,
		TRANSLATION_SETTINGS,
		PROXY_SETTINGS,
		DEBUG,

		PLAYER,

		// Added at runtime by things like lua scripts.
		RUNTIME_CUSTOM
	};

	struct navigation_struct
	{
		char name[80]              = "";
		std::function<void()> func = nullptr;
		std::map<tabs, navigation_struct> sub_nav{};
		rage::joaat_t hash = rage::joaat(name);
	};

// Used for constructing translation key for tabs
#define TAB_DECL_INTERNAL(prefix, tab) \
	tabs::tab,                         \
	{                                  \
		#prefix #tab
#define TAB_DECL(tab) TAB_DECL_INTERNAL(GUI_TAB_, tab)

	class gui_service final
	{
		std::vector<tabs> current_tab{};
		bool switched_view = true;

		// clang-format off
		std::map<tabs, navigation_struct> nav = {
		    {
				TAB_DECL(SELF),
		            view::self,
		            {
						{
							TAB_DECL(WEAPONS),
								view::weapons,
								{
									{TAB_DECL(WEAPONS_AMMUNATION), view::ammunation}},
								},
							},
						},
						{TAB_DECL(MOBILE), view::mobile}},
						{TAB_DECL(TELEPORT), view::teleport,
						{
							{TAB_DECL(CUSTOM_TELEPORT), view::custom_teleport}},
						}}},
						{TAB_DECL(OUTFIT_EDITOR), view::outfit_editor}},
		                {TAB_DECL(OUTFIT_SLOTS), view::outfit_slots}},
						{TAB_DECL(ANIMATIONS), view::animations}},
		            },
		        },
		    },
		    {
		        TAB_DECL(VEHICLE),
		            view::vehicle,
		            {
		                {
		                    TAB_DECL(HANDLING),
								view::handling_current_profile,
		                        {
		                            {TAB_DECL(HANDLING_CURRENT_PROFILE), view::handling_current_profile}},
		                            {TAB_DECL(HANDLING_SAVED_PROFILE), view::handling_saved_profiles}},
		                        },
		                    },
		                },
						{TAB_DECL(LSC), view::lsc}},
						{TAB_DECL(SPAWN_VEHICLE), view::spawn_vehicle}},
						{TAB_DECL(FUN_VEHICLE), view::fun_vehicle}},
		            },
		        },
		    },
		    {
				TAB_DECL(WORLD),
		            view::world,
		            {
						{TAB_DECL(SPAWN_PED), view::spawn_ped}},
						{TAB_DECL(SQUAD_SPAWNER), view::squad_spawner}},
						{TAB_DECL(CREATOR), view::creator}},
						{TAB_DECL(TRAIN), view::train}},
						{TAB_DECL(BLACKHOLE), view::blackhole}},
						{TAB_DECL(MODEL_SWAPPER), view::model_swapper}},
						{TAB_DECL(VFX), view::vfx}},
						{TAB_DECL(XML_MAPS), view::xml_maps}},
		            },
		        },
		    },
		    {
		        TAB_DECL(NETWORK),
		            view::network,
		            {
						{TAB_DECL(CHAT), view::chat}},
						{TAB_DECL(NETWORK_CONTROLS), view::network_controls}},
						{TAB_DECL(MISSIONS), view::missions}},
						{TAB_DECL(SPOOFING), view::spoofing}},
						{TAB_DECL(PLAYER_DATABASE), view::player_database}},
						{TAB_DECL(SESSION_BROWSER), view::session_browser}},
						{TAB_DECL(STAT_EDITOR), view::stat_editor}},
		            },
		        },
		    },
		    {
		        TAB_DECL(SETTINGS),
		            view::settings,
		            {
						{TAB_DECL(LUA_SCRIPTS), view::lua_scripts}},
						{TAB_DECL(ESP_SETTINGS), view::esp_settings}},
						{TAB_DECL(GTA_CACHE_SETTINGS), view::gta_cache}},
						{TAB_DECL(GUI_SETTINGS), view::gui_settings}},
						{TAB_DECL(HOTKEY_SETTINGS), view::hotkey_settings}},
						{TAB_DECL(REACTION_SETTINGS), view::reaction_settings}},
						{TAB_DECL(PROTECTION_SETTINGS), view::protection_settings}},
						{TAB_DECL(PROXY_SETTINGS), view::proxy_settings}},
						{TAB_DECL(DEBUG), nullptr}},
		            },
		        },
		    },
		    {
		        tabs::PLAYER,
		        {"", view::view_player},
		    },
		};
		// clang-format on

		void remove_from_nav_internal(std::map<big::tabs, big::navigation_struct>& nav, big::tabs existing_tab_id);

	public:
		gui_service();
		virtual ~gui_service();

		int nav_ctr = 0;

		navigation_struct* get_selected();
		std::vector<tabs>& get_selected_tab();
		bool has_switched_view();
		void set_selected(tabs);
		void set_nav_size(int);
		void increment_nav_size();
		void reset_nav_size();
		std::map<tabs, navigation_struct>& get_navigation();
		void remove_from_nav(tabs existing_tab_id);
	};

	inline gui_service* g_gui_service{};
}