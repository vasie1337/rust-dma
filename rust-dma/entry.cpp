#include "include.hpp"

#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

static void DoStackWalk()
{
	CONTEXT context;
	RtlCaptureContext(&context);
	STACKFRAME64 stack_frame;
	ZeroMemory(&stack_frame, sizeof(STACKFRAME64));
	stack_frame.AddrPC.Offset = context.Rip;
	stack_frame.AddrPC.Mode = AddrModeFlat;
	stack_frame.AddrFrame.Offset = context.Rbp;
	stack_frame.AddrFrame.Mode = AddrModeFlat;
	stack_frame.AddrStack.Offset = context.Rsp;
	stack_frame.AddrStack.Mode = AddrModeFlat;
	HANDLE process = GetCurrentProcess();
	HANDLE thread = GetCurrentThread();
	while (StackWalk64(IMAGE_FILE_MACHINE_AMD64, process, thread, &stack_frame, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
	{
		if (stack_frame.AddrPC.Offset == 0)
		{
			break;
		}
		DWORD64 displacement = 0;
		IMAGEHLP_SYMBOL64* symbol = (IMAGEHLP_SYMBOL64*)malloc(sizeof(IMAGEHLP_SYMBOL64) + MAX_SYM_NAME);
		ZeroMemory(symbol, sizeof(IMAGEHLP_SYMBOL64) + MAX_SYM_NAME);
		symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
		symbol->MaxNameLength = MAX_SYM_NAME;
		if (SymGetSymFromAddr64(process, stack_frame.AddrPC.Offset, &displacement, symbol))
		{
			std::cout << symbol->Name << std::endl;
		}
		free(symbol);
	}
}

int main()
{
	AddVectoredExceptionHandler(1, [](PEXCEPTION_POINTERS ExceptionInfo) -> LONG
	{
		DoStackWalk();
		return EXCEPTION_CONTINUE_SEARCH;
	});

	AddVectoredContinueHandler(1, [](struct _EXCEPTION_POINTERS* ExceptionInfo) -> LONG
	{
		DoStackWalk();
		return EXCEPTION_CONTINUE_SEARCH;
	});

	try
	{
		Cheat cheat;
		return 0;
	}
	catch (const std::exception& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
		std::getchar();
		return 1;
	}
}