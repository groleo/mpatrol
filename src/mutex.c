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
 * data structures used by the mpatrol library.  An extremely informative
 * book on POSIX threads and multithreaded programming in general is
 * Programming with POSIX Threads, First Edition by David R. Butenhof
 * (Addison-Wesley, 1997, ISBN 0-201-63392-2).
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
#ident "$Id: mutex.c,v 1.3 2000-05-22 00:27:45 graeme Exp $"
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
}
mutex;


/* A recursive mutex allows a thread to relock a mutex that is currently
 * locked by that thread.
 */

typedef struct recmutex
{
    struct mutex guard;  /* guard mutex */
    struct mutex real;   /* actual mutex */
    unsigned long owner; /* owning thread */
    unsigned long count; /* recursion count */
}
recmutex;


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* The mutex for the library.  This should not really be a file scope
 * variable as it prevents this module from being re-entrant.
 */

#if TARGET == TARGET_UNIX
/* We can make use of the POSIX threads static initialisation of mutexes
 * feature in order to initialise the mutexes.
 */

static recmutex lock =
{
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    0,
    0
};
#else /* TARGET */
static recmutex lock;
#endif /* TARGET */


#ifdef __cplusplus
/* C++ provides a way of initialising the mutexes before
 * main() is called.  Unfortunately, that may not be early enough
 * if a system startup module allocates dynamic memory.
 */

struct mutexinit
{
    mutexinit()
    {
        __mp_newmutex();
    }
};


/* The library initialiser variable.  As this is a file scope variable,
 * C++ must initialise it before main() is called, which will hopefully
 * be before any calls are made to dynamically allocate memory.
 */

static mutexinit init;
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
#if TARGET == TARGET_AMIGA
    InitSemaphore(&lock.real.handle);
#elif TARGET == TARGET_WINDOWS
    lock.real.handle = CreateMutex(NULL, 0, NULL);
#elif TARGET == TARGET_NETWARE
    lock.real.handle = OpenLocalSemaphore(1);
#endif /* TARGET */
}


/* Remove the mpatrol library mutex.
 */

MP_GLOBAL void __mp_deletemutex(void)
{
#if TARGET == TARGET_WINDOWS
    CloseHandle(lock.real.handle);
#elif TARGET == TARGET_NETWARE
    CloseLocalSemaphore(lock.real.handle);
#endif /* TARGET */
}


/* Lock an mpatrol library mutex.
 */

static void lockmutex(mutex *m)
{
#if TARGET == TARGET_UNIX
    pthread_mutex_lock(&m->handle);
#elif TARGET == TARGET_AMIGA
    ObtainSemaphore(&m->handle);
#elif TARGET == TARGET_WINDOWS
    WaitForSingleObject(m->handle, INFINITE);
#elif TARGET == TARGET_NETWARE
    WaitOnLocalSemaphore(m->handle);
#endif /* TARGET */
}


/* Lock an mpatrol library recursive mutex.
 */

MP_GLOBAL void __mp_lockrecmutex(void)
{
    unsigned long i;

    i = __mp_threadid();
    lockmutex(&lock.guard);
    if ((lock.owner == i) && (lock.count > 0))
        lock.count++;
    else
    {
        unlockmutex(&lock.guard);
        lockmutex(&lock.real);
        lockmutex(&lock.guard);
        lock.owner = i;
        lock.count = 1;
    }
    unlockmutex(&lock.guard);
}


/* Unlock an mpatrol library mutex.
 */

static void unlockmutex(mutex *m)
{
#if TARGET == TARGET_UNIX
    pthread_mutex_unlock(&m->handle);
#elif TARGET == TARGET_AMIGA
    ReleaseSemaphore(&m->handle);
#elif TARGET == TARGET_WINDOWS
    ReleaseMutex(m->handle);
#elif TARGET == TARGET_NETWARE
    SignalLocalSemaphore(m->handle);
#endif /* TARGET */
}


/* Unlock an mpatrol library recursive mutex.
 */

MP_GLOBAL void __mp_unlockrecmutex(void)
{
    unsigned long i;

    i = __mp_threadid();
    lockmutex(&lock.guard);
    if ((lock.owner == i) && (lock.count > 0))
        lock.count--;
    else
    {
        unlockmutex(&lock.guard);
        unlockmutex(&lock.real);
        lockmutex(&lock.guard);
        lock.owner = i;
        lock.count = 1;
    }
    unlockmutex(&lock.guard);
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
