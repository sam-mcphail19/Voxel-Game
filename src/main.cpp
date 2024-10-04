
#include <windows.h>
#include <dbghelp.h>
#include <iostream>
#include <sstream>
#include <string>
#include "voxelGame.hpp"

#pragma comment(lib, "dbghelp.lib")

void printStackTrace() {
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);

    void* stack[100];
    unsigned short frames = CaptureStackBackTrace(0, 100, stack, NULL);

    SYMBOL_INFO* symbol = (SYMBOL_INFO*)malloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char));
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    IMAGEHLP_LINE64 line;
    DWORD displacement;
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    for (unsigned short i = 0; i < frames; i++) {
        SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
        if (SymGetLineFromAddr64(process, (DWORD64)stack[i], &displacement, &line)) {
            std::cout << "\t" << frames - i - 1 << ": " << symbol->Name << " - 0x" << std::hex << symbol->Address << std::dec << " (" << line.FileName << ":" << line.LineNumber << ")" << std::endl;
        } else {
            std::cout << "\t" << frames - i - 1 << ": " << symbol->Name << " - 0x" << std::hex << symbol->Address << std::dec << std::endl;
        }
    }

    free(symbol);
    SymCleanup(process);
}

LONG WINAPI exceptionHandler(EXCEPTION_POINTERS* ExceptionInfo) {
    std::cerr << "Exception caught!" << std::endl;
    printStackTrace();
    return EXCEPTION_EXECUTE_HANDLER;
}

int main()
{
	SetUnhandledExceptionFilter(exceptionHandler);

	voxel_game::VoxelGame voxelGame;
	while (!voxelGame.shouldClose())
	{
		voxelGame.update();
	}

	return 0;
}
