#pragma once
#include "fwddec.hpp"
#include "pointers.hpp"

#include <base/atRTTI.hpp>
#include <cstdint>

#pragma pack(push, 1)
namespace rage
{
#pragma pack(push, 8)
	class CSyncDataBase
	{
	public:
		virtual ~CSyncDataBase()                                                     = default;
		virtual bool SerializeDword(uint32_t* dword, int size)                       = 0; // 1
		virtual bool SerializeWord(uint16_t* word, int size)                         = 0; // 2
		virtual bool SerializeByte(uint8_t* byte, int size)                          = 0; // 3
		virtual bool SerializeInt32(int32_t* i, int size)                            = 0; // 4
		virtual bool SerializeInt16(int16_t* i, int size)                            = 0; // 5
		virtual bool SerializeSignedByte(int8_t* byte, int size)                     = 0; // 6
		virtual bool SerializeBool(bool* flag)                                       = 0; // 7
		virtual bool SerializeInt64(int64_t* i, int size)                            = 0; // 8
		virtual bool SerializeInt32Alt(int32_t* i, int size)                         = 0; // 9
		virtual bool SerializeInt16Alt(int16_t* i, int size)                         = 0; // 10
		virtual bool SerializeSignedByteAlt(int8_t* byte, int size)                  = 0; // 11
		virtual bool SerializeQword(uint64_t* qword, int size)                       = 0; // 12
		virtual bool SerializeDwordAlt(uint32_t* dword, int size)                    = 0; // 13
		virtual bool SerializeWordAlt(uint16_t* word, int size)                      = 0; // 14
		virtual bool SerializeByteAlt(uint8_t* byte, int size)                       = 0; // 15
		virtual bool SerializeSignedFloat(float* flt, float divisor, int size)       = 0; // 16
		virtual bool SerializeFloat(float* flt, float divisor, int size)             = 0; // 17
		virtual bool SerializeNetworkId(uint16_t* net_id)                            = 0; // 18
		virtual bool SerializeVector3(rage::fvector3* vec3, float divisor, int size) = 0; // 19
		virtual bool SerializeQuaternion(void* unk)                                  = 0; // i have no clue what that is
		virtual bool SerializeVector3SignedZComponent(rage::fvector3* vec3, float divisor, int size) = 0; // 21
		virtual bool SerializeOrientation(rage::fvector4* vec4, float size) = 0; // yes, the size is a float
		virtual bool SerializeArray(void* array, int size)                  = 0;
		virtual bool SerializeString(char* str, int max_length)             = 0;
		virtual bool IsSizeCalculator()                                     = 0;
		virtual bool IsSizeCalculator2()                                    = 0;

		enum class Type
		{
			Reader = 1,
			Writer,
			SizeCalculator,
			Logger
		};

		Type m_type;
		void* m_sync_log;
		datBitBuffer* m_buffer;
	};
#pragma pack(pop)

	class netPlayer;

	class datBitBuffer
	{
	public:
		datBitBuffer(void* data, uint32_t size)
		{
			m_data            = data;
			m_bitOffset       = 0;
			m_maxBit          = size * 8;
			m_bitsRead        = 0;
			m_curBit          = 0;
			m_highestBitsRead = 0;
			m_flagBits        = 0;
		}
		uint32_t GetPosition()
		{
			return m_bitsRead;
		}
		void Seek(uint32_t bits)
		{
			if (bits >= 0)
			{
				uint32_t length = (m_flagBits & 1) ? m_maxBit : m_curBit;
				if (bits <= length)
					m_bitsRead = bits;
			}
		}
		void SeekForward(uint32_t bits)
		{
			m_bitsRead += static_cast<uint32_t>(bits);
			if (m_bitsRead > m_highestBitsRead)
				m_highestBitsRead = m_bitsRead;
		}
		bool WriteBool(bool boolean)
		{
			return big::g_pointers->m_gta.m_write_bitbuf_bool(this, boolean, 1);
		}
		bool ReadBool(bool* boolean)
		{
			return big::g_pointers->m_gta.m_read_bitbuf_bool(this, boolean, 1);
		}
		bool ReadRockstarId(int64_t* rockstar_id)
		{
			return this->ReadInt64(rockstar_id, sizeof(rockstar_id) * 8);
		}
		uint64_t ReadBits(size_t numBits)
		{
			auto const totalBits = (m_flagBits & 1) ? m_maxBit : m_curBit;
			if ((m_flagBits & 2) || m_bitsRead + numBits > totalBits)
				return 0;
			auto const bufPos           = m_bitsRead + m_bitOffset;
			auto const initialBitOffset = bufPos & 0b111;
			auto const start            = &((uint8_t*)m_data)[bufPos / 8];
			auto const next             = &start[1];
			auto result                 = (start[0] << initialBitOffset) & 0xff;
			for (auto i = 0; i < ((numBits - 1) / 8); i++)
			{
				result <<= 8;
				result |= next[i] << initialBitOffset;
			}
			if (initialBitOffset)
				result |= next[0] >> (8 - initialBitOffset);
			m_bitsRead += static_cast<uint32_t>(numBits);
			if (m_bitsRead > m_highestBitsRead)
				m_highestBitsRead = m_bitsRead;
			return result >> ((8 - numBits) % 8);
		}
		int GetDataLength()
		{
			int leftoverBit = (m_curBit % 8) ? 1 : 0;
			return (m_curBit / 8) + leftoverBit;
		}
		bool ReadString(char* string, int bits)
		{
			return big::g_pointers->m_gta.m_read_bitbuf_string(this, string, bits);
		}
		bool WriteByte(uint8_t integer, int bits)
		{
			return big::g_pointers->m_gta.m_write_bitbuf_dword(this, integer, bits);
		}
		bool ReadByte(uint8_t* integer, int bits)
		{
			uint32_t read;
			if (big::g_pointers->m_gta.m_read_bitbuf_dword(this, &read, bits))
			{
				*integer = read;
				return true;
			}
			return false;
		}
		bool WriteWord(uint16_t integer, int bits)
		{
			return big::g_pointers->m_gta.m_write_bitbuf_dword(this, integer, bits);
		}
		bool ReadWord(uint16_t* integer, int bits)
		{
			uint32_t read;
			if (big::g_pointers->m_gta.m_read_bitbuf_dword(this, &read, bits))
			{
				*integer = read;
				return true;
			}
			return false;
		}
		bool WriteDword(uint32_t integer, int bits)
		{
			return big::g_pointers->m_gta.m_write_bitbuf_dword(this, integer, bits);
		}
		bool ReadDword(uint32_t* integer, int bits)
		{
			return big::g_pointers->m_gta.m_read_bitbuf_dword(this, integer, bits);
		}
		bool WriteInt32(int32_t integer, int bits)
		{
			return big::g_pointers->m_gta.m_write_bitbuf_int32(this, integer, bits);
		}
		bool ReadInt32(int32_t* integer, int bits)
		{
			int32_t v8;
			int32_t v9;
			if (ReadDword((uint32_t*)&v8, 1u) && ReadDword((uint32_t*)&v9, bits - 1))
			{
				*integer = v8 + (v9 ^ -v8);
				return true;
			}
			return false;
		}
		bool WriteQWord(uint64_t integer, int bits)
		{
			return big::g_pointers->m_gta.m_write_bitbuf_qword(this, integer, bits);
		}
		bool ReadQWord(uint64_t* integer, int bits)
		{
			if (bits <= 32)
			{
				uint32_t v10;
				if (ReadDword(&v10, bits))
				{
					*integer = v10;
					return true;
				}
			}
			else
			{
				uint32_t v10, v11;
				if (ReadDword(&v11, 32u) && ReadDword(&v10, bits - 32u))
				{
					*integer = v11 | ((uint64_t)v10 << 32);
					return true;
				}
			}
			return false;
		}
		bool WriteInt64(int64_t integer, int bits)
		{
			return big::g_pointers->m_gta.m_write_bitbuf_int64(this, integer, bits);
		}
		bool WriteRockstarId(int64_t rockstar_id)
		{
			return big::g_pointers->m_gta.m_write_bitbuf_int64(this, rockstar_id, sizeof(rockstar_id) * 8);
		}
		bool ReadInt64(int64_t* integer, int bits)
		{
			uint32_t v8;
			uint64_t v9;
			if (ReadDword(&v8, 1u) && ReadQWord(&v9, bits - 1))
			{
				*integer = v8 + (v9 ^ -(int64_t)v8);
				return true;
			}
			return false;
		}
		bool WriteArray(void* array, int size)
		{
			return big::g_pointers->m_gta.m_write_bitbuf_array(this, array, size, 0);
		}

		void WriteString(const char* string, int max_len)
		{
			auto len      = std::min(max_len, (int)strlen(string) + 1);
			bool extended = len > 127;
			Write<bool>(extended, 1);
			Write<int>(len, extended ? 15 : 7);
			WriteArray((void*)string, 8 * len);
		}

		bool ReadArray(PVOID array, int size)
		{
			return big::g_pointers->m_gta.m_read_bitbuf_array(this, array, size, 0);
		}

		template<typename T>
		inline T Read(int length)
		{
			static_assert(sizeof(T) <= 8, "maximum of 64 bit read");
			static_assert(!std::is_same_v<T, float>, "use ReadFloat to read floating point values from the bitbuffer");

			uint64_t val = 0;
			ReadQWord(&val, length);

			return T(val);
		}

		template<typename T>
		inline T ReadSigned(int length)
		{
			static_assert(sizeof(T) <= 4, "maximum of 32 bit read");
			static_assert(!std::is_same_v<T, float>, "use ReadSignedFloat to read signed floating point values from the bitbuffer");

			int val = 0;
			ReadInt32(&val, length);

			return T(val);
		}

		template<typename T>
		inline void Write(T data, int length)
		{
			static_assert(sizeof(T) <= 8, "maximum of 64 bit write");

			WriteQWord((uint64_t)data, length);
		}

		template<typename T>
		inline void WriteSigned(int length, T data)
		{
			int sign   = data < 0;
			int signEx = (data < 0) ? 0xFFFFFFFF : 0;
			int d      = (data ^ signEx);

			Write<int>(1, sign);
			Write<int>(length - 1, d);
		}

		inline float ReadFloat(int length, float divisor)
		{
			auto integer = Read<int>(length);

			float max = (1 << length) - 1;
			return ((float)integer / max) * divisor;
		}

		inline void WriteFloat(int length, float divisor, float value)
		{
			float max   = (1 << length) - 1;
			int integer = (int)((value / divisor) * max);

			Write<int>(integer, length);
		}

		inline float ReadSignedFloat(int length, float divisor)
		{
			auto integer = ReadSigned<int>(length);

			float max = (1 << (length - 1)) - 1;
			return ((float)integer / max) * divisor;
		}

		inline void WriteSignedFloat(int length, float divisor, float value)
		{
			float max   = (1 << (length - 1)) - 1;
			int integer = (int)((value / divisor) * max);

			WriteSigned<int>(integer, length);
		}

	public:
		void* m_data;               //0x0000
		uint32_t m_bitOffset;       //0x0008
		uint32_t m_maxBit;          //0x000C
		uint32_t m_bitsRead;        //0x0010
		uint32_t m_curBit;          //0x0014
		uint32_t m_highestBitsRead; //0x0018
		uint8_t m_flagBits;         //0x001C
	};

	enum class eNetMessage : uint32_t
	{
		MsgInvalid                     = 0xFFFFF,
		MsgSessionAcceptChat           = 0x62,
		MsgStartMatchCmd               = 0x2D,
		MsgSetInvitableCmd             = 0x1F,
		MsgSessionMemberIds            = 0x23,
		MsgRequestGamerInfo            = 0x54,
		MsgRemoveGamersFromSessionCmd  = 0x53,
		MsgNotMigrating                = 0x35,
		MsgMigrateHostResponse         = 0x12,
		MsgMigrateHostRequest          = 0x66,
		MsgJoinResponse                = 0x2A,
		MsgJoinRequest                 = 0x41,
		MsgHostLeftWhilstJoiningCmd    = 0x58,
		MsgConfigResponse              = 0x5F,
		MsgConfigRequest               = 0x48,
		MsgChangeSessionAttributesCmd  = 0x5A,
		MsgAddGamerToSessionCmd        = 0x64, // this is where send net info to lobby is called, among other things
		MsgReassignResponse            = 0x10,
		MsgReassignNegotiate           = 0x01,
		MsgReassignConfirm             = 0x26,
		MsgPlayerData                  = 0x18,
		MsgPackedReliables             = 0x30,
		MsgPackedCloneSyncACKs         = 0x3B,
		MsgNonPhysicalData             = 0x16,
		MsgNetArrayMgrUpdateAck        = 0x5D,
		MsgNetArrayMgrUpdate           = 0x60,
		MsgNetArrayMgrSplitUpdateAck   = 0x25,
		MsgScriptVerifyHostAck         = 0x0B,
		MsgScriptVerifyHost            = 0x3E,
		MsgScriptNewHost               = 0x0E,
		MsgScriptMigrateHostFailAck    = 0x1A,
		MsgScriptMigrateHost           = 0x33,
		MsgScriptLeaveAck              = 0x40,
		MsgScriptLeave                 = 0x17,
		MsgScriptJoinHostAck           = 0x4D,
		MsgScriptJoinAck               = 0x43,
		MsgScriptJoin                  = 0x5C,
		MsgScriptHostRequest           = 0x67,
		MsgScriptHandshakeAck          = 0x5B,
		MsgScriptHandshake             = 0x57,
		MsgScriptBotLeave              = 0x2B, // unused?
		MsgScriptBotJoinAck            = 0x63, // unused?
		MsgScriptBotJoin               = 0x1C, // unused?
		MsgScriptBotHandshakeAck       = 0x31, // unused?
		MsgScriptBotHandshake          = 0x4B, // unused?
		MsgPartyLeaveGame              = 0x3D,
		MsgPartyEnterGame              = 0x1E,
		MsgCloneSync                   = 0x4E, // aka clone_create, clone_sync etc.
		MsgActivateNetworkBot          = 0x65, // unused?
		MsgRequestObjectIds            = 0x29,
		MsgInformObjectIds             = 0x09,
		MsgTextMessage                 = 0x24, // this one is for chat
		MsgPlayerIsTyping              = 0x61,
		MsgPackedEvents                = 0x4F, // aka received_event
		MsgPackedEventReliablesMsgs    = 0x20,
		MsgRequestKickFromHost         = 0x0D,
		MsgTransitionToGameStart       = 0x50,
		MsgTransitionToGameNotify      = 0x02,
		MsgTransitionToActivityStart   = 0x06,
		MsgTransitionToActivityFinish  = 0x36,
		MsgTransitionParameters        = 0x3C,
		MsgTransitionParameterString   = 0x37,
		MsgTransitionLaunchNotify      = 0x1B,
		MsgTransitionLaunch            = 0x19,
		MsgTransitionGamerInstruction  = 0x14,
		MsgTextMessage2                = 0x0A, // this one is for phone message
		MsgSessionEstablishedRequest   = 0x52,
		MsgSessionEstablished          = 0x07,
		MsgRequestTransitionParameters = 0x42,
		MsgRadioStationSyncRequest     = 0x47,
		MsgRadioStationSync            = 0x46,
		MsgPlayerCardSync              = 0x3A,
		MsgPlayerCardRequest           = 0x6A,
		MsgLostConnectionToHost        = 0x81,
		MsgKickPlayer                  = 0x34, // host kick
		MsgDebugStall                  = 0x7E, // unused?
		MsgCheckQueuedJoinRequestReply = 0x59,
		MsgCheckQueuedJoinRequest      = 0x51,
		MsgBlacklist                   = 0x0C,
		MsgRoamingRequestBubbleRequiredResponse = 0x83,
		MsgRoamingRequestBubbleRequiredCheck    = 0x82,
		MsgRoamingRequestBubble                 = 0x2E,
		MsgRoamingJoinBubble                    = 0x4C,
		MsgRoamingJoinBubbleAck                 = 0x3F,
		MsgRoamingInitialBubble                 = 0x32,
		MsgVoiceStatus                          = 0x03,
		MsgTextChatStatus                       = 0x00,
		MsgJoinResponse2                        = 0x08,
		MsgJoinRequest2                         = 0x68,
		MsgNetTimeSync      = 0x38, // ctor 40 53 48 83 EC 20 BA ? ? ? ? 4C 8D 0D ? ? ? ? 48 8B D9 44 8D 42 37
		MsgNetComplaint     = 0x55, // ctor 40 53 48 83 EC 20 BA ? ? ? ? 4C 8D 0D ? ? ? ? 48 8B D9 44 8D 42 54
		MsgNetLagPing       = 0x27, // unused? ctor 40 53 48 83 EC 20 BA ? ? ? ? 4C 8D 0D ? ? ? ? 48 8B D9 44 8D 42 26
		MsgSearchResponse   = 0x6B, // unused? ctor 40 53 48 83 EC 20 BA ? ? ? ? 4C 8D 0D ? ? ? ? 48 8B D9 44 8D 42 6A
		MsgSearchRequest    = 0x05, // unused?
		MsgQosProbeResponse = 0x2C, // ctor 40 53 48 83 EC 20 BA ? ? ? ? 4C 8D 0D ? ? ? ? 48 8B D9 44 8D 42 2B
		MsgQosProbeRequest  = 0x1D, // ctor 40 53 48 83 EC 20 BA ? ? ? ? 4C 8D 0D ? ? ? ? 48 8B D9 44 8D 42 1C
		MsgCxnRelayAddressChanged  = 0x49, // ctor 40 53 48 83 EC 20 BA ? ? ? ? 4C 8D 0D ? ? ? ? 48 8B D9 44 8D 42 48
		MsgCxnRequestRemoteTimeout = 0x2F, // ctor 40 53 48 83 EC 20 BA ? ? ? ? 4C 8D 0D ? ? ? ? 48 8B D9 44 8D 42 2E
		MsgSessionDetailRequest    = 0x22, // ctor 40 53 48 83 EC 20 BA ? ? ? ? 4C 8D 0D ? ? ? ? 48 8B D9 44 8D 42 21
		MsgSessionDetailResponse   = 0x13, // ctor 40 53 48 83 EC 20 BA ? ? ? ? 4C 8D 0D ? ? ? ? 48 8B D9 44 8D 42 12
		MsgKeyExchangeOffer = 0x0F, // ctor 40 53 48 83 EC 20 BA ? ? ? ? 4C 8D 0D ? ? ? ? 48 8B D9 44 8D 42 0E (last result)
		MsgKeyExchangeAnswer = 0x44, // ctor 40 53 48 83 EC 20 BA ? ? ? ? 4C 8D 0D ? ? ? ? 48 8B D9 44 8D 42 43
		Msg_0x87             = 0x87,
		Msg_0x88             = 0x88,
		Msg_0x80             = 0x80,
		Msg_0x28             = 0x28,
		Msg_0x11             = 0x11,
		Msg_0x45             = 0x45,
		Msg_0x89             = 0x89,
		Msg_0x86             = 0x86,
		MsgBattlEyeCmd       = 0x8F,
	};

	enum class eEventNetworkType : int64_t
	{
		CEventNetworkPlayerJoinScript                     = 153,
		CEventNetworkPlayerLeftScript                     = 154,
		CEventNetworkStorePlayerLeft                      = 155,
		CEventNetworkStartSession                         = 156,
		CEventNetworkEndSession                           = 157,
		CEventNetworkStartMatch                           = 158,
		CEventNetworkRemovedFromSessionDueToStall         = 160,
		CEventNetworkRemovedFromSessionDueToComplaints    = 161,
		CEventNetworkConnectionTimeout                    = 162,
		CEventNetworkPedDroppedWeapon                     = 163,
		CEventNetworkPlayerSpawn                          = 164,
		CEventNetworkPlayerCollectedPickup                = 165,
		CEventNetworkPlayerCollectedAmbientPickup         = 166,
		CEventNetworkPlayerCollectedPortablePickup        = 167,
		CEventNetworkPlayerDroppedPortablePickup          = 168,
		CEventNetworkInviteAccepted                       = 170,
		CEventNetworkInviteConfirmed                      = 171,
		CEventNetworkInviteRejected                       = 172,
		CEventNetworkSummo                                = 173,
		CEventNetworkScriptEvent                          = 174,
		CEventNetworkSignInStateChanged                   = 176,
		CEventNetworkSignInChangeActioned                 = 177,
		CEventNetworkRosChanged                           = 178,
		CEventNetworkBail                                 = 179,
		CEventNetworkHostMigration                        = 180,
		CEventNetworkFindSession                          = 181,
		EventNetworkHostSession                           = 182,
		CEventNetworkJoinSession                          = 183,
		CEventNetworkJoinSessionResponse                  = 184,
		CEventNetworkCheatTriggered                       = 185,
		CEventNetworkEntityDamage                         = 186,
		CEventNetworkPlayerArrest                         = 187,
		CEventNetworkTimedExplosion                       = 188,
		CEventNetworkPrimaryClanChanged                   = 189,
		CEventNetworkClanJoined                           = 190,
		CEventNetworkClanLeft                             = 191,
		CEventNetworkClanInviteReceived                   = 192,
		CEventNetworkVoiceSessionStarted                  = 193,
		CEventNetworkVoiceSessionEnded                    = 194,
		CEventNetworkVoiceConnectionRequested             = 195,
		CEventNetworkVoiceConnectionResponse              = 196,
		CEventNetworkVoiceConnectionTerminated            = 197,
		CEventNetworkTextMessageReceived                  = 198,
		CEventNetworkCloudFileResponse                    = 199,
		CEventNetworkPickupRespawned                      = 200,
		CEventNetworkPresence_StatUpdate                  = 201,
		CEventNetworkPedLeftBehind                        = 202,
		CEventNetwork_InboxMsgReceived                    = 203,
		CEventNetworkAttemptHostMigration                 = 204,
		CEventNetworkIncrementStat                        = 205,
		CEventNetworkSessionEvent                         = 206,
		CEventNetworkTransitionStarted                    = 207,
		CEventNetworkTransitionEvent                      = 208,
		CEventNetworkTransitionMemberJoined               = 209,
		CEventNetworkTransitionMemberLeft                 = 210,
		CEventNetworkTransitionParameterChanged           = 211,
		CEventNetworkClanKicked                           = 212,
		CEventNetworkTransitionStringChanged              = 213,
		EventNetworkTransitionGamerInstruction            = 214,
		CEventNetworkPresenceInvite                       = 215,
		EventNetworkPresenceInviteRemoved                 = 216,
		EventNetworkPresenceInviteReply                   = 217,
		CEventNetworkCashTransactionLog                   = 218,
		CEventNetworkClanRankChanged                      = 219,
		CEventNetworkVehicleUndrivable                    = 220,
		CEventNetworkPresenceTriggerEvent                 = 221,
		CEventNetworkEmailReceivedEvent                   = 222,
		CEventNetworkFollowInviteReceived                 = 223,
		CEventNetworkSpectateLocal                        = 225,
		CEventNetworkCloudEvent                           = 226,
		CEventNetworkShopTransaction                      = 227,
		CEventNetworkOnlinePermissionsUpdated             = 230,
		CEventNetworkSystemServiceEvent                   = 231,
		CEventNetworkRequestDelay                         = 232,
		CEventNetworkSocialClubAccountLinked              = 233,
		CEventNetworkScAdminPlayerUpdated                 = 234,
		CEventNetworkScAdminReceivedCash                  = 235,
		CEventNetworkClanInviteRequestReceived            = 236,
		CEventNetworkMarketingEmailReceivedEvent          = 237,
		CEventNetworkStuntPerformed                       = 238,
		CEventNetworkFiredDummyProjectile                 = 239,
		CEventNetworkPlayerEnteredVehicle                 = 240,
		CEventNetworkPlayerActivatedSpecialAbility        = 241,
		CEventNetworkPlayerDeactivatedSpecialAbility      = 242,
		CEventNetworkPlayerSpecialAbilityFailedActivation = 243,
		CEventNetworkFiredVehicleProjectile               = 244,
	};

	class CEventNetwork
	{
	public:
		DEFINE_RAGE_RTTI(rage::CEventNetwork);
		virtual void unk_0008()                                         = 0; //0x0008 (1)
		virtual void unk_0010()                                         = 0; //0x0010 (2)
		virtual eEventNetworkType get_type()                            = 0; //0x0018 (3)
		virtual void unk_0020()                                         = 0; //0x0020 (4)
		virtual void unk_0028()                                         = 0; //0x0028 (5)
		virtual bool get_extra_information(void* info_array, int check) = 0; //0x0030 (6)
		virtual void unk_0038()                                         = 0; //0x0038 (7)
	};                                                                       //Size: 0x0008

	class sEntityDamagedData
	{
	public:
		alignas(8) Entity m_victim_index;               //0x0000
		alignas(8) Entity m_damager_index;              //0x0008
		alignas(8) float m_damage;                      //0x0010
		alignas(8) float m_endurance_damage;            //0x0018
		alignas(8) bool m_victim_incapacitated;         //0x0020
		alignas(8) bool m_victim_destroyed;             //0x0028
		alignas(8) int m_weapon_used;                   //0x0030
		alignas(8) float m_victim_speed;                //0x0038
		alignas(8) float m_damager_speed;               //0x0040
		alignas(8) bool m_is_responsible_for_collision; //0x0048
		alignas(8) bool m_is_headshot;                  //0x0050
		alignas(8) bool m_is_with_melee_weapon;         //0x0058
		alignas(8) int m_hit_material;                  //0x0060
	};                                                  //Size: 0x0068
	static_assert(sizeof(sEntityDamagedData) == 0x68);

	class netGameEvent
	{
	public:
		virtual ~netGameEvent() = default;

		virtual const char* get_name()
		{
			return 0;
		};
		virtual bool is_in_scope(netPlayer* player)
		{
			return 0;
		};
		virtual bool time_to_resend(uint32_t time)
		{
			return 0;
		};
		virtual bool can_change_scope()
		{
			return 0;
		};

		virtual void prepare_data(datBitBuffer* buffer, netPlayer* source_player, netPlayer* target_player){};
		virtual void handle_data(datBitBuffer* buffer, netPlayer* source_player, netPlayer* target_player){};

		virtual bool decide(netPlayer* source_player, netPlayer* target_player)
		{
			return 0;
		};

		virtual void prepare_reply(datBitBuffer* buffer, netPlayer* reply_player){};
		virtual void handle_reply(datBitBuffer* buffer, netPlayer* souce_player){};

		virtual void prepare_extra_data(datBitBuffer* buffer, bool is_reply, netPlayer* player, netPlayer* player2){};
		virtual void handle_extra_data(datBitBuffer* buffer, bool is_reply, netPlayer* player, netPlayer* player2){};

	private:
		virtual void unk_0x60(){};
		virtual void unk_0x68(){};
		virtual void unk_0x70(){};
		virtual void unk_0x78(){};

	public:
		virtual bool operator==(netGameEvent const& other)
		{
			return 0;
		};
		virtual bool operator!=(netGameEvent const& other)
		{
			return 0;
		};

		virtual bool must_persist()
		{
			return 0;
		};
		virtual bool must_persist_when_out_of_scope()
		{
			return 0;
		};
		virtual bool has_timed_out()
		{
			return 0;
		};

	public:
		uint16_t m_id;         // 0x08
		bool m_requires_reply; // 0x0A
	private:
		char m_padding1[0x05]; // 0x0B
	public:
		netPlayer* m_source_player; // 0x10
		netPlayer* m_target_player; // 0x18
		uint32_t m_resend_time;     // 0x20
	private:
		uint16_t m_0x24; // 0x24
		uint8_t m_0x26;  // 0x26
		uint8_t m_0x27;  // 0x27
		uint32_t m_0x28; // 0x28
		char m_padding2[0x04];
	};
	static_assert(sizeof(rage::netGameEvent) == 0x30);
}

class CScriptedGameEvent : public rage::netGameEvent
{
public:
	char m_padding[0x40];    // 0x30
	std::int64_t m_args[54]; // 0x70
	uint32_t m_bitset;       // 0x220
	uint32_t m_args_size;    // 0x224
};

class CNetworkIncrementStatEvent : public rage::netGameEvent
{
public:
	Hash m_stat;       // 0x30
	uint32_t m_amount; // 0x34
};

class CDoorBreakEvent : public rage::netGameEvent
{
public:
	std::uint16_t m_vehicle_id; // 0x30
	std::uint8_t m_door_id;     // 0x32
};
#pragma pack(pop)
