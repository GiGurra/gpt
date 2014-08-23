#include "common.h"
#include <memory>
#include <mutex>

void logText(const char * str) {

	if (!str || strlen(str) == 0)
		return;

	static std::mutex mutex;

	std::lock_guard<std::mutex> lock(mutex);
	std::shared_ptr<FILE> file(fopen("hook.log", "a"), fclose);

	if (file)
		fprintf(file.get(), "%s\n", str);
}

void logNumber(const double d) {
	char buf[32] = { };
	sprintf(buf, "%f", d);
	logText(buf);
}
