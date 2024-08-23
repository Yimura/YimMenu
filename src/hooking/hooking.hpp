#pragma once
#include "MinHook.h"
#include "detour_hook.hpp"
#include "vmt_hook.hpp"
#include "vtable_hook.hpp"
#include "call_hook.hpp"

#include <gta/enums.hpp>
#include <network/netConnection.hpp> // cannot stub this

class CPlayerGamerDataNode;
class CPlayerGameStateDataNode;
class CPedInventoryDataNode;
class CDynamicEntityGameStateDataNode;
class CVehicleGadgetDataNode;
class CPlayerCameraDataNode;
class CJoinRequestContext;
class SessionSortEntry;
class RemoteGamerInfoMsg;
class CMsgTextMessage;
class CNetGamePlayerDataMsg;
class MatchmakingAttributes;
class CNetworkIncrementStatEvent;
class CScriptedGameEvent;
class NetworkGameFilterMatchmakingComponent;
class ClonedTakeOffPedVariationInfo;
class CPlayerCardStats;
class CStatsSerializationContext;
class CPlayerCreationDataNode;
class CPlayerAppearanceDataNode;
class CFoundDevice;
class IDirectSoundCapture;
class CVehicleProximityMigrationDataNode;
class CNonPhysicalPlayerData;
class TimecycleKeyframeData;
class CPedTaskSpecificDataNode;
class CPedTaskSequenceDataNode;
class Network;
class GtaThread;
class CNetworkPlayerMgr;
class CNetworkObjectMgr;
class CPhysicalScriptGameStateDataNode;
class MatchmakingId;
class CMsgJoinRequest;
class GenericPool;

enum class eAckCode : uint32_t;

namespace rage
{
	class snSession;
	class JSONNode;
	class netArrayHandlerBase;
	class CEventNetwork;
	class CSyncDataBase;
	class rlGamerHandle;
	class netConnectionManager;
	class datBitBuffer;
	class rlMetric;
	class rlTaskStatus;
	class netEventMgr;
	class json_serializer;
	class netGameEvent;
	class netSyncDataNode;
	class rlSessionDetailMsg;
	class netEvent;
}

namespace big
{
	struct hooks
	{
		static bool run_script_threads(uint32_t ops_to_execute);

		static constexpr auto swapchain_num_funcs           = 19;
		static constexpr auto swapchain_present_index       = 8;
		static constexpr auto swapchain_resizebuffers_index = 13;
		static HRESULT swapchain_present(IDXGISwapChain* this_, UINT sync_interval, UINT flags);
		static HRESULT swapchain_resizebuffers(IDXGISwapChain* this_, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swapchain_flags);

		static LRESULT wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

		static const char* get_label_text(void* unk, const char* label);

		static GtaThread* gta_thread_start(unsigned int** a1, unsigned int a2);
		static rage::eThreadState gta_thread_kill(GtaThread* thread);
		static bool init_native_tables(rage::scrProgram* program);
		static rage::eThreadState script_vm(uint64_t* start_stack, uint64_t** scr_globals, rage::scrProgram* program, rage::scrThreadContext* ctx);

		static bool network_player_mgr_init(CNetworkPlayerMgr* _this, uint64_t a2, uint32_t a3, uint32_t a4[4]);
		static void network_player_mgr_shutdown(CNetworkPlayerMgr* _this);

		static bool fragment_physics_crash_2(float* a1, float* a2);

		static void received_event(rage::netEventMgr* event_manager, CNetGamePlayer* source_player, CNetGamePlayer* target_player, uint16_t event_id, int event_index, int event_handled_bitset, int unk, rage::datBitBuffer* bit_buffer);

		// these two aren't actually hooks, just helper functions for hooks
		static bool scripted_game_event(CScriptedGameEvent* scripted_game_event, CNetGamePlayer* player);

		static bool receive_net_message(void* a1, rage::netConnectionManager* mgr, rage::netEvent* event);
		static rage::CEventNetwork* get_network_event_data(int64_t unk, rage::CEventNetwork* net_event);

		static void* assign_physical_index(CNetworkPlayerMgr* netPlayerMgr, CNetGamePlayer* player, uint8_t new_index);

		//SYNC
		static void received_clone_create(CNetworkObjectMgr* mgr, CNetGamePlayer* src, CNetGamePlayer* dst, eNetObjType object_type, int32_t object_id, int32_t object_flag, rage::datBitBuffer* buffer, int32_t timestamp);
		static eAckCode received_clone_sync(CNetworkObjectMgr* mgr, CNetGamePlayer* src, CNetGamePlayer* dst, eNetObjType object_type, uint16_t object_id, rage::datBitBuffer* bufer, uint16_t unk, uint32_t timestamp);
		static bool can_apply_data(rage::netSyncTree* tree, rage::netObject* object);

		static void update_sync_tree(rage::netSyncTree* _this, rage::netObject* object, uint32_t flags, uint32_t timestamp, bool a5);

		static void invalid_decal(uintptr_t a1, int a2);
		static int task_parachute_object(uint64_t _this, int a2, int a3);
		static int task_ambient_clips(uint64_t _this, int a2, int a3);

		static bool update_presence_attribute_int(void* presence_data, int profile_index, char* attr, uint64_t value);
		static bool update_presence_attribute_string(void* presence_data, int profile_index, char* attr, char* value);

		static bool handle_join_request(Network* network, rage::snSession* session, rage::rlGamerInfo* player_info, CJoinRequestContext* ctx, BOOL is_transition_session);

		static bool sort_session_details(SessionSortEntry* e1, SessionSortEntry* e2);

		static bool send_chat_message(void* team_mgr, rage::rlGamerInfo* local_gamer_info, char* message, bool is_team);

		static bool process_matchmaking_find_response(void* _this, void* unused, rage::JSONNode* node, int* unk);

		static bool serialize_join_request_message(RemoteGamerInfoMsg* info, void* data, int size, int* bits_serialized);
		static bool serialize_join_request_message_2(CMsgJoinRequest* msg, void* buf, int size, int* bits_serialized);

		static bool start_matchmaking_find_sessions(int profile_index, int available_slots, NetworkGameFilterMatchmakingComponent* filter, unsigned int max_sessions, rage::rlSessionInfo* results, int* num_sessions_found, rage::rlTaskStatus* status);

		static unsigned int broadcast_net_array(rage::netArrayHandlerBase* _this, CNetGamePlayer* target, rage::datBitBuffer* bit_buffer, uint16_t counter, uint32_t* elem_start, bool silent);

		static void serialize_take_off_ped_variation_task(ClonedTakeOffPedVariationInfo* info, rage::CSyncDataBase* serializer);
		static void serialize_parachute_task(__int64 info, rage::CSyncDataBase* serializer);

		static int nt_query_virtual_memory(void* _this, HANDLE handle, PVOID base_addr, int info_class, MEMORY_BASIC_INFORMATION* info, int size, size_t* return_len);
		static int queue_dependency(void* a1, int a2, int64_t dependency);

		static bool prepare_metric_for_sending(rage::json_serializer* bit_buffer, int unk, int time, rage::rlMetric* metric);
		static bool http_start_request(void* request, const char* uri);

		static bool received_array_update(rage::netArrayHandlerBase* array, CNetGamePlayer* sender, rage::datBitBuffer* buffer, int size, int16_t cycle);

		static bool receive_pickup(rage::netObject* netobject, void* unk, CPed* ped);

		static rage::netGameEvent* send_player_card_stats(rage::netGameEvent* a1, CPlayerCardStats* stats);
		static void serialize_stats(CStatsSerializationContext* context, rage::joaat_t* stats, uint32_t stat_count);

		static __int64 task_jump_constructor(uint64_t a1, int a2);

		static void* task_fall_constructor(uint64_t a1, int a2);

		static CBaseModelInfo* get_model_info(rage::joaat_t hash, uint32_t* a2);

		static int enumerate_audio_devices(CFoundDevice* found_devices, int count, int flags);
		static HRESULT direct_sound_capture_create(GUID* guid, IDirectSoundCapture** sound, void* unknown);

		static void write_vehicle_proximity_migration_data_node(rage::netObject* veh, CVehicleProximityMigrationDataNode* node);

		static int netfilter_handle_message(__int64 filter, char* message, int flags);

		static void log_error_message_box(rage::joaat_t joaated_error_code, bool a2);
		static void log_error_message_box_2(rage::joaat_t joaated_error_code);

		static bool send_non_physical_player_data(CNetGamePlayer* player, __int64 message, int flags, void* a4, CNetGamePlayer* a5);

		static void update_timecycle_keyframe_data(int64_t timecycleManager, TimecycleKeyframeData* timecycleKeyframeData);

		static void* allocate_memory_reliable(rage::netConnection* cxn, int required_memory);

		static void* render_ped(__int64 renderer, CPed* ped, __int64 a3, __int64 a4);
		static void render_entity(__int64 renderer, rage::fwEntity* entity, int unk, bool a4);
		static __int64 render_big_ped(__int64 renderer, CPed* ped, __int64 a3, __int64 a4);

		static bool read_bits_single(void* data, int* out_value, int size, int offset);

		static void received_clone_remove(CNetworkObjectMgr* mgr, CNetGamePlayer* src, CNetGamePlayer* dst, int16_t object_id, uint32_t ownership_token);

		static bool sync_reader_serialize_dword(void* _this, uint32_t* dword, int size);
		static bool sync_reader_serialize_word(void* _this, uint16_t* word, int size);
		static bool sync_reader_serialize_byte(void* _this, uint8_t* byte, int size);
		static bool sync_reader_serialize_int32(void* _this, int32_t* i, int size);
		static bool sync_reader_serialize_int16(void* _this, int16_t* i, int size);
		static bool sync_reader_serialize_signed_byte(void* _this, int8_t* i, int size);
		static bool sync_reader_serialize_bool(void* _this, bool* flag, int size);
		static bool sync_reader_serialize_signed_float(void* _this, float* flt, float divisor, int size);
		static bool sync_reader_serialize_float(void* _this, float* flt, float divisor, int size);
		static bool sync_reader_serialize_net_id(void* _this, uint16_t* id);
		static bool sync_reader_serialize_vec3(void* _this, rage::fvector3* vec, float divisor, int size);
		static bool sync_reader_serialize_vec3_signed(void* _this, rage::fvector3* vec, float divisor, int size);
		static bool sync_reader_serialize_array(void* _this, void* array, int size);

		static bool can_create_vehicle();

		static bool aimbot_cam_gameplay_director_update(uintptr_t this_);

		static void format_int(int64_t integer_to_format, char* format_string, size_t size_always_64, bool use_commas);

		static void write_node_data(void* data_node, rage::netObject* net_object, rage::datBitBuffer* buffer, void* log, bool update);
		static bool can_send_node_to_player(void* node, rage::netObject* object, std::uint8_t player, int sync_type, int a5, int a6);
		static bool write_node(rage::netSyncDataNode* node, int sync_type, int a3, rage::netObject* object, rage::datBitBuffer* buffer, int a6, void* log, std::uint8_t player, int* a9, int* a10);
		static void searchlight_crash(void* a1, CPed* ped);

		static bool advertise_session(int profile_index, int num_slots, int available_slots, MatchmakingAttributes* data, std::uint64_t session_id, rage::rlSessionInfo* info, MatchmakingId* out_id, rage::rlTaskStatus* status);
		static bool update_session_advertisement(int profile_index, MatchmakingId* id, int num_slots, int available_slots, rage::rlSessionInfo* info, MatchmakingAttributes* data, rage::rlTaskStatus* status);
		static bool unadvertise_session(int profile_index, MatchmakingId* id, rage::rlTaskStatus* status);
		static void send_session_detail_msg(rage::netConnectionManager* mgr, rage::netConnection::InFrame* request_frame, rage::rlSessionDetailMsg* msg);

		static std::uint32_t get_dlc_hash(void* mgr, std::uint32_t seed);
		static bool add_gamer_to_session(rage::netConnectionManager* mgr, std::uint32_t msg_id, int* req_id, RemoteGamerInfoMsg* info, int flags, void* a6);

		static void error_packet_memmove(void* dst, void* src, int size);

		static void* create_pool_item(GenericPool* pool);
	};

	class minhook_keepalive
	{
	public:
		minhook_keepalive()
		{
			MH_Initialize();
		}
		~minhook_keepalive()
		{
			MH_Uninitialize();
		}
	};

	class hooking
	{
		friend hooks;

	public:
		explicit hooking();
		~hooking();

		void enable();
		void disable();

		class detour_hook_helper
		{
			friend hooking;

			using ret_ptr_fn = std::function<void*()>;

			ret_ptr_fn m_on_hooking_available = nullptr;

			detour_hook* m_detour_hook;

			void enable_hook_if_hooking_is_already_running();

			template<auto detour_function>
			struct hook_to_detour_hook_helper
			{
				static inline detour_hook m_detour_hook;
			};

		public:
			template<auto detour_function>
			static void add(const std::string& name, void* target)
			{
				hook_to_detour_hook_helper<detour_function>::m_detour_hook.set_instance(name, target, detour_function);

				detour_hook_helper d{};
				d.m_detour_hook = &hook_to_detour_hook_helper<detour_function>::m_detour_hook;

				d.enable_hook_if_hooking_is_already_running();

				m_detour_hook_helpers.push_back(d);
			}

			template<auto detour_function>
			static void* add_lazy(const std::string& name, detour_hook_helper::ret_ptr_fn on_hooking_available)
			{
				hook_to_detour_hook_helper<detour_function>::m_detour_hook.set_instance(name, detour_function);

				detour_hook_helper d{};
				d.m_detour_hook          = &hook_to_detour_hook_helper<detour_function>::m_detour_hook;
				d.m_on_hooking_available = on_hooking_available;

				d.enable_hook_if_hooking_is_already_running();

				m_detour_hook_helpers.push_back(d);

				return nullptr;
			}

			~detour_hook_helper();
		};

		template<auto detour_function>
		static auto get_original()
		{
			return detour_hook_helper::hook_to_detour_hook_helper<detour_function>::m_detour_hook.get_original<decltype(detour_function)>();
		}

	private:
		bool m_enabled{};
		minhook_keepalive m_minhook_keepalive;

		vmt_hook m_swapchain_hook;
		vtable_hook m_sync_data_reader_hook;

		call_hook m_error_packet_memmove_hook;

		WNDPROC m_og_wndproc = nullptr;

		static inline std::vector<detour_hook_helper> m_detour_hook_helpers;
	};

	inline hooking* g_hooking{};
}
