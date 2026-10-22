#include "core.h"
#include <time.h>

CN_API uint64_t now_ms(void)
{
    #ifdef _WIN32
        return (long long)GetTickCount64();
    #else
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    #endif
}

CN_API void sleep_ms(uint64_t ms)
{
    if (ms <= 0)
        return;

    #ifdef _WIN32
        Sleep((DWORD)ms);
    #else
        struct timespec ts;

        ts.tv_sec  = (time_t)(ms / 1000);
        ts.tv_nsec = (ms % 1000) * 1000000L;

        nanosleep(&ts, NULL);
    #endif
}
