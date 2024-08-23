#include "fiber_pool.hpp"
#include "gta/enums.hpp"
#include "gta/net_game_event.hpp"
#include "gta/weapon_info_manager.hpp"
#include "hooking/hooking.hpp"
#include "util/math.hpp"
#include "util/mobile.hpp"
#include "util/notify.hpp"
#include "util/toxic.hpp"

#include <base/CObject.hpp>
#include <network/CNetGamePlayer.hpp>

namespace big
{
	static void script_id_deserialize(CGameScriptId& id, rage::datBitBuffer& buffer)
	{
		id.m_hash      = buffer.Read<uint32_t>(32);
		id.m_timestamp = buffer.Read<uint32_t>(32);

		if (buffer.Read<bool>(1))
			id.m_position_hash = buffer.Read<uint32_t>(32);

		if (buffer.Read<bool>(1))
			id.m_instance_id = buffer.Read<int32_t>(8);
	}

	static bool is_valid_weapon(rage::joaat_t hash)
	{
		for (const auto& info : g_pointers->m_gta.m_weapon_info_manager->m_item_infos)
		{
			if (info && info->m_name == hash && info->GetClassId() == "cweaponinfo"_J)
			{
				return true;
			}
		}

		return false;
	}

	inline bool is_local_vehicle(int16_t net_id)
	{
		return g_local_player && g_local_player->m_vehicle && g_local_player->m_vehicle->m_net_object
		    && g_local_player->m_vehicle->m_driver == g_local_player && g_local_player->m_vehicle->m_net_object->m_object_id == net_id;
	}

	inline bool is_in_vehicle(CPed* ped)
	{
		for (int i = 0; i < 15; i++)
			if (g_local_player->m_vehicle->m_passengers[i] == ped)
				return true;

		return false;
	}

	// Returns true if bad event
	bool scan_weapon_damage_event(rage::netEventMgr* event_manager, CNetGamePlayer* player, CNetGamePlayer* target_player, int event_index, int event_handled_bitset, rage::datBitBuffer* buffer)
	{
		uint8_t damageType;
		uint32_t weaponType; // weaponHash

		bool overrideDefaultDamage;
		bool hitEntityWeapon;
		bool hitWeaponAmmoAttachment;
		bool silenced;

		uint32_t damageFlags;
		bool hasActionResult;

		uint32_t actionResultName;
		uint16_t actionResultId;
		uint32_t f104;

		uint32_t weaponDamage;
		bool isNetTargetPos;

		rage::fvector3 localPos;

		bool f112;

		uint32_t damageTime;
		bool willKill;
		uint32_t f120;
		bool hasVehicleData;

		uint16_t f112_1;

		uint16_t parentGlobalId; // Source entity?
		uint16_t hitGlobalId;    // Target entity?

		uint8_t tyreIndex;
		uint8_t suspensionIndex;
		uint8_t hitComponent;

		bool f133;
		bool hasImpactDir;

		rage::fvector3 impactDir;

		damageType = buffer->Read<uint8_t>(2);
		weaponType = buffer->Read<uint32_t>(32);

		if ("WEAPON_TRANQUILIZER"_J == weaponType)
		{
			if (auto plyr = g_player_service->get_by_id(player->m_player_id))
			{
				g.reactions.break_game.process(plyr);
			}
			return true;
		}

		if (!is_valid_weapon(weaponType))
		{
			notify::crash_blocked(player, "invalid weapon type");
			LOGF(stream::net_events,
			    WARNING,
			    "Blocked WEAPON_DAMAGE_EVENT from {} with invalid weapon hash {:X}",
			    player->get_name(),
			    weaponType);
			return true;
		}

		overrideDefaultDamage   = buffer->Read<uint8_t>(1);
		hitEntityWeapon         = buffer->Read<uint8_t>(1);
		hitWeaponAmmoAttachment = buffer->Read<uint8_t>(1);
		silenced                = buffer->Read<uint8_t>(1);

		damageFlags = buffer->Read<uint32_t>(24);
		// (damageFlags >> 1) & 1
		hasActionResult = buffer->Read<uint8_t>(1);

		if (hasActionResult)
		{
			actionResultName = buffer->Read<uint32_t>(32);
			actionResultId   = buffer->Read<uint16_t>(16);
			f104
				= buffer->Read<uint32_t>(32);
		}

		if (overrideDefaultDamage)
		{
			weaponDamage = buffer->Read<uint32_t>(17);
		}
		else
		{
			weaponDamage = 0;
		}

		bool _f92 = buffer->Read<uint8_t>(1);

		if (_f92)
		{
			buffer->Read<uint8_t>(4);
		}

		isNetTargetPos = buffer->Read<uint8_t>(1);

		if (isNetTargetPos)
		{
			localPos.x = buffer->ReadSignedFloat(16, 55.f); // divisor: 0x425C0000
			localPos.y = buffer->ReadSignedFloat(16, 55.f);
			localPos.z = buffer->ReadSignedFloat(16, 55.f);
		}

		if (damageType == 3)
		{
			damageTime = buffer->Read<uint32_t>(32);
			willKill   = buffer->Read<uint8_t>(1);

			if (hasActionResult)
			{
				hitGlobalId = buffer->Read<uint16_t>(13);
			}
			else
			{
				hitGlobalId = 0;
			}

			f112 = buffer->Read<uint8_t>(1);

			if (!f112)
			{
				f112_1 = buffer->Read<uint16_t>(11);
			}
			else
			{
				f112_1 = buffer->Read<uint16_t>(16);
			}
		}
		else
		{
			parentGlobalId = buffer->Read<uint16_t>(13); // +118
			hitGlobalId    = buffer->Read<uint16_t>(13); // +120
		}

		if (damageType < 2)
		{
			localPos.x = buffer->ReadSignedFloat(16, 55.f); // divisor: 0x425C0000
			localPos.y = buffer->ReadSignedFloat(16, 55.f);
			localPos.z = buffer->ReadSignedFloat(16, 55.f);

			if (damageType == 1)
			{
				hasVehicleData = buffer->Read<uint8_t>(1);

				if (hasVehicleData)
				{
					tyreIndex       = buffer->Read<uint8_t>(4); // +122
					suspensionIndex = buffer->Read<uint8_t>(4); // +123
				}
			}
		}
		else
		{
			hitComponent = buffer->Read<uint8_t>(5); // +108
		}

		f133         = buffer->Read<uint8_t>(1);
		hasImpactDir = buffer->Read<uint8_t>(1);

		if (hasImpactDir)
		{
			impactDir.x = buffer->ReadSignedFloat(16, 6.2831854820251f); // divisor: 0x40C90FDB
			impactDir.y = buffer->ReadSignedFloat(16, 6.2831854820251f);
			impactDir.z = buffer->ReadSignedFloat(16, 6.2831854820251f);
		}

		buffer->Seek(0);

		// LOG(INFO) << localPos.x << " " << localPos.y << " " << localPos.z << ". " << math::distance_between_vectors(localPos, *g_local_player->m_navigation->get_position()) << " " << (g_local_player->m_net_object->m_object_id == hitGlobalId ? "TRUE" : "FALSE") << " " << damageType << " " << damageFlags;

		if (damageType == 3 && (damageFlags & (1 << 1)) == 0)
			hitGlobalId = g_local_player ? g_local_player->m_net_object->m_object_id : 0;

		if (g.session.damage_karma && g_local_player && g_local_player->m_net_object
		    && (g_local_player->m_net_object->m_object_id == hitGlobalId
		        || math::distance_between_vectors(localPos, *g_local_player->m_navigation->get_position()) < 1.5f))
		{
			int id = player->m_player_id;
			g_fiber_pool->queue_job([id, hitComponent, overrideDefaultDamage, weaponType, weaponDamage, tyreIndex, suspensionIndex, damageFlags, actionResultName, actionResultId, f104, hitEntityWeapon, hitWeaponAmmoAttachment, silenced, hasImpactDir, impactDir, localPos] {
				auto player = g_player_service->get_by_id(id);

				if (!player->is_valid() || !player->get_ped())
					return;

				g_pointers->m_gta.m_send_network_damage(g_player_service->get_self()->get_ped(),
				    player->get_ped(),
				    (rage::fvector3*)&localPos,
				    hitComponent,
				    overrideDefaultDamage,
				    weaponType,
				    weaponDamage,
				    tyreIndex,
				    suspensionIndex,
				    damageFlags,
				    actionResultName,
				    actionResultId,
				    f104,
				    hitEntityWeapon,
				    hitWeaponAmmoAttachment,
				    silenced,
				    false,
				    player->get_ped()->m_navigation->get_position());
			});
		}

		return false;
	}

	void scan_explosion_event(CNetGamePlayer* player, rage::datBitBuffer* buffer)
	{
		uint16_t f186;
		uint16_t targetEntity;
		uint16_t ownerNetId;
		uint16_t f214;
		eExplosionTag explosionType;
		float damageScale;

		float posX;
		float posY;
		float posZ;

		bool f242;
		uint16_t f104;
		float cameraShake;

		bool isAudible;
		bool f189;
		bool isInvisible;
		bool f126;
		bool addOwnedExplosion;

		bool hasTargetEnt2;
		uint16_t targetEnt2;

		float unkX;
		float unkY;
		float unkZ;

		bool f190;
		bool f191;

		uint32_t f164;

		float posX224;
		float posY224;
		float posZ224;

		bool f240;
		uint16_t f218;
		bool f216;

		// clang-format off

		f186 = buffer->Read<uint16_t>(16);
		targetEntity = buffer->Read<uint16_t>(13);
		ownerNetId  = buffer->Read<uint16_t>(13);
		f214        = buffer->Read<uint16_t>(13);               // 1604+
		explosionType = (eExplosionTag)buffer->ReadSigned<int>(8);// 1604+ bit size
		damageScale = buffer->Read<int>(8) / 255.0f;

		posX        = buffer->ReadSignedFloat(22, 27648.0f);
		posY        = buffer->ReadSignedFloat(22, 27648.0f);
		posZ        = buffer->ReadFloat(22, 4416.0f) - 1700.0f;

		f242        = buffer->Read<bool>(1);
		f104        = buffer->Read<uint16_t>(16);
		cameraShake = buffer->Read<int>(8) / 127.0f;

		isAudible   = buffer->Read<uint8_t>(1);
		f189        = buffer->Read<uint8_t>(1);
		isInvisible = buffer->Read<uint8_t>(1);
		f126        = buffer->Read<uint8_t>(1);
		addOwnedExplosion = buffer->Read<uint8_t>(1);
		buffer->Read<uint8_t>(1);

		hasTargetEnt2 = buffer->Read<bool>(1);// 1604+
		targetEnt2 = buffer->Read<uint16_t>(13);

		unkX        = buffer->ReadSignedFloat(16, 1.1f);
		unkY        = buffer->ReadSignedFloat(16, 1.1f);
		unkZ        = buffer->ReadSignedFloat(16, 1.1f);

		f190     	= buffer->Read<uint8_t>(1);
		f191        = buffer->Read<uint8_t>(1);
		f164        = buffer->Read<uint32_t>(32);

		if (f242)
		{
			posX224 = buffer->ReadSignedFloat(31, 27648.0f);
			posY224 = buffer->ReadSignedFloat(31, 27648.0f);
			posZ224 = buffer->ReadFloat(31, 4416.0f) - 1700.0f;
		}
		else
		{
			posX224 = 0;
			posY224 = 0;
			posZ224 = 0;
		}

		auto f168 = buffer->Read<uint32_t>(32);// >= 1868: f_168


		f240 = buffer->Read<uint8_t>(1);
		if (f240)
		{
			f218 = buffer->Read<uint16_t>(16);

			if (f191)
			{
				f216 = buffer->Read<uint8_t>(8);
			}
		}

		buffer->Seek(0);

		auto object = g_pointers->m_gta.m_get_net_object(*g_pointers->m_gta.m_network_object_mgr, ownerNetId, true);
		auto entity = object ? object->GetGameObject() : nullptr;

		auto offset_object = g_pointers->m_gta.m_get_net_object(*g_pointers->m_gta.m_network_object_mgr, targetEnt2, true);

		if (addOwnedExplosion && entity && entity->m_entity_type == 4 && reinterpret_cast<CPed*>(entity)->m_player_info
			&& player->m_player_info->m_ped && player->m_player_info->m_ped->m_net_object
			&& ownerNetId != player->m_player_info->m_ped->m_net_object->m_object_id)
		{
			auto p_name = player->get_name();

			g_notification_service.push_error("WARNING"_T.data(),
				std::vformat("BLAMED_FOR_EXPLOSION"_T,
					std::make_format_args(p_name,
						reinterpret_cast<CPed*>(entity)->m_player_info->m_net_player_data.m_name)));
			session::add_infraction(g_player_service->get_by_id(player->m_player_id), Infraction::BLAME_EXPLOSION_DETECTED);
			LOGF(stream::net_events, WARNING, "{} sent an EXPLOSION_EVENT with addOwnedExplosion enabled and with the wrong owner", player->get_name());
			return;
		}

		if (g.session.explosion_karma && g_local_player
			&& math::distance_between_vectors({posX, posY, posZ}, *g_local_player->m_navigation->get_position()) < 3.0f)
		{
			int id = player->m_player_id;
			g_fiber_pool->queue_job([id, explosionType, damageScale, cameraShake, isAudible, isInvisible] {
				toxic::blame_explode_player(g_player_service->get_self(), g_player_service->get_by_id(id), explosionType, damageScale, isAudible, isInvisible, cameraShake);
			});
		}

		// clang-format on
	}

	bool scan_play_sound_event(player_ptr plyr, rage::datBitBuffer& buffer)
	{
		bool is_entity = buffer.Read<bool>(1);
		std::int16_t entity_net_id;
		rage::fvector3 position;

		if (is_entity)
		{
			entity_net_id = buffer.Read<uint16_t>(13);
		}
		else
		{
			position.x = buffer.ReadSignedFloat(19, 1337.0f);
			position.y = buffer.ReadSignedFloat(19, 1337.0f);
			position.z = buffer.ReadFloat(19, 1337.0f);
		}

		bool has_ref        = buffer.Read<bool>(1);
		uint32_t ref_hash   = has_ref ? buffer.Read<uint32_t>(32) : 0;
		uint32_t sound_hash = buffer.Read<uint32_t>(32);
		uint8_t sound_id    = buffer.Read<uint8_t>(8);

		bool has_script_hash = buffer.Read<bool>(1);
		uint32_t script_hash = has_script_hash ? buffer.Read<uint32_t>(32) : 0;

		static const std::unordered_set<uint32_t> blocked_ref_hashes = {"Arena_Vehicle_Mod_Shop_Sounds"_J, "CELEBRATION_SOUNDSET"_J, "DLC_AW_Arena_Office_Planning_Wall_Sounds"_J, "DLC_AW_Arena_Spin_Wheel_Game_Frontend_Sounds"_J, "DLC_Biker_SYG_Sounds"_J, "DLC_BTL_SECURITY_VANS_RADAR_PING_SOUNDS"_J, "DLC_BTL_Target_Pursuit_Sounds"_J, "DLC_GR_Bunker_Door_Sounds"_J, "DLC_GR_CS2_Sounds"_J, "DLC_IO_Warehouse_Mod_Garage_Sounds"_J, "DLC_MPSUM2_HSW_Up_Sounds"_J, "DLC_sum20_Business_Battle_AC_Sounds"_J, "DLC_TG_Running_Back_Sounds"_J, "dlc_vw_table_games_frontend_sounds"_J, "dlc_xm_facility_entry_exit_sounds"_J, "Frontend"_J, "GTAO_Boss_Goons_FM_Soundset"_J, "GTAO_Exec_SecuroServ_Computer_Sounds"_J, "GTAO_Exec_SecuroServ_Warehouse_PC_Sounds"_J, "GTAO_Script_Doors_Faded_Screen_Sounds"_J, "GTAO_SMG_Hangar_Computer_Sounds"_J, "HUD_AMMO_SHOP_SOUNDSET"_J, "HUD_FRONTEND_CUSTOM_SOUNDSET"_J, "HUD_FRONTEND_DEFAULT_SOUNDSET"_J, "HUD_FRONTEND_MP_SOUNDSET"_J, "HUD_FRONTEND_MP_COLLECTABLE_SOUNDS"_J, "HUD_FRONTEND_TATTOO_SHOP_SOUNDSET"_J, "HUD_FRONTEND_CLOTHESSHOP_SOUNDSET"_J, "HUD_FRONTEND_STANDARD_PICKUPS_NPC_SOUNDSET"_J, "HUD_FRONTEND_VEHICLE_PICKUPS_NPC_SOUNDSET"_J, "HUD_FRONTEND_WEAPONS_PICKUPS_NPC_SOUNDSET"_J, "HUD_FREEMODE_SOUNDSET"_J, "HUD_MINI_GAME_SOUNDSET"_J, "HUD_AWARDS"_J, "JA16_Super_Mod_Garage_Sounds"_J, "Low2_Super_Mod_Garage_Sounds"_J, "MissionFailedSounds"_J, "MP_CCTV_SOUNDSET"_J, "MP_LOBBY_SOUNDS"_J, "MP_MISSION_COUNTDOWN_SOUNDSET"_J, "Phone_SoundSet_Default"_J, "Phone_SoundSet_Glasses_Cam"_J, "Phone_SoundSet_Prologue"_J, "Phone_SoundSet_Franklin"_J, "Phone_SoundSet_Michael"_J, "Phone_SoundSet_Trevor"_J, "PLAYER_SWITCH_CUSTOM_SOUNDSET"_J, "RESPAWN_ONLINE_SOUNDSET"_J, "TATTOOIST_SOUNDS"_J, "WastedSounds"_J, "WEB_NAVIGATION_SOUNDS_PHONE"_J};
		static const std::unordered_set<uint32_t> blocked_sound_hashes = {"Remote_Ring"_J, "COP_HELI_CAM_ZOOM"_J, "Object_Dropped_Remote"_J};
		static const std::unordered_set<uint32_t> blocked_script_hashes = {"main_persistent"_J, "shop_controller"_J};

		bool should_block = [&] {
			if (blocked_ref_hashes.contains(ref_hash) || blocked_sound_hashes.contains(sound_hash)
			    || blocked_script_hashes.contains(script_hash))
				return true;

			switch (sound_hash)
			{
			case "DLC_XM_Explosions_Orbital_Cannon"_J:
			{
				if (is_entity)
					return true;

				if (!scr_globals::globalplayer_bd.as<GlobalPlayerBD*>()->Entries[plyr->id()].OrbitalBitset.IsSet(eOrbitalBitset::kOrbitalCannonActive))
					return true;

				static const std::unordered_set<uint32_t> valid_script_hashes = {"am_mp_defunct_base"_J, "am_mp_orbital_cannon"_J, "fm_mission_controller_2020"_J, "fm_mission_controller"_J};
				if (!valid_script_hashes.contains(script_hash))
					return true;

				break;
			}
			}

			switch (ref_hash)
			{
			case "GTAO_Biker_Modes_Soundset"_J:
			case "DLC_Biker_Sell_Postman_Sounds"_J:
			{
				if (is_entity)
					return true;

				if (script_hash != "gb_biker_contraband_sell"_J)
					return true;

				break;
			}
			case "DLC_AW_General_Sounds"_J:
			{
				if (sound_hash != "Airhorn_Blast_Long"_J)
					return true;

				if (script_hash != "gb_casino_heist"_J)
					return true;

				if (!gta_util::find_script_thread("gb_casino_heist"_J))
					return true;

				break;
			}
			case "GTAO_FM_Events_Soundset"_J:
			{
				if (!is_entity)
					return true;

				if (sound_hash != "Explosion_Countdown"_J)
					return true;

				break;
			}
			}

			return false;
		}();

		if (should_block)
		{
			LOGF(stream::net_events, WARNING, "Blocked NETWORK_PLAY_SOUND_EVENT from {} with is_entity: {}, ref_hash: {:X}, sound_hash: {:X}, sound_id: {}, script_hash: {:X}", plyr->get_name(), is_entity ? "T" : "F", ref_hash, sound_hash, sound_id, script_hash);
		}

		buffer.Seek(0);
		return should_block;
	}

	void hooks::received_event(rage::netEventMgr* event_manager, CNetGamePlayer* source_player, CNetGamePlayer* target_player, uint16_t event_id, int event_index, int event_handled_bitset, int buffer_size, rage::datBitBuffer* buffer)
	{
		if (event_id > 91u) [[unlikely]]
		{
			g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
			return;
		}

		const auto event_name = *(char**)((DWORD64)event_manager + 8i64 * event_id + 243376);
		if (event_name == nullptr || source_player == nullptr || source_player->m_player_id < 0 || source_player->m_player_id >= 32) [[unlikely]]
		{
			g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
			return;
		}

		auto plyr = g_player_service->get_by_id(source_player->m_player_id);

		if (plyr && plyr->block_net_events) [[unlikely]]
		{
			g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
			return;
		}

		switch (static_cast<eNetworkEvents>(event_id))
		{
		case eNetworkEvents::KICK_VOTES_EVENT:
		{
			uint32_t player_bitfield = buffer->Read<uint32_t>(32);
			if (player_bitfield & (1 << target_player->m_player_id))
			{
				g.reactions.kick_vote.process(plyr);
			}
			buffer->Seek(0);
			break;
		}
		case eNetworkEvents::SCRIPT_ENTITY_STATE_CHANGE_EVENT:
		{
			uint16_t entity = buffer->Read<uint16_t>(13);
			auto type       = buffer->Read<ScriptEntityChangeType>(4);
			uint32_t unk    = buffer->Read<uint32_t>(32);
			if (type == ScriptEntityChangeType::SettingOfTaskVehicleTempAction)
			{
				uint16_t ped_id = buffer->Read<uint16_t>(13);
				uint32_t action = buffer->Read<uint32_t>(8);

				if ((action >= 15 && action <= 18) || action == 33)
				{
					g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
					notify::crash_blocked(source_player, "vehicle temp action");
					LOGF(stream::net_events, WARNING, "Blocked SCRIPT_ENTITY_STATE_CHANGE_EVENT of type SettingOfTaskVehicleTempAction with action {} that would crash the game", plyr->get_name(), action);
					return;
				}
			}
			else if (type == ScriptEntityChangeType::SetVehicleLockState)
			{
				if (g_local_player && g_local_player->m_vehicle && g_local_player->m_vehicle->m_net_object
				    && g_local_player->m_vehicle->m_net_object->m_object_id == entity)
				{
					g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
					LOGF(stream::net_events, WARNING, "Blocked SCRIPT_ENTITY_STATE_CHANGE_EVENT of type SetVehicleLockState from {} on our local vehicle", plyr->get_name());
					return;
				}
			}
			else if (type == ScriptEntityChangeType::SetVehicleExclusiveDriver)
			{
				if (g_local_player && g_local_player->m_vehicle && g_local_player->m_vehicle->m_net_object
				    && g_local_player->m_vehicle->m_net_object->m_object_id == entity)
				{
					g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
					LOGF(stream::net_events, WARNING, "Blocked SCRIPT_ENTITY_STATE_CHANGE_EVENT of type SetVehicleExclusiveDriver from {} on our local vehicle", plyr->get_name());
					g.reactions.vehicle_kick.process(plyr);
					return;
				}
			}
			else if (type == ScriptEntityChangeType::SetPedFacialIdleAnimOverride)
			{
				if (g_local_player && g_local_player->m_net_object && g_local_player->m_net_object->m_object_id)
				{
					g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
					LOGF(stream::net_events, WARNING, "Blocked SCRIPT_ENTITY_STATE_CHANGE_EVENT of type SetPedFacialIdleAnimOverride from {} on our local player", plyr->get_name());
					return;
				}
			}
			else if (type > ScriptEntityChangeType::SetVehicleExclusiveDriver || type < ScriptEntityChangeType::BlockingOfNonTemporaryEvents)
			{
				notify::crash_blocked(source_player, "invalid script entity change type");
				g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
				return;
			}
			buffer->Seek(0);
			break;
		}
		case eNetworkEvents::SCRIPTED_GAME_EVENT:
		{
			const auto scripted_game_event = std::make_unique<CScriptedGameEvent>();

			buffer->ReadDword(&scripted_game_event->m_args_size, 32);
			if (scripted_game_event->m_args_size > sizeof(scripted_game_event->m_args))
			{
				g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
				return;
			}

			buffer->ReadArray(&scripted_game_event->m_args, 8 * scripted_game_event->m_args_size);

			if (hooks::scripted_game_event(scripted_game_event.get(), source_player))
			{
				g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
				return;
			}

			buffer->Seek(0);
			break;
		}
		case eNetworkEvents::NETWORK_CLEAR_PED_TASKS_EVENT:
		{
			int net_id = buffer->Read<int>(13);

			if (g_local_player && g_local_player->m_net_object && g_local_player->m_net_object->m_object_id == net_id)
			{
				g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
				g.reactions.clear_ped_tasks.process(plyr);
				return;
			}

			buffer->Seek(0);
			break;
		}
		case eNetworkEvents::RAGDOLL_REQUEST_EVENT:
		{
			int net_id = buffer->Read<int>(13);

			if (g_local_player && g_local_player->m_net_object && g_local_player->m_net_object->m_object_id == net_id)
			{
				g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
				g.reactions.remote_ragdoll.process(plyr);
				return;
			}

			buffer->Seek(0);
			break;
		}
		// Don't block this event, we still want to report this player
		// because if we still report others, our account seems less fishy
		case eNetworkEvents::REPORT_CASH_SPAWN_EVENT:
		{
			uint32_t money;

			buffer->Seek(64);
			buffer->ReadDword(&money, 32);
			buffer->Seek(0);

			if (money >= 2000)
			{
				g.reactions.report_cash_spawn.process(plyr);
			}

			break;
		}
		// player sending this event is a modder
		case eNetworkEvents::REPORT_MYSELF_EVENT:
		{
			auto p1 = buffer->Read<int>(32);
			auto p2 = buffer->Read<int>(32);
			LOGF(stream::net_events, VERBOSE, "Received REPORT_MYSELF_EVENT from {} with parameters ({}, {})", plyr->get_name(), p1, p2);
			session::add_infraction(plyr, Infraction::TRIGGERED_ANTICHEAT);
			g.reactions.game_anti_cheat_modder_detection.process(plyr);
			buffer->Seek(0);
			break;
		}
		case eNetworkEvents::REQUEST_CONTROL_EVENT:
		{
			auto net_id = buffer->Read<int>(13);
			if (g_local_player && g_local_player->m_vehicle && g_local_player->m_vehicle->m_net_object
			    && g_local_player->m_vehicle->m_net_object->m_object_id == net_id) //The request is for a vehicle we are currently in.
			{
				Vehicle personal_vehicle = mobile::mechanic::get_personal_vehicle();
				Vehicle veh              = g_pointers->m_gta.m_ptr_to_handle(g_local_player->m_vehicle);
				if (!NETWORK::NETWORK_IS_ACTIVITY_SESSION()     //If we're in Freemode.
				    || personal_vehicle == veh                  //Or we're in our personal vehicle.
				    || self::spawned_vehicles.contains(net_id)) // Or it's a vehicle we spawned.
				{
					// Let trusted friends and players request control (e.g., they want to hook us to their tow-truck or something)
					if (plyr && (plyr->is_trusted || (g.session.trust_friends && plyr->is_friend())))
					{
						buffer->Seek(0);
						break;
					}

					if (g_local_player->m_vehicle->m_driver != source_player->m_player_info->m_ped) //This will block hackers who are not in the car but still want control.
					{
						g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset); // Tell them to get bent.
						g.reactions.request_control_event.process(plyr);
						return;
					}
				}
			}
			buffer->Seek(0);
			break;
		}
		case eNetworkEvents::SCRIPT_WORLD_STATE_EVENT:
		{
			auto type = buffer->Read<WorldStateDataType>(4);
			buffer->Read<bool>(1);
			CGameScriptId id;
			script_id_deserialize(id, *buffer);

			if (type == WorldStateDataType::Rope)
			{
				buffer->Read<int>(9);        // network rope id
				buffer->ReadSigned<int>(19); // pos x
				buffer->ReadSigned<int>(19); // pos y
				buffer->Read<int>(19);       // pos z
				buffer->ReadSigned<int>(19); // rot x
				buffer->ReadSigned<int>(19); // rot y
				buffer->Read<int>(19);       // rot z
				float max_length     = buffer->ReadSignedFloat(16, 100.0f);
				int type             = buffer->Read<int>(4);
				float initial_length = buffer->ReadSignedFloat(16, 100.0f);
				float min_length     = buffer->ReadSignedFloat(16, 100.0f);

				if (type == 0 || initial_length < min_length || max_length < min_length || max_length < 0.0f)
				{
					LOGF(stream::net_events, WARNING, "{} sent a SCRIPT_WORLD_STATE_EVENT of type Rope that would crash the game. Script Hash: {:X}, Type: {}, Initial Length: {}, Min Length: {}, Max Length: {}", plyr->get_name(), id.m_hash, type, initial_length, min_length, max_length);
					notify::crash_blocked(source_player, "rope");
					g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
					return;
				}
			}
			else if (type == WorldStateDataType::PopGroupOverride)
			{
				int pop_schedule = buffer->ReadSigned<int>(8); // Pop Schedule
				int pop_group    = buffer->Read<int>(32);      // Pop Group
				int percentage   = buffer->Read<int>(7);       // Percentage

				if (pop_group == 0 && (percentage == 0 || percentage == 103))
				{
					notify::crash_blocked(source_player, "pop group override");
					LOGF(stream::net_events, WARNING, "{} sent a SCRIPT_WORLD_STATE_EVENT of type PopGroupOverride that would crash the game. Pop schedule: {}, Pop group: {}, Percentage: {}, Script Hash: {:X}", plyr->get_name(), pop_schedule, pop_group, percentage, id.m_hash);
					g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
					return;
				}
			}
			else if (type > WorldStateDataType::VehiclePlayerLocking || type < WorldStateDataType::CarGen)
			{
				notify::crash_blocked(source_player, "invalid world state type");
				g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
				return;
			}
			else if (type == WorldStateDataType::PopMultiplierArea && g.protections.stop_traffic && !NETWORK::NETWORK_IS_ACTIVITY_SESSION())
			{
				LOGF(stream::net_events, WARNING, "Blocked a SCRIPT_WORLD_STATE_EVENT of type PopMultiplierArea from {}", plyr->get_name());
				g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
				return;
			}

			buffer->Seek(0);
			break;
		}
		case eNetworkEvents::REMOVE_WEAPON_EVENT:
		{
			std::int16_t net_id = buffer->Read<std::int16_t>(13);
			Hash hash           = buffer->Read<Hash>(32);

			if (hash == "WEAPON_UNARMED"_J)
			{
				LOGF(stream::net_events, WARNING, "{} sent a REMOVED_WEAPON_EVENT with weapon hash == WEAPON_UNARMED", plyr->get_name());
				notify::crash_blocked(source_player, "remove unarmed");
				g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
				return;
			}

			if (g_local_player && g_local_player->m_net_object && g_local_player->m_net_object->m_object_id == net_id)
			{
				weapon_item weapon = g_gta_data_service.weapon_by_hash(hash);
				g_notification_service.push_warning("PROTECTIONS"_T.data(),
				    std::format("{} {} {}.", source_player->get_name(), "REMOVE_WEAPON_ATTEMPT_MESSAGE"_T, weapon.m_display_name));
				g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
				return;
			}

			buffer->Seek(0);
			break;
		}
		case eNetworkEvents::GIVE_WEAPON_EVENT:
		{
			std::int16_t net_id = buffer->Read<std::int16_t>(13);
			Hash hash           = buffer->Read<Hash>(32);

			if (g_local_player && g_local_player->m_net_object && g_local_player->m_net_object->m_object_id == net_id)
			{
				weapon_item weapon = g_gta_data_service.weapon_by_hash(hash);
				g_notification_service.push_warning("PROTECTIONS"_T.data(),
				    std::format("{} {} {}.", source_player->get_name(), "GIVE_WEAPON_ATTEMPT_MESSAGE"_T, weapon.m_display_name));
				g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
				return;
			}

			buffer->Seek(0);
			break;
		}
		case eNetworkEvents::GIVE_CONTROL_EVENT:
		{
			uint32_t timestamp                = buffer->Read<uint32_t>(32);
			int count                         = buffer->Read<int>(2);
			bool all_objects_migrate_together = buffer->Read<bool>(1);
			eNetObjType sync_type;

			if (count > 3)
			{
				count = 3;
			}

			for (int i = 0; i < count; i++)
			{
				int net_id              = buffer->Read<int>(13);
				eNetObjType object_type = buffer->Read<eNetObjType>(4);
				int migration_type      = buffer->Read<int>(3);

				if (object_type < eNetObjType::NET_OBJ_TYPE_AUTOMOBILE || object_type > eNetObjType::NET_OBJ_TYPE_TRAIN)
				{
					notify::crash_blocked(source_player, "out of bounds give control type");
					g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
					return;
				}

				sync_type = object_type;
			}

			buffer->Seek(0);

			if (count)
			{
				g.m_syncing_player      = source_player;
				g.m_syncing_object_type = sync_type;
			}
			break;
		}
		case eNetworkEvents::NETWORK_PLAY_SOUND_EVENT:
		{
			if (plyr && plyr->m_play_sound_rate_limit.process())
			{
				if (plyr->m_play_sound_rate_limit.exceeded_last_process())
				{
					//notify::crash_blocked(source_player, "sound spam"); --- false positives
				}
				g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
				return;
			}

			if (plyr && scan_play_sound_event(plyr, *buffer))
			{
				g.reactions.sound_spam.process(plyr);
				g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
				return;
			}

			break;
		}
		case eNetworkEvents::EXPLOSION_EVENT:
		{
			if (plyr && plyr->block_explosions)
			{
				g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
				return;
			}

			scan_explosion_event(source_player, buffer);
			break;
		}
		case eNetworkEvents::WEAPON_DAMAGE_EVENT:
		{
			if (scan_weapon_damage_event(event_manager, source_player, target_player, event_index, event_handled_bitset, buffer))
			{
				g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
				return;
			}
			break;
		}
		case eNetworkEvents::ACTIVATE_VEHICLE_SPECIAL_ABILITY_EVENT:
		{
			int16_t net_id = buffer->Read<int16_t>(13);

			if (is_local_vehicle(net_id))
			{
				g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
				return;
			}

			buffer->Seek(0);
			break;
		}
		case eNetworkEvents::DOOR_BREAK_EVENT:
		{
			int16_t net_id = buffer->Read<int16_t>(13);

			if (is_local_vehicle(net_id))
			{
				g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
				return;
			}

			buffer->Seek(0);
			break;
		}
		case eNetworkEvents::CHANGE_RADIO_STATION_EVENT:
		{
			int16_t net_id = buffer->Read<int16_t>(13);

			if (is_local_vehicle(net_id))
			{
				if (!is_in_vehicle(plyr->get_ped()))
				{
					g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
					return;
				}

				if (plyr->m_radio_station_change_rate_limit.process())
				{
					g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
					return;
				}
			}
			else
			{
				g_pointers->m_gta.m_send_event_ack(event_manager, source_player, target_player, event_index, event_handled_bitset);
				return;
			}

			buffer->Seek(0);
			break;
		}
		}

		return g_hooking->get_original<received_event>()(event_manager, source_player, target_player, event_id, event_index, event_handled_bitset, buffer_size, buffer);
	}
}
