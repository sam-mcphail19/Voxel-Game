
#include <windows.h>
#include <dbghelp.h>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include "noiseTool.hpp"
#include "voxelGame.hpp"

#pragma comment(lib, "dbghelp.lib")

std::string getStackTrace() {
	HANDLE process = GetCurrentProcess();
	SymInitialize(process, nullptr, TRUE);

	void* stack[100];
	unsigned short frames = CaptureStackBackTrace(0, 100, stack, nullptr);

	SYMBOL_INFO* symbol = (SYMBOL_INFO*)malloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char));
	symbol->MaxNameLen = 255;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

	IMAGEHLP_LINE64 line;
	DWORD displacement;
	line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

	std::ostringstream oss;

	for (unsigned short i = 0; i < frames; i++) {
		SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
		if (SymGetLineFromAddr64(process, (DWORD64)stack[i], &displacement, &line)) {
			oss << "\t" << frames - i - 1 << ": " << symbol->Name
				<< " - 0x" << std::hex << symbol->Address << std::dec
				<< " (" << line.FileName << ":" << line.LineNumber << ")\n";
		}
		else {
			oss << "\t" << frames - i - 1 << ": " << symbol->Name
				<< " - 0x" << std::hex << symbol->Address << std::dec << "\n";
		}
	}

	free(symbol);
	SymCleanup(process);

	return oss.str();
}

LONG WINAPI exceptionHandler(EXCEPTION_POINTERS* ExceptionInfo) {
	std::ostringstream message;
	message << "Exception caught!\n" << getStackTrace();
	voxel_game::log::error(message.str());
	return EXCEPTION_EXECUTE_HANDLER;
}

int main()
{
	SetUnhandledExceptionFilter(exceptionHandler);

#ifdef NOISE_TOOL
	voxel_game::NoiseTool noiseTool;

	std::random_device rd;
	std::mt19937 engine(rd());

	std::uniform_int_distribution<long> dist(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());

	noiseTool.generate(dist(engine));
	while (!noiseTool.shouldClose())
	{
		noiseTool.draw();
		if (voxel_game::input::isKeyPressed(GLFW_KEY_R))
		{
			long seed = dist(engine);
			voxel_game::log::info("Regenerating with seed: " + std::to_string(seed));
			noiseTool.generate(seed);
		}

		voxel_game::input::update();
	}
#else
	voxel_game::VoxelGame voxelGame;
	while (!voxelGame.shouldClose())
	{
		voxelGame.update();
	}
#endif

	return 0;
}
