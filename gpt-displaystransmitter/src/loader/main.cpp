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
	std::string args = stream.str().append("\0");
	
	return !DetourCreateProcessWithDll("Falcon BMS.exe", &args[0], NULL, NULL, TRUE, 0, NULL, NULL, &s, &p, "gpt_displaystransmitter_hook.dll", 0);
}
