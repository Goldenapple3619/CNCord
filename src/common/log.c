#include "log.h"

static LogCallback g_cb = default_sink;
static void *g_userdata = NULL;
static LogLevel g_min_level = LOG_LVL_INFO;

CN_API void default_sink(LogLevel level, const char *msg, void *userdata)
{
    (void)userdata;

    static const char *names[] = {"DEBUG", "USER", "INFO", "WARN", "ERROR"};
    FILE *out = (level >= LOG_LVL_WARN) ? stderr : stdout;

    fprintf(out, "[%s] %s\n", names[level], msg);
    fflush(out);
}

CN_API void set_log_callback(LogCallback cb, void *userdata)
{
    g_cb = cb ? cb : default_sink;
    g_userdata = cb ? userdata : NULL;
}

CN_API void set_log_level(LogLevel min_level)
{
    g_min_level = min_level;
}

CN_API void log_msg(LogLevel level, const char *fmt, ...)
{
    char buf[MAX_LOG_LINE_SIZE];
    va_list args;

    if (level < g_min_level)
        return;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    g_cb(level, buf, g_userdata);
}
