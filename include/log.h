/**
 * THIS FILE DEFINES MACROS FOR INTERNAL LOGGING PURPOSES: THEY ARE NOT
 * INTENDED TO BE USED BY CLIENT CODE
 *
 */
#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>

#ifdef _DEBUG
/**
 * @brief Logs the given formatted message using the given prefix.
 *
 * @param prefix the prefix to use when logging this message
 */
#define _LOG(prefix, ...) printf("[" prefix "] " __VA_ARGS__)

#if _DEBUG >= 3
/**
 * Prints the formatted message with level LOG
 */
#define LOG(...) _LOG("LOG", __VA_ARGS__)
#else
#define LOG(...)
#endif

#if _DEBUG >= 2
/**
 * Prints the formatted message with level INFO
 */
#define INFO(...) _LOG("INFO", __VA_ARGS__)
#else
#define INFO(...)
#endif

#if _DEBUG >= 1
/**
 * Prints the formatted message with level ALARM
 */
#define ALARM(...) _LOG("ALARM", __VA_ARGS__)
#else
#define ALARM(...)
#endif
#else
#define LOG(...)
#define INFO(...)
#define ALARM(...)
#endif

/**
 * Prints the formatted message with level ERROR
 */
#define ERROR(...) fprintf(stderr, "[ERROR] " __VA_ARGS__)

#endif // _LOG_H_
