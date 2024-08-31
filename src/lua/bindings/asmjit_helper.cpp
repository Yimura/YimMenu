#pragma once
#include "asmjit_helper.hpp"

namespace lua::memory
{
	bool is_general_register(const asmjit::TypeId type_id)
	{
		switch (type_id)
		{
		case asmjit::TypeId::kInt8:
		case asmjit::TypeId::kUInt8:
		case asmjit::TypeId::kInt16:
		case asmjit::TypeId::kUInt16:
		case asmjit::TypeId::kInt32:
		case asmjit::TypeId::kUInt32:
		case asmjit::TypeId::kInt64:
		case asmjit::TypeId::kUInt64:
		case asmjit::TypeId::kIntPtr:
		case asmjit::TypeId::kUIntPtr: return true;
		default: return false;
		}
	}

	bool is_XMM_register(const asmjit::TypeId type_id)
	{
		switch (type_id)
		{
		case asmjit::TypeId::kFloat32:
		case asmjit::TypeId::kFloat64: return true;
		default: return false;
		}
	}

	asmjit::CallConvId get_call_convention(const std::string& conv)
	{
		if (conv == "cdecl")
		{
			return asmjit::CallConvId::kCDecl;
		}
		else if (conv == "stdcall")
		{
			return asmjit::CallConvId::kStdCall;
		}
		else if (conv == "fastcall")
		{
			return asmjit::CallConvId::kFastCall;
		}

		return asmjit::CallConvId::kHost;
	}

	asmjit::TypeId get_type_id(const std::string& type)
	{
		if (type.find('*') != std::string::npos)
		{
			return asmjit::TypeId::kUIntPtr;
		}

#define TYPEID_MATCH_STR_IF(var, T)                                      \
	if (var == #T)                                                       \
	{                                                                    \
		return asmjit::TypeId(asmjit::TypeUtils::TypeIdOfT<T>::kTypeId); \
	}
#define TYPEID_MATCH_STR_ELSEIF(var, T)                                  \
	else if (var == #T)                                                  \
	{                                                                    \
		return asmjit::TypeId(asmjit::TypeUtils::TypeIdOfT<T>::kTypeId); \
	}

		TYPEID_MATCH_STR_IF(type, signed char)
		TYPEID_MATCH_STR_ELSEIF(type, unsigned char)
		TYPEID_MATCH_STR_ELSEIF(type, short)
		TYPEID_MATCH_STR_ELSEIF(type, unsigned short)
		TYPEID_MATCH_STR_ELSEIF(type, int)
		TYPEID_MATCH_STR_ELSEIF(type, unsigned int)
		TYPEID_MATCH_STR_ELSEIF(type, long)
		TYPEID_MATCH_STR_ELSEIF(type, unsigned long)
#ifdef POLYHOOK2_OS_WINDOWS
		TYPEID_MATCH_STR_ELSEIF(type, __int64)
		TYPEID_MATCH_STR_ELSEIF(type, unsigned __int64)
#endif
		TYPEID_MATCH_STR_ELSEIF(type, long long)
		TYPEID_MATCH_STR_ELSEIF(type, unsigned long long)
		TYPEID_MATCH_STR_ELSEIF(type, char)
		TYPEID_MATCH_STR_ELSEIF(type, char16_t)
		TYPEID_MATCH_STR_ELSEIF(type, char32_t)
		TYPEID_MATCH_STR_ELSEIF(type, wchar_t)
		TYPEID_MATCH_STR_ELSEIF(type, uint8_t)
		TYPEID_MATCH_STR_ELSEIF(type, int8_t)
		TYPEID_MATCH_STR_ELSEIF(type, uint16_t)
		TYPEID_MATCH_STR_ELSEIF(type, int16_t)
		TYPEID_MATCH_STR_ELSEIF(type, int32_t)
		TYPEID_MATCH_STR_ELSEIF(type, uint32_t)
		TYPEID_MATCH_STR_ELSEIF(type, uint64_t)
		TYPEID_MATCH_STR_ELSEIF(type, int64_t)
		TYPEID_MATCH_STR_ELSEIF(type, float)
		TYPEID_MATCH_STR_ELSEIF(type, double)
		TYPEID_MATCH_STR_ELSEIF(type, bool)
		TYPEID_MATCH_STR_ELSEIF(type, void)
		else if (type == "intptr_t")
		{
			return asmjit::TypeId::kIntPtr;
		}
		else if (type == "uintptr_t")
		{
			return asmjit::TypeId::kUIntPtr;
		}

		return asmjit::TypeId::kVoid;
	}
}