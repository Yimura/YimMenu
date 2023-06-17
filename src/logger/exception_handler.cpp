#include "exception_handler.hpp"

#include "stack_trace.hpp"

#include <Zydis/Zydis.h>

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
		SetErrorMode(0);
		SetUnhandledExceptionFilter(&vectored_exception_handler);
	}

	exception_handler::~exception_handler()
	{
		// passing nullptr will make it go back to normal exception handling
		SetUnhandledExceptionFilter(nullptr);
	}

	inline static stack_trace trace;
	LONG vectored_exception_handler(EXCEPTION_POINTERS* totally_not_exception_info)
	{
		const auto exception_code = totally_not_exception_info->ExceptionRecord->ExceptionCode;
		if (exception_code == EXCEPTION_BREAKPOINT || exception_code == DBG_PRINTEXCEPTION_C || exception_code == DBG_PRINTEXCEPTION_WIDE_C)
			return EXCEPTION_CONTINUE_SEARCH;

		static std::set<std::size_t> logged_exceptions;

		trace.new_stack_trace(totally_not_exception_info);
		const auto trace_hash = hash_stack_trace(trace.frame_pointers());
		if (const auto it = logged_exceptions.find(trace_hash); it == logged_exceptions.end())
		{
			LOG(FATAL) << trace;

			logged_exceptions.insert(trace_hash);
		}

		ZyanU64 opcode_address = totally_not_exception_info->ContextRecord->Rip;
		ZydisDisassembledInstruction instruction;
		ZydisDisassembleIntel(ZYDIS_MACHINE_MODE_LONG_64, opcode_address, reinterpret_cast<void*>(opcode_address), 32, &instruction);

		totally_not_exception_info->ContextRecord->Rip += instruction.info.length;

		return EXCEPTION_CONTINUE_EXECUTION;
	}
}