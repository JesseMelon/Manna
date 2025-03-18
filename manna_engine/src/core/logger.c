#include "logger.h"
#include "asserts.h"    // implements report assertion fail
#include "core/mstring.h"
#include "platform/filesystem.h"
#include "platform/platform.h"

//TODO temporary
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

typedef struct logger_state {
    file_handle log_file_handle;
} logger_state;

void report_assertion_fail(const char* file, int line, const char* expression, const char* message) {
	m_log(LOG_LEVEL_ERROR, "Assertion Failed: %s File: %s Line: %d Message: %s\n", expression, file, line, message);
}

static logger_state* state_ptr;

void append_to_logfile(const char* message) {
    if (state_ptr && state_ptr->log_file_handle.is_valid) {
        u64 length = get_string_length(message);
        u64 written = 0;
        if (!filesystem_write(&state_ptr->log_file_handle, length, message, &written)) {
            platform_console_write("Error writing to log", LOG_LEVEL_ERROR);
        }
    }
}

b8 init_logger(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(logger_state);
    if (state == 0) {
        return TRUE;
    }

    state_ptr = state;

    if (!filesystem_open("console.log", FILE_MODE_WRITE, false, &state_ptr->log_file_handle)) {
        platform_console_write("Unable to open console.log", LOG_LEVEL_ERROR);
    }

    //TODO: remove this
	//LOG_FATAL("Fatal! %f:", 1.0);
	LOG_ERROR("Error!");
	LOG_WARN("Warn!");
	LOG_DEBUG("Debug!");
	LOG_INFO("Info!");
	LOG_TRACE("Trace!");

	//TODO create log file
	return TRUE;
}
void shutdown_logger(void* state) {
	//TODO cleanup logging/write queued entries
}
void m_log(log_level level, const char* message, ...) {
	//TODO add to a thread
	const char* levels[6] = { "FATAL", "ERROR", "WARNING","DEBUG", "INFO", "TRACE"};
	b8 is_error = level > LOG_LEVEL_WARN;
#define MESSAGE_LENGTH 32000
	char buffer[MESSAGE_LENGTH]; //TODO mind the limit

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

    append_to_logfile(message);
}
