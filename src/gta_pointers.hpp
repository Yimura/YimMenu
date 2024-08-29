#pragma once
#include "function_types.hpp"
#include "gta/enums.hpp"

#include <memory/handle.hpp>

class CCommunications;
class FriendRegistry;
class CNetworkPlayerMgr;
class Network;
class ChatData;
class ScInfo;
class GenericPool;
class VehiclePool;
class CVehicleSeatMetadataMgr;
class CVehicleDriveByMetadataMgr;
class CBlipList;
class TimecycleKeyframeData;
class CTrainConfig;
class CWeaponInfoManager;
class CGameScriptHandlerMgr;
class CPedFactory;
class GtaThread;
class GameDataHash;
class InputMethodEditor;

namespace rage
{
	template<typename T>
	class atSingleton;

	template<typename T>
	class atArray;

	class RageSecurity;
	class netTime;
	class rlGamerInfo;
	struct game_skeleton;
	class scrProgramTable;
}

template<typename T>
class HashTable;

namespace big
{
	// needed for serialization of the pointers cache
#pragma pack(push, 1)
	struct gta_pointers
	{
		PVOID m_world_model_spawn_bypass;

		memory::handle m_blame_explode;

		memory::handle m_explosion_patch;

		memory::handle m_is_matchmaking_session_valid;

		memory::handle m_broadcast_patch;

		memory::handle m_creator_warp_cheat_triggered_patch;

		memory::handle m_disable_collision;

		memory::handle m_crash_trigger;

		memory::handle m_window_hook;

		memory::handle m_script_vm_patch_1;
		memory::handle m_script_vm_patch_2;
		memory::handle m_script_vm_patch_3;
		memory::handle m_script_vm_patch_4;
		memory::handle m_script_vm_patch_5;
		memory::handle m_script_vm_patch_6;

		eGameState* m_game_state;
		bool* m_is_session_started;

		CPedFactory** m_ped_factory;
		CNetworkPlayerMgr** m_network_player_mgr;
		CNetworkObjectMgr** m_network_object_mgr;
		rage::game_skeleton* m_game_skeleton;
		void (*m_nullsub)();

		functions::ptr_to_handle m_ptr_to_handle;
		functions::handle_to_ptr m_handle_to_ptr;
		rage::scrNativeRegistrationTable* m_native_registration_table;
		functions::get_native_handler m_get_native_handler;
		functions::fix_vectors m_fix_vectors;

		rage::atArray<GtaThread*>* m_script_threads;
		rage::scrProgramTable* m_script_program_table;
		functions::run_script_threads m_run_script_threads;
		std::int64_t** m_script_globals;

		CGameScriptHandlerMgr** m_script_handler_mgr;

		IDXGISwapChain** m_swapchain;

		int* m_resolution_x;
		int* m_resolution_y;

		uint32_t* m_region_code;

		uint64_t m_ocean_quads;
		float* m_gravity_level;
		functions::set_gravity_level m_set_gravity_level;

		PVOID m_native_return;
		PVOID m_ctext_file_ptr;
		PVOID m_get_label_text;

		ChatData** m_chat_data;
		ScInfo* m_sc_info;
		FriendRegistry* m_friend_registry;

		functions::get_screen_coords_for_world_coords m_get_screen_coords_for_world_coords;

		HashTable<CBaseModelInfo*>* m_model_table;
		PVOID m_get_model_info;

		PVOID m_gta_thread_start;
		PVOID m_gta_thread_kill;

		PVOID m_network_player_mgr_init;
		PVOID m_network_player_mgr_shutdown;

		functions::get_gameplay_cam_coords m_get_gameplay_cam_coords;

		functions::trigger_script_event m_trigger_script_event;

		// Bitbuffer Read/Write START
		functions::read_bitbuf_dword m_read_bitbuf_dword;
		functions::read_bitbuf_string m_read_bitbuf_string;
		functions::read_bitbuf_bool m_read_bitbuf_bool;
		functions::read_bitbuf_array m_read_bitbuf_array;
		functions::write_bitbuf_qword m_write_bitbuf_qword;
		functions::write_bitbuf_dword m_write_bitbuf_dword;
		functions::write_bitbuf_int64 m_write_bitbuf_int64;
		functions::write_bitbuf_int32 m_write_bitbuf_int32;
		functions::write_bitbuf_bool m_write_bitbuf_bool;
		functions::write_bitbuf_array m_write_bitbuf_array;
		// Bitbuffer Read/Write END

		// Received Event Signatures START
		PVOID m_received_event;
		functions::send_event_ack m_send_event_ack;
		// Received Event Signatures END

		//Sync Signatures START
		PVOID m_received_clone_create;
		PVOID m_received_clone_sync;
		PVOID m_can_apply_data;
		functions::get_sync_tree_for_type m_get_sync_tree_for_type;
		functions::get_sync_type_info m_get_sync_type_info;
		functions::get_net_object m_get_net_object;
		functions::read_bitbuffer_into_sync_tree m_read_bitbuffer_into_sync_tree;
		PVOID m_update_sync_tree;
		//Sync Signatures END

		PVOID m_receive_net_message;
		PVOID m_get_network_event_data;
		PVOID m_assign_physical_index;

		Network** m_network;

		functions::get_gamer_online_state m_get_gamer_online_state;
		functions::start_get_session_by_gamer_handle m_start_get_session_by_gamer_handle;
		functions::start_matchmaking_find_sessions m_start_matchmaking_find_sessions;
		functions::join_session_by_info m_join_session_by_info;

		functions::invite_player_by_gamer_handle m_invite_player_by_gamer_handle;
		functions::add_friend_by_gamer_handle m_add_friend_by_gamer_handle;
		functions::show_profile_by_gamer_handle m_show_profile_by_gamer_handle;

		functions::reset_network_complaints m_reset_network_complaints;

		functions::fidevice_get_device m_fidevice_get_device;
		functions::fipackfile_ctor m_fipackfile_ctor;
		functions::fipackfile_dtor m_fipackfile_dtor;
		rage::fiPackfile** m_fipackfile_instances;
		functions::fipackfile_open_archive m_fipackfile_open_archive;
		functions::fipackfile_mount m_fipackfile_mount;
		functions::fipackfile_unmount m_fipackfile_unmount;
		functions::fipackfile_close_archive m_fipackfile_close_archive;

		PVOID m_invalid_decal_crash;
		PVOID m_task_parachute_object;
		PVOID m_task_ambient_clips;

		int64_t** m_send_chat_ptr;
		functions::send_chat_message m_send_chat_message;

		PVOID m_init_native_tables;
		functions::script_vm m_script_vm;

		functions::generate_uuid m_generate_uuid;
		uint64_t* m_host_token;
		uint64_t* m_peer_id;
		rage::rlGamerInfo* m_profile_gamer_info;     // per profile gamer info
		rage::rlGamerInfo* m_player_info_gamer_info; // the gamer info that is applied to CPlayerInfo
		CCommunications** m_communications;

		PVOID m_handle_join_request;
		functions::write_join_response_data m_write_join_response_data;

		functions::queue_packet m_queue_packet;

		PVOID m_sort_session_details;

		PVOID m_process_matchmaking_find_response;

		PVOID m_serialize_join_request_message;
		PVOID m_serialize_join_request_message_2;

		functions::give_pickup_rewards m_give_pickup_rewards;
		functions::send_network_damage m_send_network_damage;
		functions::request_ragdoll m_request_ragdoll;
		functions::request_control m_request_control;
		functions::clear_ped_tasks_network m_clear_ped_tasks_network;

		functions::get_connection_peer m_get_connection_peer;
		functions::send_remove_gamer_cmd m_send_remove_gamer_cmd;
		functions::handle_remove_gamer_cmd m_handle_remove_gamer_cmd;

		PVOID m_broadcast_net_array;

		PVOID m_prepare_metric_for_sending;

		PVOID m_queue_dependency;
		PVOID m_interval_check_func;

		PVOID m_http_start_request;

		PVOID m_serialize_take_off_ped_variation_task;
		PVOID m_serialize_parachute_task;

		functions::encode_session_info m_encode_session_info;
		functions::decode_session_info m_decode_session_info;
		functions::decode_peer_info m_decode_peer_info;

		datafile_commands::SveFileObject* m_main_file_object;
		functions::load_cloud_file m_load_cloud_file;
		functions::set_as_active_cloud_file m_set_as_active_cloud_file;
		functions::save_json_data m_save_json_data;

		rage::netTime** m_network_time;
		functions::sync_network_time m_sync_network_time;

		functions::send_packet m_send_packet;
		functions::connect_to_peer m_connect_to_peer;

		PVOID m_fragment_physics_crash_2;

		functions::get_entity_attached_to m_get_entity_attached_to;

		PVOID m_received_array_update;

		PVOID m_receive_pickup;

		PVOID m_send_player_card_stats;
		bool* m_force_player_card_refresh;

		PVOID m_serialize_stats;

		PVOID m_enumerate_audio_devices;
		PVOID m_direct_sound_capture_create;
		bool* m_refresh_audio_input;
		bool* m_refresh_audio_input_2;

		PVOID m_allow_weapons_in_vehicle;

		PVOID m_taskjump_constructor;
		PVOID m_taskfall_constructor;

		PVOID m_write_vehicle_proximity_migration_data_node;
		functions::migrate_object m_migrate_object;

		const char* m_game_version;
		const char* m_online_version;

		GenericPool** m_ped_pool;
		GenericPool** m_prop_pool;
		VehiclePool*** m_vehicle_pool;

		PVOID m_netfilter_handle_message;

		functions::handle_chat_message m_handle_chat_message;

		eGameLanguage* m_language;
		functions::update_language m_update_language;

		PVOID m_model_spawn_bypass;

		functions::get_host_array_handler_by_index m_get_host_array_handler_by_index;

		PVOID m_error_message_box;
		PVOID m_error_message_box_2;

		functions::get_title_caption_error_message_box m_get_title_caption_error_message_box;

		PVOID m_send_non_physical_player_data;

		void** m_presence_data;

		CVehicleSeatMetadataMgr* m_vehicle_layout_metadata_mgr{};
		CVehicleDriveByMetadataMgr* m_driveby_metadata_mgr{};

		CBlipList* m_blip_list;
		PVOID m_timecycle_keyframe_override;

		PVOID m_allocate_memory_reliable;
		functions::connection_manager_try_free_memory m_connection_manager_try_free_memory;
		functions::remove_message_from_queue m_remove_message_from_queue;
		functions::remove_message_from_unacked_reliables m_remove_message_from_unacked_reliables;

		PVOID m_free_event_error;

		PVOID* m_draw_handler_mgr;
		PVOID m_render_ped;
		PVOID m_render_entity;
		PVOID m_render_big_ped;

		bool* m_force_relay_connections;

		functions::remove_reference m_remove_reference;

		PVOID m_read_bits_single;
		void** m_sync_data_reader_vtable;

		GenericPool** m_interior_proxy_pool;

		rage::atArray<CTrainConfig>* m_train_config_array;

		functions::activate_special_ability m_activate_special_ability;
		PVOID m_activate_special_ability_patch;

		functions::set_wanted_level m_set_wanted_level;

		rage::netEventMgr** m_net_event_manager;
		GenericPool** m_net_event_pool;
		functions::check_event_queue m_check_event_queue;
		functions::get_new_pool_item m_get_new_pool_item;
		functions::construct_door_break_event m_construct_door_break_event;
		functions::queue_network_event m_queue_network_event;

		functions::delete_ped m_delete_ped;
		functions::delete_vehicle m_delete_vehicle;
		functions::delete_object m_delete_object;

		functions::decal_manager_remove m_decal_manager_remove;
		PVOID m_decal_manager;

		bool* m_is_social_club_overlay_active;

		functions::get_ped_seat m_get_ped_seat;

		functions::received_clone_remove m_received_clone_remove;

		CWeaponInfoManager* m_weapon_info_manager;

		functions::can_create_vehicle m_can_create_vehicle;

		uintptr_t* m_cam_gameplay_director;
		functions::cam_gameplay_directory_update m_cam_gameplay_director_update;

		PVOID m_format_int;

		PVOID m_searchlight_crash;
		functions::get_searchlight m_get_searchlight;

		GenericPool** m_vehicle_allocator; // this is not a normal pool

		PVOID m_write_node_data;
		PVOID m_can_send_node_to_player;
		PVOID m_write_node;
		functions::get_sector_data m_get_sector_data;

		PVOID m_advertise_session;
		PVOID m_update_session_advertisement;
		PVOID m_unadvertise_session;
		PVOID m_send_session_detail_msg;

		PVOID m_session_request_patch;

		functions::get_peer_by_security_id m_get_peer_by_security_id;

		GameDataHash** m_game_data_hash;

		void** m_dlc_manager;
		PVOID m_get_dlc_hash;

		PVOID m_add_gamer_to_session;

		functions::set_head_blend_data m_set_head_blend_data;

		std::uint32_t* m_object_ids_offset;

		PVOID m_error_packet_memmove;

		PVOID m_create_pool_item;

		PVOID m_scope_sway_function;

		PVOID m_report_myself_sender;

		functions::create_chat_guid m_create_chat_guid;

		uint32_t* m_game_lifetime;

		functions::begin_scaleform m_begin_scaleform;

		functions::is_ped_enemies_with m_is_ped_enemies_with;
		functions::can_do_damage_to_ped m_can_do_damage_to_ped;

		bool* m_allow_keyboard_layout_change;
		InputMethodEditor* m_ime;
	};
#pragma pack(pop)
	static_assert(sizeof(gta_pointers) % 8 == 0, "Pointers are not properly aligned");
}
