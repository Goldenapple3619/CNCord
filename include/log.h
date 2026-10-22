#ifndef _LOG_H_
    #define _LOG_H_

    #include "core.h"

    #define MAX_LOG_LINE_SIZE 1024

    typedef enum {
        LOG_LVL_DEBUG,
        LOG_LVL_USER,
        LOG_LVL_INFO,
        LOG_LVL_WARN,
        LOG_LVL_ERROR
    } LogLevel;

    typedef void (*LogCallback)(LogLevel level, const char *msg, void *userdata);

    CN_API void set_log_callback(LogCallback cb, void *userdata);
    CN_API void set_log_level(LogLevel min_level);
    CN_API void default_sink(LogLevel level, const char *msg, void *userdata);

    #if defined(__MINGW32__) || defined(__MINGW64__)
        CN_API void log_msg(LogLevel level, const char *fmt, ...) __attribute__((format(gnu_printf, 2, 3)));
    #elif defined(__GNUC__) || defined(__clang__)
        CN_API void log_msg(LogLevel level, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
    #else
        CN_API void log_msg(LogLevel level, const char *fmt, ...);
    #endif

    #define LOG_D(...) log_msg(LOG_LVL_DEBUG, __VA_ARGS__)
    #define LOG_U(...) log_msg(LOG_LVL_USER, __VA_ARGS__)
    #define LOG_I(...) log_msg(LOG_LVL_INFO,  __VA_ARGS__)
    #define LOG_W(...) log_msg(LOG_LVL_WARN,  __VA_ARGS__)
    #define LOG_E(...) log_msg(LOG_LVL_ERROR, __VA_ARGS__)
#endif
