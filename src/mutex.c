/*
 * mpatrol
 * A library for controlling and tracing dynamic memory allocations.
 * Copyright (C) 1997-2000 Graeme S. Roy <graeme@epc.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307, USA.
 */


/*
 * Threads interface.  Provides thread-safe facilities for locking
 * data structures used by the mpatrol library.
 */


#include "mutex.h"
#include "inter.h"
#include <stddef.h>
#if TARGET == TARGET_UNIX
#include <pthread.h>
#elif TARGET == TARGET_AMIGA
#include <proto/exec.h>
#include <exec/semaphores.h>
#elif TARGET == TARGET_WINDOWS
#include <windows.h>
#include <winbase.h>
#elif TARGET == TARGET_NETWARE
#include <nwsemaph.h>
#include <nwthread.h>
#endif /* TARGET */


#if MP_IDENT_SUPPORT
#ident "$Id: mutex.c,v 1.2 2000-01-09 20:35:18 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


/* A mutex provides a way to lock a data structure in an atomic way.
 */

typedef struct mutex
{
#if TARGET == TARGET_UNIX
    pthread_mutex_t handle;        /* POSIX threads mutex */
#elif TARGET == TARGET_AMIGA
    struct SignalSemaphore handle; /* Amiga semaphore */
#elif TARGET == TARGET_WINDOWS
    HANDLE handle;                 /* Windows handle */
#elif TARGET == TARGET_NETWARE
    LONG handle;                   /* Netware handle */
#endif /* TARGET */
    char init;                     /* initialisation flag */
}
mutex;


#ifdef __cplusplus
/* C++ provides a way of initialising the mpatrol library before
 * main() is called.  Unfortunately, that may not be early enough
 * if a system startup module allocates dynamic memory.
 */

struct libinit
{
    libinit()
    {
        __mp_init();
    }
};
#endif /* __cplusplus */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* The mutex for the library.  This should not really be a file scope
 * variable as it prevents this module from being re-entrant.
 */

static mutex lock;


#if TARGET == TARGET_UNIX
/* We can make use of the POSIX threads function pthread_once() in
 * order to prevent the mpatrol library being initialised more than
 * once at the same time.
 */

static pthread_once_t lock_flag = PTHREAD_ONCE_INIT;
#endif /* TARGET */


#ifdef __cplusplus
/* The library initialiser variable.  As this is a file scope variable,
 * C++ must initialise it before main() is called, which will hopefully
 * be before any calls are made to dynamically allocate memory.
 */

static libinit init;
#endif /* __cplusplus */


/* Return the identifier of the currently running thread.
 */

MP_GLOBAL unsigned long __mp_threadid(void)
{
#if TARGET == TARGET_UNIX
    return (unsigned long) pthread_self();
#elif TARGET == TARGET_AMIGA
    return (unsigned long) FindTask(NULL);
#elif TARGET == TARGET_WINDOWS
    return (unsigned long) GetCurrentThreadId();
#elif TARGET == TARGET_NETWARE
    return (unsigned long) GetThreadId();
#endif /* TARGET */
}


/* Initialise the mpatrol library mutex.
 */

MP_GLOBAL void __mp_newmutex(void)
{
    if (lock.init)
        return;
    lock.init = 1;
#if TARGET == TARGET_UNIX
    pthread_mutex_init(&lock.handle, NULL);
#elif TARGET == TARGET_AMIGA
    InitSemaphore(&lock.handle);
#elif TARGET == TARGET_WINDOWS
    lock.handle = CreateMutex(NULL, 0, NULL);
#elif TARGET == TARGET_NETWARE
    lock.handle = OpenLocalSemaphore(1);
#endif /* TARGET */
}


/* Remove the mpatrol library mutex.
 */

MP_GLOBAL void __mp_deletemutex(void)
{
    if (!lock.init)
        return;
    lock.init = 0;
#if TARGET == TARGET_UNIX
    pthread_mutex_destroy(&lock.handle);
#elif TARGET == TARGET_WINDOWS
    CloseHandle(lock.handle);
#elif TARGET == TARGET_NETWARE
    CloseLocalSemaphore(lock.handle);
#endif /* TARGET */
}


/* Lock the mpatrol library mutex.
 */

MP_GLOBAL void __mp_lockmutex(void)
{
#if TARGET == TARGET_UNIX
    pthread_once(&lock_flag, __mp_newmutex);
#else /* TARGET */
    if (!lock.init)
        __mp_newmutex();
#endif /* TARGET */
#if TARGET == TARGET_UNIX
    pthread_mutex_lock(&lock.handle);
#elif TARGET == TARGET_AMIGA
    ObtainSemaphore(&lock.handle);
#elif TARGET == TARGET_WINDOWS
    WaitForSingleObject(lock.handle, INFINITE);
#elif TARGET == TARGET_NETWARE
    WaitOnLocalSemaphore(lock.handle);
#endif /* TARGET */
}


/* Unlock the mpatrol library mutex.
 */

MP_GLOBAL void __mp_unlockmutex(void)
{
    if (!lock.init)
        return;
#if TARGET == TARGET_UNIX
    pthread_mutex_unlock(&lock.handle);
#elif TARGET == TARGET_AMIGA
    ReleaseSemaphore(&lock.handle);
#elif TARGET == TARGET_WINDOWS
    ReleaseMutex(lock.handle);
#elif TARGET == TARGET_NETWARE
    SignalLocalSemaphore(lock.handle);
#endif /* TARGET */
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
