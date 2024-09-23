#include "core/data/menu_event.hpp"
#include "hooking/hooking.hpp"
#include "lua/lua_manager.hpp"
#include "services/players/player_service.hpp"
#include "services/battleye/battleye_service.hpp"

#include <network/CNetworkPlayerMgr.hpp>

namespace big
{
	bool hooks::network_player_mgr_init(CNetworkPlayerMgr* _this, uint64_t a2, uint32_t a3, uint32_t a4[4])
	{
		if (g.notifications.network_player_mgr_init.log)
			LOG(INFO) << "CNetworkPlayerMgr#init got called, we're probably entering a session.";
		if (g.notifications.network_player_mgr_init.notify)
			g_notification_service.push("NETWORK_PLAYER_MGR"_T.data(), "NETWORK_PLAYER_MGR_INIT"_T.data());

		bool result = g_hooking->get_original<hooks::network_player_mgr_init>()(_this, a2, a3, a4);

		g.session.trust_session = false;
		g_player_service->player_join(_this->m_local_net_player);
		g_lua_manager->trigger_event<menu_event::PlayerMgrInit>();

		return result;
	}

	void hooks::network_player_mgr_shutdown(CNetworkPlayerMgr* _this)
	{
		for (auto& [_, player] : g_player_service->players())
		{
			if (player->is_valid())
			{
				g_battleye_service.remove_player(player->get_net_game_player()->get_host_token());
			}
		}

		g_player_service->do_cleanup();
		self::spawned_vehicles.clear();

		if (g.notifications.network_player_mgr_shutdown.log)
			LOG(INFO) << "CNetworkPlayerMgr#shutdown got called, we're probably leaving our session.";
		if (g.notifications.network_player_mgr_shutdown.notify)
			g_notification_service.push("NETWORK_PLAYER_MGR"_T.data(), "NETWORK_PLAYER_MGR_DESTROY"_T.data());

		g.session.trust_session = false;
		g_hooking->get_original<hooks::network_player_mgr_shutdown>()(_this);
		g_lua_manager->trigger_event<menu_event::PlayerMgrShutdown>();
	}
}
