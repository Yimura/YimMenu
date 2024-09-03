#pragma once
#include <rage/rlTaskStatus.hpp>
#include <script/scrNativeHandler.hpp>

class CMsgJoinResponse;
class NetworkGameFilterMatchmakingComponent;
class sCloudFile;
class CPlayerGameStateDataNode;
class CVehicleGadgetDataNode;
class CGameScriptHandlerNetComponent;
class CDoorBreakEvent;
class GenericPool;
class CGetPedSeatReturnClass;
enum eVehicleGadgetType : uint32_t;
enum class PedBones : uint16_t;
class CNetComplaintMgr;
class Network;
class CNetworkObjectMgr;
class CHeadBlendData;

namespace rage
{
	class netConnectionManager;
	class netPeerAddress;
	class netConnection;
	class netMessageQueue;
	class netQueuedMessage;
	class netConnectionPeer;
	class snMsgRemoveGamersFromSessionCmd;
	class snSession;
	class snPlayer;
	class CDynamicEntity;
	class netTimeSyncMsg;
	class snConnectToPeerTaskData;
	class snConnectToPeerTaskResult;
	class rlScHandle;
	class rlQueryPresenceAttributesContext;
	enum class eThreadState : uint32_t;
	class netArrayHandlerBase;
	class fwRefAwareBase;
	class fwEntity;
	class netGameEvent;
	class netEventMgr;
	class fiDevice;
	class fiPackfile;
	class scrNativeRegistrationTable;
	class rlSessionByGamerTaskResult;
	class SecurityPeer;
	struct rlScTaskStatus
	{
		void* pad  = 0;
		int status = 0;
		int unk    = 0;
	};
}

namespace datafile_commands
{
	class SveFileObject;
}

namespace big::functions
{
	using run_script_threads = bool (*)(uint32_t ops_to_execute);
	using get_native_handler = rage::scrNativeHandler (*)(rage::scrNativeRegistrationTable* registration_table, rage::scrNativeHash hash);
	using fix_vectors = void (*)(rage::scrNativeCallContext* call_ctx);

	using get_net_game_player = CNetGamePlayer* (*)(Player player);

	using trigger_script_event = void (*)(int event_group, int64_t* args, int arg_count, int player_bits, int event_id);

	using ptr_to_handle = Entity (*)(void*);
	using handle_to_ptr = rage::CDynamicEntity* (*)(Entity);

	using set_gravity_level = void (*)(int level);

	using write_player_game_state_data_node = bool (*)(rage::netObject* plr, CPlayerGameStateDataNode* node);

	using get_gameplay_cam_coords = Vector3 (*)();

	using get_screen_coords_for_world_coords = bool (*)(float* world_coords, float* out_x, float* out_y);

	using give_pickup_rewards = void (*)(int players, uint32_t hash);

	// Bitbuffer read/write START
	using read_bitbuf_dword  = bool (*)(rage::datBitBuffer* buffer, PVOID read, int bits);
	using read_bitbuf_string = bool (*)(rage::datBitBuffer* buffer, char* read, int bits);
	using read_bitbuf_bool   = bool (*)(rage::datBitBuffer* buffer, bool* read, int bits);
	using read_bitbuf_array  = bool (*)(rage::datBitBuffer* buffer, PVOID read, int bits, int unk);
	using write_bitbuf_qword = bool (*)(rage::datBitBuffer* buffer, uint64_t val, int bits);
	using write_bitbuf_dword = bool (*)(rage::datBitBuffer* buffer, uint32_t val, int bits);
	using write_bitbuf_int64 = bool (*)(rage::datBitBuffer* buffer, int64_t val, int bits);
	using write_bitbuf_int32 = bool (*)(rage::datBitBuffer* buffer, int32_t val, int bits);
	using write_bitbuf_bool  = bool (*)(rage::datBitBuffer* buffer, bool val, int bits);
	using write_bitbuf_array = bool (*)(rage::datBitBuffer* buffer, void* val, int bits, int unk);

	// Bitbuffer read/write END
	// Received Event Signatures START
	using send_event_ack = void (*)(rage::netEventMgr* event_manager, CNetGamePlayer* source_player, CNetGamePlayer* target_player, int event_index, int event_handled_bitset);
	// Received Event Signatures END

	//Sync signatures START
	using get_sync_type_info = const char* (*)(uint16_t sync_type, char a2);

	using get_sync_tree_for_type = rage::netSyncTree* (*)(CNetworkObjectMgr* mgr, uint16_t sync_type);

	using get_net_object = rage::netObject* (*)(CNetworkObjectMgr* mgr, int16_t id, bool can_delete_be_pending);

	using get_net_object_for_player = rage::netObject* (*)(CNetworkObjectMgr*, int16_t, CNetGamePlayer*, bool);

	using read_bitbuffer_into_sync_tree = void (*)(rage::netSyncTree* tree, uint64_t flag, uint32_t flag2, rage::datBitBuffer* buffer, uint64_t netLogStub);
	//Sync signatures END

	using reset_network_complaints = void (*)(CNetComplaintMgr* mgr);

	using fidevice_get_device = rage::fiDevice* (*)(const char* path, bool allow_root);
	using fipackfile_ctor     = rage::fiPackfile* (*)(rage::fiPackfile* this_);
	using fipackfile_dtor     = rage::fiPackfile* (*)(rage::fiPackfile* this_);
	using fipackfile_open_archive = bool (*)(rage::fiPackfile* this_, const char* archive, bool b_true, int type, intptr_t very_false);
	using fipackfile_mount         = bool (*)(rage::fiPackfile* this_, const char* mount_point);
	using fipackfile_unmount       = bool (*)(const char* mount_point);
	using fipackfile_close_archive = void (*)(rage::fiDevice* this_);

	using get_gamer_online_state = bool (*)(int profile_index, rage::rlGamerHandle* handles, uint32_t count, int* online_state, rage::rlTaskStatus* status);
	using start_get_session_by_gamer_handle = bool (*)(int profile_index, rage::rlGamerHandle* handles, int count, rage::rlSessionByGamerTaskResult* result, int unk, bool* success, rage::rlTaskStatus* state);
	using start_matchmaking_find_sessions = bool (*)(int profile_index, int available_slots, NetworkGameFilterMatchmakingComponent* m_filter, unsigned int max_sessions, rage::rlSessionInfo* result_sessions, int* result_session_count, rage::rlTaskStatus* state);
	using start_get_presence_attributes = bool (*)(int profile_index, rage::rlScHandle* handle, int num_handles, rage::rlQueryPresenceAttributesContext** contexts, int count, rage::rlScTaskStatus* state);
	using join_session_by_info = bool (*)(Network* network, rage::rlSessionInfo* info, int unk, int flags, rage::rlGamerHandle* handles, int handlecount);

	using invite_player_by_gamer_handle = bool (*)(Network* network, rage::rlGamerHandle* handles, int handlecount, const char* subject, const char* msg, rage::rlTaskStatus* state);
	using add_friend_by_gamer_handle   = void (*)(rage::rlGamerHandle* handle, const char* unk);
	using show_profile_by_gamer_handle = void (*)(rage::rlGamerHandle* handle);

	using generate_uuid = bool (*)(uint64_t* uuid);

	using get_vehicle_gadget_array_size = int (*)(eVehicleGadgetType type);

	using write_join_response_data = bool (*)(CMsgJoinResponse* response, void* data, int size, uint32_t* size_used);

	using queue_packet = bool (*)(rage::netConnectionManager* mgr, int msg_id, void* data, int size, int flags, void* unk);

	using generate_uuid = bool (*)(uint64_t* uuid);

	using send_chat_message = bool (*)(int64_t* send_chat_ptr, rage::rlGamerInfo* gamer_info, char* message, bool is_team);

	using send_network_damage = void (*)(rage::CEntity* source, rage::CEntity* target, rage::fvector3* position, int hit_component, bool override_default_damage, int weapon_type, float override_damage, int tire_index, int suspension_index, int flags, uint32_t action_result_hash, int16_t action_result_id, int action_unk, bool hit_weapon, bool hit_weapon_ammo_attachment, bool silenced, bool unk, rage::fvector3* impact_direction);
	using request_ragdoll = void (*)(uint16_t object_id);
	using request_control = void (*)(rage::netObject* net_object);

	using get_connection_peer = rage::netConnectionPeer* (*)(rage::netConnectionManager* manager, int peer_id);
	using send_remove_gamer_cmd = void (*)(rage::netConnectionManager* net_connection_mgr, rage::netPeerAddress* adde, int connection_id, rage::snMsgRemoveGamersFromSessionCmd* cmd, int flags);
	using handle_remove_gamer_cmd = void* (*)(rage::snSession* session, rage::snPlayer* origin, rage::snMsgRemoveGamersFromSessionCmd* cmd);

	using script_vm = rage::eThreadState (*)(uint64_t* stack, int64_t** scr_globals, rage::scrProgram* program, rage::scrThreadContext* ctx);

	using encode_session_info = bool (*)(rage::rlSessionInfo* info, char* buffer, int buffer_size, int* bytes_written);
	using decode_session_info = bool (*)(rage::rlSessionInfo* out_info, char* buffer, int* bytes_read);
	using decode_peer_info    = bool (*)(rage::rlGamerInfoBase* info, char* buffer, int* bytes_read);

	using load_cloud_file          = void (*)(sCloudFile** out_cloud_file, char* buffer, int size, const char* reason);
	using set_as_active_cloud_file = void (*)(datafile_commands::SveFileObject* object, sCloudFile** file);
	using save_json_data = char* (*)(datafile_commands::SveFileObject* object, int* out_length, const char* reason);

	using sync_network_time = bool (*)(rage::netConnectionManager* mgr, rage::netPeerAddress* addr, int connection_id, rage::netTimeSyncMsg* msg, int flags);
	using send_packet = bool (*)(rage::netConnectionManager* mgr, rage::netPeerAddress* adde, int connection_id, void* data, int size, int flags);
	using connect_to_peer = bool (*)(rage::netConnectionManager* mgr, rage::rlGamerHandle* handle, rage::rlGamerInfoBase* gamer_info, rage::snConnectToPeerTaskData* data, rage::snConnectToPeerTaskResult* result, rage::rlTaskStatus* status);

	using clear_ped_tasks_network = void (*)(CPed* ped, bool immediately);

	using get_next_carriage      = void* (*)(void* carriage);
	using get_entity_attached_to = rage::CDynamicEntity* (*)(rage::CDynamicEntity* entity);

	using migrate_object = void (*)(CNetGamePlayer* player, rage::netObject* object, int type);

	using handle_chat_message = void (*)(void* chat_data, void*, rage::rlGamerHandle* handle, const char* text, bool is_team);

	using update_language = void (*)(bool);

	using get_host_array_handler_by_index = rage::netArrayHandlerBase* (*)(CGameScriptHandlerNetComponent* component, int index);

	using get_title_caption_error_message_box = const wchar_t* (*)(rage::joaat_t joaated_error_code);

	using update_presence_attribute_int = void (*)(void* presence_data, int profile_index, char* attr, uint64_t value);
	using update_presence_attribute_string = void (*)(void* presence_data, int profile_index, char* attr, char* value);

	using connection_manager_try_free_memory = void (*)(rage::netConnectionManager* mgr);
	using remove_message_from_queue          = void (*)(rage::netMessageQueue* queue, rage::netQueuedMessage* message);
	using remove_message_from_unacked_reliables = void (*)(void* list, uint16_t* unk);

	using remove_reference = void (*)(rage::fwRefAwareBase* object, void* reference);

	using activate_special_ability = void (*)(CVehicle* vehicle, int special_ability_type);
	using set_wanted_level = void (*)(CNetGamePlayer* player, int wanted_level, int unk, std::uint8_t unk_player_id);

	using check_event_queue          = bool (*)(rage::netEventMgr* mgr, bool free);
	using get_new_pool_item          = void* (*)(GenericPool* pool);
	using construct_door_break_event = CDoorBreakEvent* (*)(CDoorBreakEvent* _this);
	using queue_network_event        = void (*)(rage::netEventMgr* mgr, rage::netGameEvent* event);

	using delete_ped     = bool (*)(CPed* ped);
	using delete_vehicle = bool (*)(CVehicle* veh);
	using delete_object  = bool (*)(CObject* object, bool unk);

	using decal_manager_remove = void (*)(PVOID manager, rage::fwEntity*, DWORD a3, DWORD64 a4, DWORD ignore_bitset);

	using remove_player_from_sender_list = bool (*)(void* list, uint64_t* rockstar_id);

	using get_ped_seat = CGetPedSeatReturnClass* (*)(PVOID seat_info, CPed* ped);

	using received_clone_remove = void (*)(CNetworkObjectMgr*, CNetGamePlayer*, CNetGamePlayer*, int16_t, uint32_t);

	using can_create_vehicle = bool (*)();


	using cam_gameplay_directory_update = bool (*)(uintptr_t this_);

	using get_searchlight = void* (*) (CPed*);

	using get_sector_data = void (*) (rage::fvector3* coords, std::uint16_t* x, std::uint16_t* y, std::uint16_t* z, rage::fvector3* sector_pos);

	using get_peer_by_security_id = rage::SecurityPeer*(*)(int id);

	using set_head_blend_data = void(*)(CPed* ped, CHeadBlendData* data);

	using create_chat_guid = void (*)(GUID* guid);

	using begin_scaleform = bool (*)(uint32_t* scaleform, const char* method);

	using is_ped_enemies_with = bool (*)(CPedIntelligence* from, CPed* target, bool check_relationship, bool skip_friend_check, bool skip_combat_task_check);
	using can_do_damage_to_ped = bool (*)(CPed* from, CWeaponInfo* current_weapon, CPed* target);

	using get_last_keyboard_state = KeyboardState (*)();
}
