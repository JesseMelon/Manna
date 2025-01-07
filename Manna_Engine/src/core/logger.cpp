#include "logger.h"
#include "asserts.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

void report_assertion_fail(const char* file, int line, const char* expression, const char* message) {
	log(LOG_FATAL, "Assertion Failed: %s File: %s Line: %d Message: %s\n", expression, file, line, message);
}
b8 init_logger() {
	//TODO create log file
	return TRUE;
}
void shutdown_logger() {
	//TODO cleanup logging/write queued entries
}
void log(log_level level, const char* message, ...) {
	//TODO add to a thread
	const char* levels[6] = { "FATAL", "ERROR", "WARNING","DEBUG", "INFO", "TRACE"};
	b8 is_error = level > LOG_WARN;

	char buffer[1024]; //TODO mind the limit

	va_list args;
	va_start(args, message);
	int written = snprintf(buffer, sizeof(buffer), "[%s]: ", levels[level]);	//prepend level
	vsnprintf(buffer + written, sizeof(buffer) - written, message, args);		//pass message
	va_end(args);

	//TODO platform specific output
	printf("%s\n", buffer);
}