#include "logger.h"
#include "asserts.h"    // implements report assertion fail
#include "platform/platform.h"

//TODO temporary
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

void report_assertion_fail(const char* file, int line, const char* expression, const char* message) {
	m_log(LOG_LEVEL_ERROR, "Assertion Failed: %s File: %s Line: %d Message: %s\n", expression, file, line, message);
}
b8 init_logger() {
	//TODO create log file
	return TRUE;
}
void shutdown_logger() {
	//TODO cleanup logging/write queued entries
}
void m_log(log_level level, const char* message, ...) {
	//TODO add to a thread
	const char* levels[6] = { "FATAL", "ERROR", "WARNING","DEBUG", "INFO", "TRACE"};
	b8 is_error = level > LOG_LEVEL_WARN;
	const i32 msg_length = 32000;
	char buffer[msg_length]; //TODO mind the limit

	va_list args;
	va_start(args, message);
	int written = snprintf(buffer, sizeof(buffer), "[%s]: ", levels[level]);	//prepend level
	vsnprintf(buffer + written, sizeof(buffer) - written, message, args);		//pass message
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "\n");	//null terminate
	va_end(args);

	if (is_error) {
		platform_console_write_error(buffer, level);
	} else {
		platform_console_write(buffer, level);
	}
}
