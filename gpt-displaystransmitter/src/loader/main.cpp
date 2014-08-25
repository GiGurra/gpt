#pragma comment(lib,"detours.lib")
#include "windows.h"
#include "detours.h"
#include <sstream>

int main(int argc, char *argv[]) {
	STARTUPINFO s = { sizeof(STARTUPINFO) };
	PROCESS_INFORMATION p = { 0 };

	std::stringstream stream;
	for (int i = 0; i < argc; i++) {
		stream << " ";
		stream << argv[i];
	}
	std::string args = stream.str();
	
	return !DetourCreateProcessWithDll("Falcon BMS.exe", (char*)(args.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &s, &p, "gpt_displaystransmitter_hook.dll", 0);
}
