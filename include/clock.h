#ifndef _CLOCK_H_
    #define _CLOCK_H_

    #include "core.h"

    #ifndef _WIN32
        #ifndef _POSIX_C_SOURCE
            #define _POSIX_C_SOURCE 200809L
        #endif
    #endif

    CN_API uint64_t now_ms(void);
    CN_API void sleep_ms(uint64_t ms);
#endif
