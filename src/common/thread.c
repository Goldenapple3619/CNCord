#ifndef _WIN32
    #ifndef _POSIX_C_SOURCE
        #define _POSIX_C_SOURCE 200809L
    #endif
#endif

#include "thread.h"
#include "error.h"

#include <stdlib.h>

#ifdef _WIN32
    #include <process.h>

    static unsigned __stdcall thread_tramp(void *p)
    {
        ThreadStart s = *(ThreadStart *)p;

        free(p);
        s.fn(s.arg);
        return (0);
    }

    CN_API uint8_t thread_create(Thread *t, ThreadFn fn, void *arg)
    {
        ThreadStart *s = (ThreadStart *)malloc(sizeof *s);
        uintptr_t h;

        if (!s) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate thread start data.");
            return (1);
        }

        s->fn = fn;
        s->arg = arg;

        h = _beginthreadex(NULL, 0, thread_tramp, s, 0, NULL);
        if (!h) {
            free(s);
            RAISE(ERR_OS, "failed to create thread.");
            return (1);
        }

        *t = (HANDLE)h;
        return (0);
    }

    CN_API void thread_join(Thread *t)
    {
        WaitForSingleObject(*t, INFINITE);
        CloseHandle(*t);
    }

    CN_API void mutex_init(Mutex *m)    { InitializeCriticalSection(m); }
    CN_API void mutex_lock(Mutex *m)    { EnterCriticalSection(m); }
    CN_API void mutex_unlock(Mutex *m)  { LeaveCriticalSection(m); }
    CN_API void mutex_destroy(Mutex *m) { DeleteCriticalSection(m); }
#else
    #include <signal.h>

    static void *thread_tramp(void *p)
    {
        ThreadStart s = *(ThreadStart *)p;

        free(p);
        s.fn(s.arg);
        return (NULL);
    }

    CN_API uint8_t thread_create(Thread *t, ThreadFn fn, void *arg)
    {
        ThreadStart *s = (ThreadStart *)malloc(sizeof *s);
        sigset_t block, old;
        int rc;

        if (!s) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate thread start data.");
            return (1);
        }

        s->fn = fn;
        s->arg = arg;

        sigemptyset(&block);
        sigaddset(&block, SIGINT);
        sigaddset(&block, SIGTERM);
        pthread_sigmask(SIG_BLOCK, &block, &old);

        rc = pthread_create(t, NULL, thread_tramp, s);

        pthread_sigmask(SIG_SETMASK, &old, NULL);

        if (rc != 0) {
            free(s);
            RAISE_FMT(ERR_OS, "failed to create thread (%d).", rc);
            return (1);
        }
        return (0);
    }

    CN_API void thread_join(Thread *t)  { pthread_join(*t, NULL); }

    CN_API void mutex_init(Mutex *m)    { pthread_mutex_init(m, NULL); }
    CN_API void mutex_lock(Mutex *m)    { pthread_mutex_lock(m); }
    CN_API void mutex_unlock(Mutex *m)  { pthread_mutex_unlock(m); }
    CN_API void mutex_destroy(Mutex *m) { pthread_mutex_destroy(m); }
#endif