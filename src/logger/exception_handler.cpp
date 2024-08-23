#include "exception_handler.hpp"

#include "stack_trace.hpp"

#include <hde64.h>

namespace big
{
	inline auto hash_stack_trace(std::vector<uint64_t> stack_trace)
	{
		auto data = reinterpret_cast<const char*>(stack_trace.data());
		std::size_t size = stack_trace.size() * sizeof(uint64_t);

		return std::hash<std::string_view>()({ data, size });
	}

	exception_handler::exception_handler()
	{
		m_old_error_mode = SetErrorMode(0);
		m_exception_handler = SetUnhandledExceptionFilter(&vectored_exception_handler);
	}

	exception_handler::~exception_handler()
	{
		SetErrorMode(m_old_error_mode);
		SetUnhandledExceptionFilter(reinterpret_cast<decltype(&vectored_exception_handler)>(m_exception_handler));
	}

	inline static stack_trace trace;
	LONG vectored_exception_handler(EXCEPTION_POINTERS* exception_info)
	{
		const auto exception_code = exception_info->ExceptionRecord->ExceptionCode;
		if (exception_code == EXCEPTION_BREAKPOINT || exception_code == DBG_PRINTEXCEPTION_C || exception_code == DBG_PRINTEXCEPTION_WIDE_C)
			return EXCEPTION_CONTINUE_SEARCH;

		static std::set<std::size_t> logged_exceptions;

		trace.new_stack_trace(exception_info);
		const auto trace_hash = hash_stack_trace(trace.frame_pointers());
		if (const auto it = logged_exceptions.find(trace_hash); it == logged_exceptions.end())
		{
			LOG(FATAL) << trace;
			Logger::FlushQueue();

			logged_exceptions.insert(trace_hash);
		}

	
		if (IsBadReadPtr(reinterpret_cast<void*>(exception_info->ContextRecord->Rip), 8))
		{
			auto return_address_ptr = (uint64_t*)exception_info->ContextRecord->Rsp;
			if (IsBadReadPtr(reinterpret_cast<void*>(return_address_ptr), 8))
			{
				LOG(FATAL) << "Cannot resume execution, crashing";
				return EXCEPTION_CONTINUE_SEARCH;
			}
			else
			{
				exception_info->ContextRecord->Rip = *return_address_ptr;
				exception_info->ContextRecord->Rsp += 8;
			}
		}
		else
		{
			hde64s opcode{};
			hde64_disasm(reinterpret_cast<void*>(exception_info->ContextRecord->Rip), &opcode);
			if (opcode.flags & F_ERROR)
			{
				LOG(FATAL) << "Cannot resume execution, crashing";
				return EXCEPTION_CONTINUE_SEARCH;
			}

			if (opcode.opcode == 0xFF && opcode.modrm_reg == 4) // JMP (FF /4)
			{
				auto return_address_ptr = (uint64_t*)exception_info->ContextRecord->Rsp;
				if (IsBadReadPtr(reinterpret_cast<void*>(return_address_ptr), 8))
				{
					LOG(FATAL) << "Cannot resume execution, crashing";
					return EXCEPTION_CONTINUE_SEARCH;
				}
				else
				{
					exception_info->ContextRecord->Rip = *return_address_ptr;
					exception_info->ContextRecord->Rsp += 8;
				}
			}
			else
			{
				exception_info->ContextRecord->Rip += opcode.len;

				if (opcode.opcode == 0x8B && opcode.modrm_mod != 3) // MOV
				{
					uint8_t reg_id = opcode.rex_r << 3 | opcode.modrm_reg;
					switch (reg_id)
					{
					case 0: exception_info->ContextRecord->Rax = 0; break;
					case 1: exception_info->ContextRecord->Rcx = 0; break;
					case 2: exception_info->ContextRecord->Rdx = 0; break;
					case 3: exception_info->ContextRecord->Rbx = 0; break;
					case 4: exception_info->ContextRecord->Rsp = 0; break;
					case 5: exception_info->ContextRecord->Rbp = 0; break;
					case 6: exception_info->ContextRecord->Rsi = 0; break;
					case 7: exception_info->ContextRecord->Rdi = 0; break;
					case 8: exception_info->ContextRecord->R8 = 0; break;
					case 9: exception_info->ContextRecord->R9 = 0; break;
					case 10: exception_info->ContextRecord->R10 = 0; break;
					case 11: exception_info->ContextRecord->R11 = 0; break;
					case 12: exception_info->ContextRecord->R12 = 0; break;
					case 13: exception_info->ContextRecord->R13 = 0; break;
					case 14: exception_info->ContextRecord->R14 = 0; break;
					case 15: exception_info->ContextRecord->R15 = 0; break;
					}
				}
			}
		}

		return EXCEPTION_CONTINUE_EXECUTION;
	}
}