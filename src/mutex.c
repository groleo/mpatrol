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
#ident "$Id: mutex.c,v 1.6 2000-05-24 00:42:07 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


/* A mutex provides a way to lock a data structure in an atomic way.
 */

#if TARGET == TARGET_UNIX
typedef pthread_mutex_t mutex;        /* POSIX threads mutex */
#elif TARGET == TARGET_AMIGA
typedef struct SignalSemaphore mutex; /* Amiga semaphore */
#elif TARGET == TARGET_WINDOWS
typedef HANDLE mutex;                 /* Windows handle */
#elif TARGET == TARGET_NETWARE
typedef LONG mutex;                   /* Netware handle */
#endif /* TARGET */


/* A recursive mutex allows a thread to relock a mutex that is currently
 * locked by that thread.
 */

typedef struct recmutex
{
    mutex guard;         /* guard mutex */
    mutex real;          /* actual mutex */
    unsigned long owner; /* owning thread */
    unsigned long count; /* recursion count */
}
recmutex;


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


#if TARGET == TARGET_UNIX
/* We can make use of the POSIX threads static initialisation of mutexes
 * feature in order to initialise the array of mutex locks, which has the
 * added advantage of not having to delete the mutexes at the end of program
 * execution.  However, care must be taken to ensure that there are exactly
 * the same number of initialisers as there are mutex types.
 */

static recmutex locks[MT_MAX] =
{
    {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER, 0, 0}
};
#else /* TARGET */
/* We will have to initialise the array of mutex locks at run-time.  This
 * means that we must find a way of initialising the mutexes before the
 * mpatrol library is initialised.
 */
static recmutex locks[MT_MAX];


#ifdef __cplusplus
/* C++ provides a way to initialise the array of mutex locks before main()
 * is called.  Unfortunately, that may not be early enough if a system
 * startup module allocates dynamic memory.
 */

static struct initmutexes
{
    initmutexes()
    {
        __mp_initmutexes();
    }
}
initlocks;
#endif /* __cplusplus */
#endif /* TARGET */


/* Initialise the mpatrol library mutexes.  We're up a brown smelly creek if
 * any of these functions dynamically allocate memory.
 */

MP_GLOBAL void __mp_initmutexes(void)
{
#if TARGET == TARGET_AMIGA || TARGET == TARGET_WINDOWS || \
    TARGET == TARGET_NETWARE
    unsigned long i;
#endif /* TARGET */

#if TARGET == TARGET_AMIGA || TARGET == TARGET_WINDOWS || \
    TARGET == TARGET_NETWARE
    for (i = 0; i < MT_MAX; i++)
    {
#if TARGET == TARGET_AMIGA
        InitSemaphore(&locks[i].guard);
        InitSemaphore(&locks[i].real);
#elif TARGET == TARGET_WINDOWS
        locks[i].guard = CreateMutex(NULL, 0, NULL);
        locks[i].real = CreateMutex(NULL, 0, NULL);
#elif TARGET == TARGET_NETWARE
        locks[i].guard = OpenLocalSemaphore(1);
        locks[i].real = OpenLocalSemaphore(1);
#endif /* TARGET */
        locks[i].owner = 0;
        locks[i].count = 0;
    }
#endif /* TARGET */
}


/* Remove the mpatrol library mutexes.  We're up another one of those creeks
 * if the mutexes are still locked or are likely to be used in the future.
 */

MP_GLOBAL void __mp_finimutexes(void)
{
#if TARGET == TARGET_WINDOWS || TARGET == TARGET_NETWARE
    unsigned long i;
#endif /* TARGET */

#if TARGET == TARGET_WINDOWS || TARGET == TARGET_NETWARE
    for (i = 0; i < MT_MAX; i++)
    {
#if TARGET == TARGET_WINDOWS
        CloseHandle(locks[i].guard);
        CloseHandle(locks[i].real);
#elif TARGET == TARGET_NETWARE
        CloseLocalSemaphore(locks[i].guard);
        CloseLocalSemaphore(locks[i].real);
#endif /* TARGET */
        locks[i].owner = 0;
        locks[i].count = 0;
    }
#endif /* TARGET */
}


/* Lock an mpatrol library mutex.
 */

static void lockmutex(mutex *m)
{
#if TARGET == TARGET_UNIX
    pthread_mutex_lock(m);
#elif TARGET == TARGET_AMIGA
    ObtainSemaphore(m);
#elif TARGET == TARGET_WINDOWS
    WaitForSingleObject(*m, INFINITE);
#elif TARGET == TARGET_NETWARE
    WaitOnLocalSemaphore(*m);
#endif /* TARGET */
}


/* Unlock an mpatrol library mutex.
 */

static void unlockmutex(mutex *m)
{
#if TARGET == TARGET_UNIX
    pthread_mutex_unlock(m);
#elif TARGET == TARGET_AMIGA
    ReleaseSemaphore(m);
#elif TARGET == TARGET_WINDOWS
    ReleaseMutex(*m);
#elif TARGET == TARGET_NETWARE
    SignalLocalSemaphore(*m);
#endif /* TARGET */
}


/* Lock an mpatrol library recursive mutex.
 */

MP_GLOBAL void __mp_lockmutex(mutextype m)
{
    recmutex *l;
    unsigned long i;

    l = &locks[m];
    i = __mp_threadid();
    while (1)
    {
        lockmutex(&l->guard);
        if (l->count == 0)
        {
            l->owner = i;
            l->count = 1;
            unlockmutex(&l->guard);
            lockmutex(&l->real);
            break;
        }
        else if (l->owner == i)
        {
            l->count++;
            unlockmutex(&l->guard);
            break;
        }
        unlockmutex(&l->guard);
    }
}


/* Unlock an mpatrol library recursive mutex.
 */

MP_GLOBAL void __mp_unlockmutex(mutextype m)
{
    recmutex *l;
    unsigned long i;

    l = &locks[m];
    i = __mp_threadid();
    lockmutex(&l->guard);
    if (l->count > 0)
        l->count--;
    if ((l->owner == i) && (l->count == 0))
        unlockmutex(&l->real);
    unlockmutex(&l->guard);
}


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


#ifdef __cplusplus
}
#endif /* __cplusplus */
