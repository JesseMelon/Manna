#pragma once
#include "../defines.h"

#define LOG_WARN_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_TRACE_ENABLED 1

#if RELEASE == 1
	#define LOG_DEBUG_ENABLED 0
	#define LOG_TRACE_ENABLED 0
#endif

typedef enum log_level{
	LOG_FATAL = 0,
	LOG_ERROR = 1,
	LOG_WARN = 2,
	LOG_DEBUG = 3,
	LOG_INFO = 4,
	LOG_TRACE = 5
} log_level;

b8 init_logger();
void shutdown_logger();

MANNA_API void m_log(log_level level, const char *message, ...); // variadic args for passing parameters into the out string

#define M_FATAL(message, ...) m_log(LOG_FATAL, message, ##__VA_ARGS__) //NOTE modern compilers should not need the ## to remove the trailing comma
#define M_ERROR(message, ...) m_log(LOG_ERROR, message, ##__VA_ARGS__)

#if LOG_WARN_ENABLED == 1
#define M_WARN(message, ...) m_log(LOG_WARN, message, ##__VA_ARGS__)
#else
#define LOG_WARN(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
#define M_DEBUG(message, ...) m_log(LOG_DEBUG, message, ##__VA_ARGS__)
#else
#define LOG_DEBUG(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
#define M_INFO(message, ...) m_log(LOG_INFO, message, ##__VA_ARGS__)
#else
#define LOG_INFO(message, ...)
#endif

#if LOG_TRACE_ENABLED == 1
#define M_TRACE(message, ...) m_log(LOG_TRACE, message, ##__VA_ARGS__)
#else
#define LOG_TRACE(message, ...)
#endif
