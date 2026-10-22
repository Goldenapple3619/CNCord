#ifndef _THREAD_H_
    #define _THREAD_H_

    #include "core.h"

    #ifdef _WIN32
        #ifndef WIN32_LEAN_AND_MEAN
            #define WIN32_LEAN_AND_MEAN
        #endif
        #include <windows.h>

        typedef HANDLE Thread;
        typedef CRITICAL_SECTION Mutex;
        typedef volatile LONG AtomicFlag;

        #define flag_set(f, v)  ((void)InterlockedExchange((f), ((v) ? 1 : 0)))
        #define flag_get(f)     (InterlockedCompareExchange((f), 0, 0) != 0)
    #else
        #include <pthread.h>
        #include <stdatomic.h>

        typedef pthread_t Thread;
        typedef pthread_mutex_t Mutex;
        typedef atomic_int AtomicFlag;

        #define flag_set(f, v)  atomic_store((f), ((v) ? 1 : 0))
        #define flag_get(f)     (atomic_load(f) != 0)
    #endif

    typedef void (*ThreadFn)(void *arg);

    typedef struct {
        ThreadFn fn;
        void *arg;
    } ThreadStart;

    CN_API uint8_t thread_create(Thread *t, ThreadFn fn, void *arg);
    CN_API void thread_join(Thread *t);
    CN_API void mutex_init(Mutex *m);
    CN_API void mutex_lock(Mutex *m);
    CN_API void mutex_unlock(Mutex *m);
    CN_API void mutex_destroy(Mutex *m);
#endif