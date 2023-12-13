#include "lua_native_binding.hpp"
#include "natives.hpp"

namespace lua::native
{
	static void LUA_NATIVE_CUTSCENE_REQUEST_CUTSCENE(const char* cutsceneName, int flags)
	{
		CUTSCENE::REQUEST_CUTSCENE(cutsceneName, flags);
	}

	static void LUA_NATIVE_CUTSCENE_REQUEST_CUTSCENE_WITH_PLAYBACK_LIST(const char* cutsceneName, int playbackFlags, int flags)
	{
		CUTSCENE::REQUEST_CUTSCENE_WITH_PLAYBACK_LIST(cutsceneName, playbackFlags, flags);
	}

	static void LUA_NATIVE_CUTSCENE_REMOVE_CUTSCENE()
	{
		CUTSCENE::REMOVE_CUTSCENE();
	}

	static bool LUA_NATIVE_CUTSCENE_HAS_CUTSCENE_LOADED()
	{
		auto retval = (bool)CUTSCENE::HAS_CUTSCENE_LOADED();
		return retval;
	}

	static bool LUA_NATIVE_CUTSCENE_HAS_THIS_CUTSCENE_LOADED(const char* cutsceneName)
	{
		auto retval = (bool)CUTSCENE::HAS_THIS_CUTSCENE_LOADED(cutsceneName);
		return retval;
	}

	static void LUA_NATIVE_CUTSCENE_SET_SCRIPT_CAN_START_CUTSCENE(int threadId)
	{
		CUTSCENE::SET_SCRIPT_CAN_START_CUTSCENE(threadId);
	}

	static bool LUA_NATIVE_CUTSCENE_CAN_REQUEST_ASSETS_FOR_CUTSCENE_ENTITY()
	{
		auto retval = (bool)CUTSCENE::CAN_REQUEST_ASSETS_FOR_CUTSCENE_ENTITY();
		return retval;
	}

	static bool LUA_NATIVE_CUTSCENE_IS_CUTSCENE_PLAYBACK_FLAG_SET(int flag)
	{
		auto retval = (bool)CUTSCENE::IS_CUTSCENE_PLAYBACK_FLAG_SET(flag);
		return retval;
	}

	static void LUA_NATIVE_CUTSCENE_SET_CUTSCENE_ENTITY_STREAMING_FLAGS(const char* cutsceneEntName, int p1, int p2)
	{
		CUTSCENE::SET_CUTSCENE_ENTITY_STREAMING_FLAGS(cutsceneEntName, p1, p2);
	}

	static void LUA_NATIVE_CUTSCENE_REQUEST_CUT_FILE(const char* cutsceneName)
	{
		CUTSCENE::REQUEST_CUT_FILE(cutsceneName);
	}

	static bool LUA_NATIVE_CUTSCENE_HAS_CUT_FILE_LOADED(const char* cutsceneName)
	{
		auto retval = (bool)CUTSCENE::HAS_CUT_FILE_LOADED(cutsceneName);
		return retval;
	}

	static void LUA_NATIVE_CUTSCENE_REMOVE_CUT_FILE(const char* cutsceneName)
	{
		CUTSCENE::REMOVE_CUT_FILE(cutsceneName);
	}

	static int LUA_NATIVE_CUTSCENE_GET_CUT_FILE_CONCAT_COUNT(const char* cutsceneName)
	{
		auto retval = CUTSCENE::GET_CUT_FILE_CONCAT_COUNT(cutsceneName);
		return retval;
	}

	static void LUA_NATIVE_CUTSCENE_START_CUTSCENE(int flags)
	{
		CUTSCENE::START_CUTSCENE(flags);
	}

	static void LUA_NATIVE_CUTSCENE_START_CUTSCENE_AT_COORDS(float x, float y, float z, int flags)
	{
		CUTSCENE::START_CUTSCENE_AT_COORDS(x, y, z, flags);
	}

	static void LUA_NATIVE_CUTSCENE_STOP_CUTSCENE(bool p0)
	{
		CUTSCENE::STOP_CUTSCENE(p0);
	}

	static void LUA_NATIVE_CUTSCENE_STOP_CUTSCENE_IMMEDIATELY()
	{
		CUTSCENE::STOP_CUTSCENE_IMMEDIATELY();
	}

	static void LUA_NATIVE_CUTSCENE_SET_CUTSCENE_ORIGIN(float x, float y, float z, float p3, int p4)
	{
		CUTSCENE::SET_CUTSCENE_ORIGIN(x, y, z, p3, p4);
	}

	static void LUA_NATIVE_CUTSCENE_SET_CUTSCENE_ORIGIN_AND_ORIENTATION(float x1, float y1, float z1, float x2, float y2, float z2, int p6)
	{
		CUTSCENE::SET_CUTSCENE_ORIGIN_AND_ORIENTATION(x1, y1, z1, x2, y2, z2, p6);
	}

	static int LUA_NATIVE_CUTSCENE_GET_CUTSCENE_TIME()
	{
		auto retval = CUTSCENE::GET_CUTSCENE_TIME();
		return retval;
	}

	static int LUA_NATIVE_CUTSCENE_GET_CUTSCENE_TOTAL_DURATION()
	{
		auto retval = CUTSCENE::GET_CUTSCENE_TOTAL_DURATION();
		return retval;
	}

	static int LUA_NATIVE_CUTSCENE_GET_CUTSCENE_END_TIME()
	{
		auto retval = CUTSCENE::GET_CUTSCENE_END_TIME();
		return retval;
	}

	static int LUA_NATIVE_CUTSCENE_GET_CUTSCENE_PLAY_DURATION()
	{
		auto retval = CUTSCENE::GET_CUTSCENE_PLAY_DURATION();
		return retval;
	}

	static bool LUA_NATIVE_CUTSCENE_WAS_CUTSCENE_SKIPPED()
	{
		auto retval = (bool)CUTSCENE::WAS_CUTSCENE_SKIPPED();
		return retval;
	}

	static bool LUA_NATIVE_CUTSCENE_HAS_CUTSCENE_FINISHED()
	{
		auto retval = (bool)CUTSCENE::HAS_CUTSCENE_FINISHED();
		return retval;
	}

	static bool LUA_NATIVE_CUTSCENE_IS_CUTSCENE_ACTIVE()
	{
		auto retval = (bool)CUTSCENE::IS_CUTSCENE_ACTIVE();
		return retval;
	}

	static bool LUA_NATIVE_CUTSCENE_IS_CUTSCENE_PLAYING()
	{
		auto retval = (bool)CUTSCENE::IS_CUTSCENE_PLAYING();
		return retval;
	}

	static int LUA_NATIVE_CUTSCENE_GET_CUTSCENE_SECTION_PLAYING()
	{
		auto retval = CUTSCENE::GET_CUTSCENE_SECTION_PLAYING();
		return retval;
	}

	static Entity LUA_NATIVE_CUTSCENE_GET_ENTITY_INDEX_OF_CUTSCENE_ENTITY(const char* cutsceneEntName, Hash modelHash)
	{
		auto retval = CUTSCENE::GET_ENTITY_INDEX_OF_CUTSCENE_ENTITY(cutsceneEntName, modelHash);
		return retval;
	}

	static int LUA_NATIVE_CUTSCENE_GET_CUTSCENE_CONCAT_SECTION_PLAYING()
	{
		auto retval = CUTSCENE::GET_CUTSCENE_CONCAT_SECTION_PLAYING();
		return retval;
	}

	static bool LUA_NATIVE_CUTSCENE_IS_CUTSCENE_AUTHORIZED(const char* cutsceneName)
	{
		auto retval = (bool)CUTSCENE::IS_CUTSCENE_AUTHORIZED(cutsceneName);
		return retval;
	}

	static int LUA_NATIVE_CUTSCENE_DOES_CUTSCENE_HANDLE_EXIST(int cutsceneHandle)
	{
		auto retval = CUTSCENE::DOES_CUTSCENE_HANDLE_EXIST(cutsceneHandle);
		return retval;
	}

	static void LUA_NATIVE_CUTSCENE_REGISTER_ENTITY_FOR_CUTSCENE(Ped cutscenePed, const char* cutsceneEntName, int p2, Hash modelHash, int p4)
	{
		CUTSCENE::REGISTER_ENTITY_FOR_CUTSCENE(cutscenePed, cutsceneEntName, p2, modelHash, p4);
	}

	static Entity LUA_NATIVE_CUTSCENE_GET_ENTITY_INDEX_OF_REGISTERED_ENTITY(const char* cutsceneEntName, Hash modelHash)
	{
		auto retval = CUTSCENE::GET_ENTITY_INDEX_OF_REGISTERED_ENTITY(cutsceneEntName, modelHash);
		return retval;
	}

	static void LUA_NATIVE_CUTSCENE_SET_VEHICLE_MODEL_PLAYER_WILL_EXIT_SCENE(Hash modelHash)
	{
		CUTSCENE::SET_VEHICLE_MODEL_PLAYER_WILL_EXIT_SCENE(modelHash);
	}

	static void LUA_NATIVE_CUTSCENE_SET_CUTSCENE_TRIGGER_AREA(float x1, float y1, float z1, float x2, float y2, float z2)
	{
		CUTSCENE::SET_CUTSCENE_TRIGGER_AREA(x1, y1, z1, x2, y2, z2);
	}

	static bool LUA_NATIVE_CUTSCENE_CAN_SET_ENTER_STATE_FOR_REGISTERED_ENTITY(const char* cutsceneEntName, Hash modelHash)
	{
		auto retval = (bool)CUTSCENE::CAN_SET_ENTER_STATE_FOR_REGISTERED_ENTITY(cutsceneEntName, modelHash);
		return retval;
	}

	static bool LUA_NATIVE_CUTSCENE_CAN_SET_EXIT_STATE_FOR_REGISTERED_ENTITY(const char* cutsceneEntName, Hash modelHash)
	{
		auto retval = (bool)CUTSCENE::CAN_SET_EXIT_STATE_FOR_REGISTERED_ENTITY(cutsceneEntName, modelHash);
		return retval;
	}

	static bool LUA_NATIVE_CUTSCENE_CAN_SET_EXIT_STATE_FOR_CAMERA(bool p0)
	{
		auto retval = (bool)CUTSCENE::CAN_SET_EXIT_STATE_FOR_CAMERA(p0);
		return retval;
	}

	static void LUA_NATIVE_CUTSCENE_SET_PAD_CAN_SHAKE_DURING_CUTSCENE(bool toggle)
	{
		CUTSCENE::SET_PAD_CAN_SHAKE_DURING_CUTSCENE(toggle);
	}

	static void LUA_NATIVE_CUTSCENE_SET_CUTSCENE_FADE_VALUES(bool p0, bool p1, bool p2, bool p3)
	{
		CUTSCENE::SET_CUTSCENE_FADE_VALUES(p0, p1, p2, p3);
	}

	static void LUA_NATIVE_CUTSCENE_SET_CUTSCENE_MULTIHEAD_FADE(bool p0, bool p1, bool p2, bool p3)
	{
		CUTSCENE::SET_CUTSCENE_MULTIHEAD_FADE(p0, p1, p2, p3);
	}

	static void LUA_NATIVE_CUTSCENE_SET_CUTSCENE_MULTIHEAD_FADE_MANUAL(bool p0)
	{
		CUTSCENE::SET_CUTSCENE_MULTIHEAD_FADE_MANUAL(p0);
	}

	static bool LUA_NATIVE_CUTSCENE_IS_MULTIHEAD_FADE_UP()
	{
		auto retval = (bool)CUTSCENE::IS_MULTIHEAD_FADE_UP();
		return retval;
	}

	static void LUA_NATIVE_CUTSCENE_NETWORK_SET_MOCAP_CUTSCENE_CAN_BE_SKIPPED(bool p0)
	{
		CUTSCENE::NETWORK_SET_MOCAP_CUTSCENE_CAN_BE_SKIPPED(p0);
	}

	static void LUA_NATIVE_CUTSCENE_SET_CAR_GENERATORS_CAN_UPDATE_DURING_CUTSCENE(bool p0)
	{
		CUTSCENE::SET_CAR_GENERATORS_CAN_UPDATE_DURING_CUTSCENE(p0);
	}

	static bool LUA_NATIVE_CUTSCENE_CAN_USE_MOBILE_PHONE_DURING_CUTSCENE()
	{
		auto retval = (bool)CUTSCENE::CAN_USE_MOBILE_PHONE_DURING_CUTSCENE();
		return retval;
	}

	static void LUA_NATIVE_CUTSCENE_SET_CUTSCENE_CAN_BE_SKIPPED(bool p0)
	{
		CUTSCENE::SET_CUTSCENE_CAN_BE_SKIPPED(p0);
	}

	static void LUA_NATIVE_CUTSCENE_SET_CAN_DISPLAY_MINIMAP_DURING_CUTSCENE_THIS_UPDATE()
	{
		CUTSCENE::SET_CAN_DISPLAY_MINIMAP_DURING_CUTSCENE_THIS_UPDATE();
	}

	static void LUA_NATIVE_CUTSCENE_SET_CUTSCENE_PED_COMPONENT_VARIATION(const char* cutsceneEntName, int componentId, int drawableId, int textureId, Hash modelHash)
	{
		CUTSCENE::SET_CUTSCENE_PED_COMPONENT_VARIATION(cutsceneEntName, componentId, drawableId, textureId, modelHash);
	}

	static void LUA_NATIVE_CUTSCENE_SET_CUTSCENE_PED_COMPONENT_VARIATION_FROM_PED(const char* cutsceneEntName, Ped ped, Hash modelHash)
	{
		CUTSCENE::SET_CUTSCENE_PED_COMPONENT_VARIATION_FROM_PED(cutsceneEntName, ped, modelHash);
	}

	static bool LUA_NATIVE_CUTSCENE_DOES_CUTSCENE_ENTITY_EXIST(const char* cutsceneEntName, Hash modelHash)
	{
		auto retval = (bool)CUTSCENE::DOES_CUTSCENE_ENTITY_EXIST(cutsceneEntName, modelHash);
		return retval;
	}

	static void LUA_NATIVE_CUTSCENE_SET_CUTSCENE_PED_PROP_VARIATION(const char* cutsceneEntName, int componentId, int drawableId, int textureId, Hash modelHash)
	{
		CUTSCENE::SET_CUTSCENE_PED_PROP_VARIATION(cutsceneEntName, componentId, drawableId, textureId, modelHash);
	}

	static bool LUA_NATIVE_CUTSCENE_HAS_CUTSCENE_CUT_THIS_FRAME()
	{
		auto retval = (bool)CUTSCENE::HAS_CUTSCENE_CUT_THIS_FRAME();
		return retval;
	}

	void init_native_binding_CUTSCENE(sol::state& L)
	{
		auto CUTSCENE = L["CUTSCENE"].get_or_create<sol::table>();
		CUTSCENE.set_function("REQUEST_CUTSCENE", LUA_NATIVE_CUTSCENE_REQUEST_CUTSCENE);
		CUTSCENE.set_function("REQUEST_CUTSCENE_WITH_PLAYBACK_LIST", LUA_NATIVE_CUTSCENE_REQUEST_CUTSCENE_WITH_PLAYBACK_LIST);
		CUTSCENE.set_function("REMOVE_CUTSCENE", LUA_NATIVE_CUTSCENE_REMOVE_CUTSCENE);
		CUTSCENE.set_function("HAS_CUTSCENE_LOADED", LUA_NATIVE_CUTSCENE_HAS_CUTSCENE_LOADED);
		CUTSCENE.set_function("HAS_THIS_CUTSCENE_LOADED", LUA_NATIVE_CUTSCENE_HAS_THIS_CUTSCENE_LOADED);
		CUTSCENE.set_function("SET_SCRIPT_CAN_START_CUTSCENE", LUA_NATIVE_CUTSCENE_SET_SCRIPT_CAN_START_CUTSCENE);
		CUTSCENE.set_function("CAN_REQUEST_ASSETS_FOR_CUTSCENE_ENTITY", LUA_NATIVE_CUTSCENE_CAN_REQUEST_ASSETS_FOR_CUTSCENE_ENTITY);
		CUTSCENE.set_function("IS_CUTSCENE_PLAYBACK_FLAG_SET", LUA_NATIVE_CUTSCENE_IS_CUTSCENE_PLAYBACK_FLAG_SET);
		CUTSCENE.set_function("SET_CUTSCENE_ENTITY_STREAMING_FLAGS", LUA_NATIVE_CUTSCENE_SET_CUTSCENE_ENTITY_STREAMING_FLAGS);
		CUTSCENE.set_function("REQUEST_CUT_FILE", LUA_NATIVE_CUTSCENE_REQUEST_CUT_FILE);
		CUTSCENE.set_function("HAS_CUT_FILE_LOADED", LUA_NATIVE_CUTSCENE_HAS_CUT_FILE_LOADED);
		CUTSCENE.set_function("REMOVE_CUT_FILE", LUA_NATIVE_CUTSCENE_REMOVE_CUT_FILE);
		CUTSCENE.set_function("GET_CUT_FILE_CONCAT_COUNT", LUA_NATIVE_CUTSCENE_GET_CUT_FILE_CONCAT_COUNT);
		CUTSCENE.set_function("START_CUTSCENE", LUA_NATIVE_CUTSCENE_START_CUTSCENE);
		CUTSCENE.set_function("START_CUTSCENE_AT_COORDS", LUA_NATIVE_CUTSCENE_START_CUTSCENE_AT_COORDS);
		CUTSCENE.set_function("STOP_CUTSCENE", LUA_NATIVE_CUTSCENE_STOP_CUTSCENE);
		CUTSCENE.set_function("STOP_CUTSCENE_IMMEDIATELY", LUA_NATIVE_CUTSCENE_STOP_CUTSCENE_IMMEDIATELY);
		CUTSCENE.set_function("SET_CUTSCENE_ORIGIN", LUA_NATIVE_CUTSCENE_SET_CUTSCENE_ORIGIN);
		CUTSCENE.set_function("SET_CUTSCENE_ORIGIN_AND_ORIENTATION", LUA_NATIVE_CUTSCENE_SET_CUTSCENE_ORIGIN_AND_ORIENTATION);
		CUTSCENE.set_function("GET_CUTSCENE_TIME", LUA_NATIVE_CUTSCENE_GET_CUTSCENE_TIME);
		CUTSCENE.set_function("GET_CUTSCENE_TOTAL_DURATION", LUA_NATIVE_CUTSCENE_GET_CUTSCENE_TOTAL_DURATION);
		CUTSCENE.set_function("GET_CUTSCENE_END_TIME", LUA_NATIVE_CUTSCENE_GET_CUTSCENE_END_TIME);
		CUTSCENE.set_function("GET_CUTSCENE_PLAY_DURATION", LUA_NATIVE_CUTSCENE_GET_CUTSCENE_PLAY_DURATION);
		CUTSCENE.set_function("WAS_CUTSCENE_SKIPPED", LUA_NATIVE_CUTSCENE_WAS_CUTSCENE_SKIPPED);
		CUTSCENE.set_function("HAS_CUTSCENE_FINISHED", LUA_NATIVE_CUTSCENE_HAS_CUTSCENE_FINISHED);
		CUTSCENE.set_function("IS_CUTSCENE_ACTIVE", LUA_NATIVE_CUTSCENE_IS_CUTSCENE_ACTIVE);
		CUTSCENE.set_function("IS_CUTSCENE_PLAYING", LUA_NATIVE_CUTSCENE_IS_CUTSCENE_PLAYING);
		CUTSCENE.set_function("GET_CUTSCENE_SECTION_PLAYING", LUA_NATIVE_CUTSCENE_GET_CUTSCENE_SECTION_PLAYING);
		CUTSCENE.set_function("GET_ENTITY_INDEX_OF_CUTSCENE_ENTITY", LUA_NATIVE_CUTSCENE_GET_ENTITY_INDEX_OF_CUTSCENE_ENTITY);
		CUTSCENE.set_function("GET_CUTSCENE_CONCAT_SECTION_PLAYING", LUA_NATIVE_CUTSCENE_GET_CUTSCENE_CONCAT_SECTION_PLAYING);
		CUTSCENE.set_function("IS_CUTSCENE_AUTHORIZED", LUA_NATIVE_CUTSCENE_IS_CUTSCENE_AUTHORIZED);
		CUTSCENE.set_function("DOES_CUTSCENE_HANDLE_EXIST", LUA_NATIVE_CUTSCENE_DOES_CUTSCENE_HANDLE_EXIST);
		CUTSCENE.set_function("REGISTER_ENTITY_FOR_CUTSCENE", LUA_NATIVE_CUTSCENE_REGISTER_ENTITY_FOR_CUTSCENE);
		CUTSCENE.set_function("GET_ENTITY_INDEX_OF_REGISTERED_ENTITY", LUA_NATIVE_CUTSCENE_GET_ENTITY_INDEX_OF_REGISTERED_ENTITY);
		CUTSCENE.set_function("SET_VEHICLE_MODEL_PLAYER_WILL_EXIT_SCENE", LUA_NATIVE_CUTSCENE_SET_VEHICLE_MODEL_PLAYER_WILL_EXIT_SCENE);
		CUTSCENE.set_function("SET_CUTSCENE_TRIGGER_AREA", LUA_NATIVE_CUTSCENE_SET_CUTSCENE_TRIGGER_AREA);
		CUTSCENE.set_function("CAN_SET_ENTER_STATE_FOR_REGISTERED_ENTITY", LUA_NATIVE_CUTSCENE_CAN_SET_ENTER_STATE_FOR_REGISTERED_ENTITY);
		CUTSCENE.set_function("CAN_SET_EXIT_STATE_FOR_REGISTERED_ENTITY", LUA_NATIVE_CUTSCENE_CAN_SET_EXIT_STATE_FOR_REGISTERED_ENTITY);
		CUTSCENE.set_function("CAN_SET_EXIT_STATE_FOR_CAMERA", LUA_NATIVE_CUTSCENE_CAN_SET_EXIT_STATE_FOR_CAMERA);
		CUTSCENE.set_function("SET_PAD_CAN_SHAKE_DURING_CUTSCENE", LUA_NATIVE_CUTSCENE_SET_PAD_CAN_SHAKE_DURING_CUTSCENE);
		CUTSCENE.set_function("SET_CUTSCENE_FADE_VALUES", LUA_NATIVE_CUTSCENE_SET_CUTSCENE_FADE_VALUES);
		CUTSCENE.set_function("SET_CUTSCENE_MULTIHEAD_FADE", LUA_NATIVE_CUTSCENE_SET_CUTSCENE_MULTIHEAD_FADE);
		CUTSCENE.set_function("SET_CUTSCENE_MULTIHEAD_FADE_MANUAL", LUA_NATIVE_CUTSCENE_SET_CUTSCENE_MULTIHEAD_FADE_MANUAL);
		CUTSCENE.set_function("IS_MULTIHEAD_FADE_UP", LUA_NATIVE_CUTSCENE_IS_MULTIHEAD_FADE_UP);
		CUTSCENE.set_function("NETWORK_SET_MOCAP_CUTSCENE_CAN_BE_SKIPPED", LUA_NATIVE_CUTSCENE_NETWORK_SET_MOCAP_CUTSCENE_CAN_BE_SKIPPED);
		CUTSCENE.set_function("SET_CAR_GENERATORS_CAN_UPDATE_DURING_CUTSCENE", LUA_NATIVE_CUTSCENE_SET_CAR_GENERATORS_CAN_UPDATE_DURING_CUTSCENE);
		CUTSCENE.set_function("CAN_USE_MOBILE_PHONE_DURING_CUTSCENE", LUA_NATIVE_CUTSCENE_CAN_USE_MOBILE_PHONE_DURING_CUTSCENE);
		CUTSCENE.set_function("SET_CUTSCENE_CAN_BE_SKIPPED", LUA_NATIVE_CUTSCENE_SET_CUTSCENE_CAN_BE_SKIPPED);
		CUTSCENE.set_function("SET_CAN_DISPLAY_MINIMAP_DURING_CUTSCENE_THIS_UPDATE", LUA_NATIVE_CUTSCENE_SET_CAN_DISPLAY_MINIMAP_DURING_CUTSCENE_THIS_UPDATE);
		CUTSCENE.set_function("SET_CUTSCENE_PED_COMPONENT_VARIATION", LUA_NATIVE_CUTSCENE_SET_CUTSCENE_PED_COMPONENT_VARIATION);
		CUTSCENE.set_function("SET_CUTSCENE_PED_COMPONENT_VARIATION_FROM_PED", LUA_NATIVE_CUTSCENE_SET_CUTSCENE_PED_COMPONENT_VARIATION_FROM_PED);
		CUTSCENE.set_function("DOES_CUTSCENE_ENTITY_EXIST", LUA_NATIVE_CUTSCENE_DOES_CUTSCENE_ENTITY_EXIST);
		CUTSCENE.set_function("SET_CUTSCENE_PED_PROP_VARIATION", LUA_NATIVE_CUTSCENE_SET_CUTSCENE_PED_PROP_VARIATION);
		CUTSCENE.set_function("HAS_CUTSCENE_CUT_THIS_FRAME", LUA_NATIVE_CUTSCENE_HAS_CUTSCENE_CUT_THIS_FRAME);
	}
}
