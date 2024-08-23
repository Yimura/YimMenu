#include "hooking/hooking.hpp"
#include "util/mobile.hpp"
#include "util/notify.hpp"

namespace big
{
	void hooks::received_clone_remove(CNetworkObjectMgr* mgr, CNetGamePlayer* src, CNetGamePlayer* dst, int16_t object_id, uint32_t ownership_token)
	{
		if (!dst || !dst->m_player_info || !dst->m_player_info->m_ped || !dst->m_player_info->m_ped->m_net_object
		    || dst->m_player_info->m_ped->m_net_object->m_object_id == object_id) [[unlikely]]
		{
			notify::crash_blocked(src, "player ped removal");
			return;
		}

		if (const auto personal_vehicle = mobile::mechanic::get_personal_cvehicle())
		{
			if (auto net_obj = personal_vehicle->m_net_object)
			{
				if (object_id == net_obj->m_object_id && entity::network_has_control_of_entity(net_obj))
				{
					/*LOG(VERBOSE) << "DELETION ORDER FOR PERSONAL VEHICLE: " << net_obj->m_object_id << " m_is_remote: " << net_obj->m_is_remote
					             << " m_bubble: " << net_obj->m_bubble << " m_owner_id: " << net_obj->m_owner_id
					             << " m_wants_to_be_owner: " << net_obj->m_wants_to_be_owner;*/
					if (auto plyr = g_player_service->get_by_id(src->m_player_id))
					{
						g.reactions.delete_vehicle.process(plyr);
					}
					return;
				}
			}
		}

		if (self::veh != 0)
		{
			if (const auto current_vehicle = g_local_player->m_vehicle)
			{
				auto net_obj = current_vehicle->m_net_object;
				if (object_id == net_obj->m_object_id && entity::network_has_control_of_entity(net_obj))
				{
					/*LOG(VERBOSE) << "DELETION ORDER FOR CURRENT VEHICLE: " << net_obj->m_object_id
					             << " m_is_remote: " << net_obj->m_is_remote << " m_bubble: " << net_obj->m_bubble
					             << " m_owner_id: " << net_obj->m_owner_id << " m_wants_to_be_owner: " << net_obj->m_wants_to_be_owner;*/
					if (auto plyr = g_player_service->get_by_id(src->m_player_id))
					{
						g.reactions.delete_vehicle.process(plyr);
					}
					return;
				}
			}
		}

		if (auto object = g_pointers->m_gta.m_get_net_object(*g_pointers->m_gta.m_network_object_mgr, object_id, true)) [[likely]]
		{
			if (object->m_object_type == (int)eNetObjType::NET_OBJ_TYPE_PLAYER && object->m_owner_id != src->m_player_id) [[unlikely]]
			{
				std::string target = "<UNKNOWN>";

				if (auto tgt = g_player_service->get_by_id(object->m_owner_id))
					target = tgt->get_name();

				LOGF(stream::net_sync, WARNING, "Rejecting clone remove from {}, who is trying to delete {}'s player ped", src->get_name(), target);
				return;
			}
		}

		g_hooking->get_original<hooks::received_clone_remove>()(mgr, src, dst, object_id, ownership_token);
	}
}