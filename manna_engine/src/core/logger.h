#pragma once
#include "defines.h"

#define LOG_WARN_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_TRACE_ENABLED 1

#if RELEASE == 1
	#define LOG_DEBUG_ENABLED 0
	#define LOG_TRACE_ENABLED 0
#endif

typedef enum log_level{
	LOG_LEVEL_FATAL = 0,
	LOG_LEVEL_ERROR = 1,
	LOG_LEVEL_WARN = 2,
	LOG_LEVEL_DEBUG = 3,
	LOG_LEVEL_INFO = 4,
	LOG_LEVEL_TRACE = 5
} log_level;

b8 init_logger();
void shutdown_logger();

MANNA_API void m_log(log_level level, const char *message, ...); // variadic args for passing parameters into the out string

#define LOG_FATAL(message, ...) m_log(LOG_LEVEL_FATAL, message, ##__VA_ARGS__) //NOTE modern compilers should not need the ## to remove the trailing comma
#define LOG_ERROR(message, ...) m_log(LOG_LEVEL_ERROR, message, ##__VA_ARGS__)

#if LOG_WARN_ENABLED == 1
#define LOG_WARN(message, ...) m_log(LOG_LEVEL_WARN, message, ##__VA_ARGS__)
#else
#define LOG_WARN(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
#define LOG_DEBUG(message, ...) m_log(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__)
#else
#define LOG_DEBUG(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
#define LOG_INFO(message, ...) m_log(LOG_LEVEL_INFO, message, ##__VA_ARGS__)
#else
#define LOG_INFO(message, ...)
#endif

#if LOG_TRACE_ENABLED == 1
#define LOG_TRACE(message, ...) m_log(LOG_LEVEL_TRACE, message, ##__VA_ARGS__)
#else
#define LOG_TRACE(message, ...)
#endif
