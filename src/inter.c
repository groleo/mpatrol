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
 * Library interface.  The module defines the visible interface for the
 * mpatrol library.
 */


#include "inter.h"
#include "diag.h"
#if MP_THREADS_SUPPORT
#include "mutex.h"
#endif /* MP_THREADS_SUPPORT */
#if TARGET == TARGET_WINDOWS
#include "sbrk.h"
#endif /* TARGET */
#include "option.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>


#if MP_IDENT_SUPPORT
#ident "$Id: inter.c,v 1.22 2000-04-03 18:21:41 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


#if MP_INUSE_SUPPORT
int _Inuse_init(unsigned long, int);
void _Inuse_close(void);
#endif /* MP_INUSE_SUPPORT */


/* The memory header structure used by the library to hold all of its data
 * and settings.
 */

static infohead memhead;


#if TARGET == TARGET_WINDOWS
/* These are global variables used by the Microsoft C run-time library to
 * indicate initialisation of the environment variables, the exit function
 * table and the streams buffers respectively.  The run-time library calls
 * malloc() and related functions to set these up so we cannot initialise
 * the mpatrol library until all of these have been set up.  Instead, we
 * will call sbrk() to allocate enough memory for these as they appear, but
 * we cannot record anything about these allocations.
 */

extern int __env_initialized;
extern void *__onexitbegin;
extern void **__piob;
#endif /* TARGET */


/* Save the current signal handlers and set them to ignore.  Also lock the
 * library data structures if we are thread-safe.
 */

static void savesignals(void)
{
    /* Only perform this step if we are not doing a recursive call.
     */
    if (!memhead.recur)
    {
#if MP_THREADS_SUPPORT
        __mp_lockmutex();
#endif /* MP_THREADS_SUPPORT */
        if (!memhead.init)
            __mp_initsignals(&memhead.signals);
        if (memhead.flags & FLG_SAFESIGNALS)
            __mp_savesignals(&memhead.signals);
    }
    memhead.recur++;
}


/* Restore the previous signal handlers.  Also unlock the library data
 * structures if we are thread-safe.
 */

static void restoresignals(void)
{
    /* Only perform this step if we are not doing a recursive call.
     */
    memhead.recur--;
    if (!memhead.recur)
    {
        __mp_restoresignals(&memhead.signals);
#if MP_THREADS_SUPPORT
        __mp_unlockmutex();
#endif /* MP_THREADS_SUPPORT */
    }
}


/* Check that a specified integer lies in a given range.
 */

static int checkrange(unsigned long l, unsigned long n, unsigned long u)
{
    /* If the lower and upper bounds are zero then the integer never lies in
     * the given range.
     */
    if ((l != 0) || (u != 0))
    {
        if (l == (unsigned long) -1)
            l = 0;
        if ((l <= n) && (n <= u))
            return 1;
    }
    return 0;
}


/* Initialise the mpatrol library.
 */

void __mp_init(void)
{
    savesignals();
    if (memhead.fini)
        /* We currently don't allow the library to be reinitialised.
         */
        __mp_abort();
    if (!memhead.init)
    {
        __mp_newinfo(&memhead);
#if MP_INUSE_SUPPORT
        _Inuse_init(0, 0);
#endif /* MP_INUSE_SUPPORT */
        atexit(__mp_fini);
        /* Read any options from the specified environment variable.
         */
        __mp_parseoptions(&memhead);
        /* Set up the random number generator for the FAILFREQ option.
         */
        if (memhead.fseed == 0)
            memhead.fseed = (unsigned long) time(NULL);
        srand(memhead.fseed);
        /* Attempt to open the log file.
         */
        if (!__mp_openlogfile(memhead.log))
            memhead.log = NULL;
        /* Obtain the program filename and attempt to read any symbols from
         * that file.
         */
        if (memhead.alloc.heap.memory.prog != NULL)
            __mp_addsymbols(&memhead.syms, memhead.alloc.heap.memory.prog, 0);
        __mp_addextsymbols(&memhead.syms);
        __mp_fixsymbols(&memhead.syms);
        if (!(memhead.flags & FLG_NOPROTECT))
        {
            __mp_protectsymbols(&memhead.syms, MA_READONLY);
            __mp_protectinfo(&memhead, MA_READONLY);
        }
        __mp_printversion();
    }
    restoresignals();
}


/* Finalise the mpatrol library.
 */

void __mp_fini(void)
{
    savesignals();
    if (memhead.init)
    {
        if (!memhead.fini)
        {
            /* Firstly, check the integrity of the memory blocks.
             */
            __mp_checkinfo(&memhead);
            /* Then close any access library handles that might still be open.
             */
            __mp_closesymbols(&memhead.syms);
            /* Then print a summary of library statistics and settings.
             */
            __mp_printsummary(&memhead);
            /* Then deal with any SHOW options that may have been requested.
             */
            if ((memhead.flags & FLG_SHOWMAP) && (memhead.alloc.list.size > 0))
                __mp_printmap(&memhead);
            if ((memhead.flags & FLG_SHOWSYMBOLS) &&
                (memhead.syms.dtree.size > 0))
                __mp_printsymbols(&memhead.syms);
            if ((memhead.flags & FLG_SHOWFREED) && (memhead.alloc.gsize > 0))
                __mp_printfreed(&memhead);
            if (memhead.alloc.asize > 0)
            {
                if (memhead.flags & FLG_SHOWUNFREED)
                    __mp_printallocs(&memhead, 0);
                if ((memhead.uabort > 0) &&
                    (memhead.alloc.asize >= memhead.uabort))
                    __mp_printallocs(&memhead, 1);
            }
            memhead.fini = 1;
#if MP_INUSE_SUPPORT
            _Inuse_close();
#endif /* MP_INUSE_SUPPORT */
        }
#if MP_DELETE
        /* We only need to perform this step if the operating system does not
         * reclaim memory from a terminated process.  We must not perform this
         * step if the operating system needs to deal with dynamically
         * allocated memory after the library has terminated.
         */
        if (!(memhead.flags & FLG_NOPROTECT))
        {
            __mp_protectinfo(&memhead, MA_READWRITE);
            __mp_protectsymbols(&memhead.syms, MA_READWRITE);
        }
        __mp_deleteinfo(&memhead);
#endif /* MP_DELETE */
#if MP_THREADS_SUPPORT
        __mp_deletemutex();
#endif /* MP_THREADS_SUPPORT */
        memhead.init = 0;
    }
    restoresignals();
}


/* Return the memory header structure.
 */

infohead *__mp_memhead(void)
{
    return &memhead;
}


/* Allocate a new block of memory of a specified size and alignment.
 */

void *__mp_alloc(size_t l, size_t a, alloctype f, char *s, char *t,
                 unsigned long u, size_t k)
{
    void *p;
    stackinfo i;
    int j;

#if TARGET == TARGET_WINDOWS
    /* If the C run-time library has not finished initialising then we must
     * allocate new memory with sbrk().  We don't even attempt to do anything
     * with calls to memalign(), valloc() and pvalloc() but these shouldn't
     * be coming through anyway.
     */
    if (!__env_initialized || !__onexitbegin || !__piob)
    {
        if (l == 0)
            l = 1;
        if ((p = sbrk(l)) == (void *) -1)
            p = NULL;
        else if (f == AT_CALLOC)
            __mp_memset(p, 0, l);
        return p;
    }
#endif /* TARGET */
    savesignals();
    if (!memhead.init)
        __mp_init();
    if (checkrange(memhead.lrange, memhead.count + 1, memhead.urange))
        __mp_checkinfo(&memhead);
    if (memhead.prologue && (memhead.recur == 1))
        memhead.prologue((void *) -1, l);
    /* Determine the call stack details.
     */
    __mp_newframe(&i);
    if (__mp_getframe(&i))
    {
        j = __mp_getframe(&i);
        while ((k > 0) && (j != 0))
        {
            j = __mp_getframe(&i);
            k--;
        }
    }
    /* If no filename was passed through then attempt to read any debugging
     * information to determine the source location of the call.
     */
    if ((memhead.recur == 1) && (t == NULL) && (i.addr != NULL) &&
        __mp_findsource(&memhead.syms, (char *) i.addr - 1, &s, &t, &u))
    {
        if (!(memhead.flags & FLG_NOPROTECT))
            __mp_protectstrtab(&memhead.syms.strings, MA_READWRITE);
        if (s != NULL)
            s = __mp_addstring(&memhead.syms.strings, s);
        if (t != NULL)
            t = __mp_addstring(&memhead.syms.strings, t);
        if (!(memhead.flags & FLG_NOPROTECT))
            __mp_protectstrtab(&memhead.syms.strings, MA_READONLY);
    }
  retry:
    p = __mp_getmemory(&memhead, l, a, f, s, t, u, &i);
    if (memhead.epilogue && (memhead.recur == 1))
        memhead.epilogue(p);
    /* Call the low-memory handler if no memory block was allocated.
     */
    if (p == NULL)
    {
        if (memhead.nomemory)
            memhead.nomemory();
        else if ((f == AT_NEW) || (f == AT_NEWVEC))
        {
            /* The C++ standard specifies that operators new and new[] should
             * always return non-NULL pointers.  Since we have ascertained that
             * we have no low-memory handler, this either means throwing an
             * exception or aborting.  Since this is a no-throw version of new
             * we'll opt for the latter.
             */
            __mp_printsummary(&memhead);
            __mp_diag("\n");
            __mp_error(f, "out of memory");
            memhead.fini = 1;
            __mp_abort();
        }
        if ((f == AT_NEW) || (f == AT_NEWVEC))
        {
            if (memhead.prologue && (memhead.recur == 1))
                memhead.prologue((void *) -1, l);
            goto retry;
        }
    }
    restoresignals();
    return p;
}


/* Allocate a new block of memory to duplicate a string.
 */

char *__mp_strdup(char *p, size_t l, alloctype f, char *s, char *t,
                  unsigned long u, size_t k)
{
    char *o;
    stackinfo i;
    size_t n;
    int j;

#if TARGET == TARGET_WINDOWS
    /* If the C run-time library has not finished initialising then we must
     * allocate new memory with sbrk() and copy the string to the new
     * allocation.
     */
    if (!__piob || !__onexitbegin || !__env_initialized)
    {
        if (p == NULL)
            o = NULL;
        else
        {
            n = strlen(p);
            if (((f == AT_STRNDUP) || (f == AT_STRNSAVE)) && (n > l))
                n = l;
            if ((o = (char *) sbrk(n + 1)) == (void *) -1)
                o = NULL;
            else
                __mp_memcopy(o, p, n + 1);
        }
        return o;
    }
#endif /* TARGET */
    savesignals();
    if (!memhead.init)
        __mp_init();
    if (checkrange(memhead.lrange, memhead.count + 1, memhead.urange))
        __mp_checkinfo(&memhead);
    if (memhead.prologue && (memhead.recur == 1))
        memhead.prologue(p, (size_t) -2);
    /* Determine the call stack details.
     */
    __mp_newframe(&i);
    if (__mp_getframe(&i))
    {
        j = __mp_getframe(&i);
        while ((k > 0) && (j != 0))
        {
            j = __mp_getframe(&i);
            k--;
        }
    }
    /* If no filename was passed through then attempt to read any debugging
     * information to determine the source location of the call.
     */
    if ((memhead.recur == 1) && (t == NULL) && (i.addr != NULL) &&
        __mp_findsource(&memhead.syms, (char *) i.addr - 1, &s, &t, &u))
    {
        if (!(memhead.flags & FLG_NOPROTECT))
            __mp_protectstrtab(&memhead.syms.strings, MA_READWRITE);
        if (s != NULL)
            s = __mp_addstring(&memhead.syms.strings, s);
        if (t != NULL)
            t = __mp_addstring(&memhead.syms.strings, t);
        if (!(memhead.flags & FLG_NOPROTECT))
            __mp_protectstrtab(&memhead.syms.strings, MA_READONLY);
    }
    if (p != NULL)
    {
        /* Determine the size of the string, allocate the memory, then
         * copy the string to the new memory block.
         */
        o = p;
        n = strlen(p);
        if (((f == AT_STRNDUP) || (f == AT_STRNSAVE)) && (n > l))
            n = l;
        if (p = (char *) __mp_getmemory(&memhead, n + 1, 1, f, s, t, u, &i))
            __mp_memcopy(p, o, n + 1);
    }
    if (memhead.epilogue && (memhead.recur == 1))
        memhead.epilogue(p);
    /* Call the low-memory handler if no memory block was allocated.
     */
    if ((p == NULL) && memhead.nomemory)
        memhead.nomemory();
    restoresignals();
    return p;
}


/* Resize an existing block of memory to a new size and alignment.
 */

void *__mp_realloc(void *p, size_t l, size_t a, alloctype f, char *s, char *t,
                   unsigned long u, size_t k)
{
#if TARGET == TARGET_WINDOWS
    void *q;
#endif /* TARGET */
    stackinfo i;
    int j;

#if TARGET == TARGET_WINDOWS
    /* If the C run-time library has not finished initialising then we must
     * allocate new memory with sbrk() and copy the old contents to the new
     * allocation.  We can't free the old allocation as we know nothing
     * about it.
     */
    if (!__env_initialized || !__onexitbegin || !__piob)
    {
        if (p == NULL)
        {
            if (l == 0)
                l = 1;
            if ((q = sbrk(l)) == (void *) -1)
                q = NULL;
            else if (f == AT_RECALLOC)
                __mp_memset(q, 0, l);
        }
        else if ((l == 0) || (f == AT_EXPAND) || ((q = sbrk(l)) == (void *) -1))
            q = NULL;
        else
            __mp_memcopy(q, p, l);
        return q;
    }
#endif /* TARGET */
    savesignals();
    if (!memhead.init)
        __mp_init();
    if (checkrange(memhead.lrange, memhead.count, memhead.urange))
        __mp_checkinfo(&memhead);
    if (memhead.prologue && (memhead.recur == 1))
        memhead.prologue(p, l);
    /* Determine the call stack details.
     */
    __mp_newframe(&i);
    if (__mp_getframe(&i))
    {
        j = __mp_getframe(&i);
        while ((k > 0) && (j != 0))
        {
            j = __mp_getframe(&i);
            k--;
        }
    }
    /* If no filename was passed through then attempt to read any debugging
     * information to determine the source location of the call.
     */
    if ((memhead.recur == 1) && (t == NULL) && (i.addr != NULL) &&
        __mp_findsource(&memhead.syms, (char *) i.addr - 1, &s, &t, &u))
    {
        if (!(memhead.flags & FLG_NOPROTECT))
            __mp_protectstrtab(&memhead.syms.strings, MA_READWRITE);
        if (s != NULL)
            s = __mp_addstring(&memhead.syms.strings, s);
        if (t != NULL)
            t = __mp_addstring(&memhead.syms.strings, t);
        if (!(memhead.flags & FLG_NOPROTECT))
            __mp_protectstrtab(&memhead.syms.strings, MA_READONLY);
    }
    p = __mp_resizememory(&memhead, p, l, a, f, s, t, u, &i);
    if (memhead.epilogue && (memhead.recur == 1))
        memhead.epilogue(p);
    /* Call the low-memory handler if no memory block was allocated.
     */
    if ((p == NULL) && memhead.nomemory)
        memhead.nomemory();
    restoresignals();
    return p;
}


/* Free an existing block of memory.
 */

void __mp_free(void *p, alloctype f, char *s, char *t, unsigned long u,
               size_t k)
{
    stackinfo i;
    int j;

#if TARGET == TARGET_WINDOWS
    /* If the C run-time library has not finished initialising then just
     * return since we know nothing about any of the prior allocations anyway.
     */
    if (!__env_initialized || !__onexitbegin || !__piob || memhead.fini)
#else /* TARGET */
    if (memhead.fini)
#endif /* TARGET */
        return;
    savesignals();
    if (!memhead.init)
        __mp_init();
    if (checkrange(memhead.lrange, memhead.count, memhead.urange))
        __mp_checkinfo(&memhead);
    if (memhead.prologue && (memhead.recur == 1))
        memhead.prologue(p, (size_t) -1);
    /* Determine the call stack details.
     */
    __mp_newframe(&i);
    if (__mp_getframe(&i))
    {
        j = __mp_getframe(&i);
        while ((k > 0) && (j != 0))
        {
            j = __mp_getframe(&i);
            k--;
        }
    }
    /* If no filename was passed through then attempt to read any debugging
     * information to determine the source location of the call.
     */
    if ((memhead.recur == 1) && (t == NULL) && (i.addr != NULL) &&
        __mp_findsource(&memhead.syms, (char *) i.addr - 1, &s, &t, &u))
    {
        if (!(memhead.flags & FLG_NOPROTECT))
            __mp_protectstrtab(&memhead.syms.strings, MA_READWRITE);
        if (s != NULL)
            s = __mp_addstring(&memhead.syms.strings, s);
        if (t != NULL)
            t = __mp_addstring(&memhead.syms.strings, t);
        if (!(memhead.flags & FLG_NOPROTECT))
            __mp_protectstrtab(&memhead.syms.strings, MA_READONLY);
    }
    __mp_freememory(&memhead, p, f, s, t, u, &i);
    if (memhead.epilogue && (memhead.recur == 1))
        memhead.epilogue((void *) -1);
    restoresignals();
}


/* Set a block of memory to contain a specific byte.
 */

void *__mp_setmem(void *p, size_t l, unsigned char c, alloctype f, char *s,
                  char *t, unsigned long u, size_t k)
{
    stackinfo i;
    int j;

    if (!memhead.init || memhead.fini)
    {
        __mp_memset(p, c, l);
        return p;
    }
    savesignals();
    /* Determine the call stack details.
     */
    __mp_newframe(&i);
    if (__mp_getframe(&i))
    {
        j = __mp_getframe(&i);
        while ((k > 0) && (j != 0))
        {
            j = __mp_getframe(&i);
            k--;
        }
    }
    /* If no filename was passed through then attempt to read any debugging
     * information to determine the source location of the call.
     */
    if ((memhead.recur == 1) && (t == NULL) && (i.addr != NULL) &&
        __mp_findsource(&memhead.syms, (char *) i.addr - 1, &s, &t, &u))
    {
        if (!(memhead.flags & FLG_NOPROTECT))
            __mp_protectstrtab(&memhead.syms.strings, MA_READWRITE);
        if (s != NULL)
            s = __mp_addstring(&memhead.syms.strings, s);
        if (t != NULL)
            t = __mp_addstring(&memhead.syms.strings, t);
        if (!(memhead.flags & FLG_NOPROTECT))
            __mp_protectstrtab(&memhead.syms.strings, MA_READONLY);
    }
    __mp_setmemory(&memhead, p, l, c, f, s, t, u, &i);
    restoresignals();
    return p;
}


/* Copy a block of memory from one address to another.
 */

void *__mp_copymem(void *p, void *q, size_t l, alloctype f, char *s, char *t,
                   unsigned long u, size_t k)
{
    stackinfo i;
    int j;

    if (!memhead.init || memhead.fini)
    {
        __mp_memcopy(q, p, l);
        return q;
    }
    savesignals();
    /* Determine the call stack details.
     */
    __mp_newframe(&i);
    if (__mp_getframe(&i))
    {
        j = __mp_getframe(&i);
        while ((k > 0) && (j != 0))
        {
            j = __mp_getframe(&i);
            k--;
        }
    }
    /* If no filename was passed through then attempt to read any debugging
     * information to determine the source location of the call.
     */
    if ((memhead.recur == 1) && (t == NULL) && (i.addr != NULL) &&
        __mp_findsource(&memhead.syms, (char *) i.addr - 1, &s, &t, &u))
    {
        if (!(memhead.flags & FLG_NOPROTECT))
            __mp_protectstrtab(&memhead.syms.strings, MA_READWRITE);
        if (s != NULL)
            s = __mp_addstring(&memhead.syms.strings, s);
        if (t != NULL)
            t = __mp_addstring(&memhead.syms.strings, t);
        if (!(memhead.flags & FLG_NOPROTECT))
            __mp_protectstrtab(&memhead.syms.strings, MA_READONLY);
    }
    __mp_copymemory(&memhead, p, q, l, f, s, t, u, &i);
    restoresignals();
    return q;
}


/* Attempt to locate the position of one block of memory in another block.
 */

void *__mp_locatemem(void *p, size_t l, void *q, size_t m, alloctype f, char *s,
                     char *t, unsigned long u, size_t k)
{
    void *r;
    stackinfo i;
    int j;
    unsigned char b;

    if (f == AT_MEMCHR)
    {
        /* If the function that called us was memchr() then we must convert the
         * second length to a character and set up the new pointer and length.
         */
        b = (unsigned char) (m & 0xFF);
        q = (void *) &b;
        m = 1;
    }
    if (!memhead.init || memhead.fini)
        return __mp_memfind(p, l, q, m);
    savesignals();
    /* Determine the call stack details.
     */
    __mp_newframe(&i);
    if (__mp_getframe(&i))
    {
        j = __mp_getframe(&i);
        while ((k > 0) && (j != 0))
        {
            j = __mp_getframe(&i);
            k--;
        }
    }
    /* If no filename was passed through then attempt to read any debugging
     * information to determine the source location of the call.
     */
    if ((memhead.recur == 1) && (t == NULL) && (i.addr != NULL) &&
        __mp_findsource(&memhead.syms, (char *) i.addr - 1, &s, &t, &u))
    {
        if (!(memhead.flags & FLG_NOPROTECT))
            __mp_protectstrtab(&memhead.syms.strings, MA_READWRITE);
        if (s != NULL)
            s = __mp_addstring(&memhead.syms.strings, s);
        if (t != NULL)
            t = __mp_addstring(&memhead.syms.strings, t);
        if (!(memhead.flags & FLG_NOPROTECT))
            __mp_protectstrtab(&memhead.syms.strings, MA_READONLY);
    }
    r = __mp_locatememory(&memhead, p, l, q, m, f, s, t, u, &i);
    restoresignals();
    return r;
}


/* Compare two blocks of memory.
 */

int __mp_comparemem(void *p, void *q, size_t l, alloctype f, char *s, char *t,
                    unsigned long u, size_t k)
{
    void *m;
    stackinfo i;
    int j, r;

    if (!memhead.init || memhead.fini)
        if (m = __mp_memcompare(p, q, l))
        {
            l = (char *) m - (char *) p;
            return (int) ((char *) p)[l] - (int) ((char *) q)[l];
        }
        else
            return 0;
    savesignals();
    /* Determine the call stack details.
     */
    __mp_newframe(&i);
    if (__mp_getframe(&i))
    {
        j = __mp_getframe(&i);
        while ((k > 0) && (j != 0))
        {
            j = __mp_getframe(&i);
            k--;
        }
    }
    /* If no filename was passed through then attempt to read any debugging
     * information to determine the source location of the call.
     */
    if ((memhead.recur == 1) && (t == NULL) && (i.addr != NULL) &&
        __mp_findsource(&memhead.syms, (char *) i.addr - 1, &s, &t, &u))
    {
        if (!(memhead.flags & FLG_NOPROTECT))
            __mp_protectstrtab(&memhead.syms.strings, MA_READWRITE);
        if (s != NULL)
            s = __mp_addstring(&memhead.syms.strings, s);
        if (t != NULL)
            t = __mp_addstring(&memhead.syms.strings, t);
        if (!(memhead.flags & FLG_NOPROTECT))
            __mp_protectstrtab(&memhead.syms.strings, MA_READONLY);
    }
    r = __mp_comparememory(&memhead, p, q, l, f, s, t, u, &i);
    restoresignals();
    return r;
}


/* Obtain any details about the memory block that contains a given address.
 */

int __mp_info(void *p, allocinfo *d)
{
    addrnode *a;
    symnode *s;
    allocnode *n;
    infonode *m;

    savesignals();
    if (!memhead.init)
        __mp_init();
    /* Check that we know something about the address that was supplied.
     */
    if (((n = __mp_findalloc(&memhead.alloc, p)) == NULL) &&
        ((n = __mp_findfreed(&memhead.alloc, p)) == NULL))
    {
        restoresignals();
        return 0;
    }
    /* We now fill in the details for the supplied structure.
     */
    m = (infonode *) n->info;
    d->block = n->block;
    d->size = n->size;
    d->type = m->data.type;
    d->alloc = m->data.alloc;
    d->realloc = m->data.realloc;
#if MP_THREADS_SUPPORT
    d->thread = m->data.thread;
#else /* MP_THREADS_SUPPORT */
    d->thread = 0;
#endif /* MP_THREADS_SUPPORT */
    d->func = m->data.func;
    d->file = m->data.file;
    d->line = m->data.line;
    d->stack = m->data.stack;
    d->freed = m->data.freed;
    if (!(memhead.flags & FLG_NOPROTECT))
        __mp_protectinfo(&memhead, MA_READWRITE);
    /* The names of the symbols in the call stack may not have been determined
     * yet, so we traverse the stack, filling in any known symbol names as we
     * go.
     */
    for (a = m->data.stack; a != NULL; a = a->data.next)
        if ((a->data.name == NULL) &&
            (s = __mp_findsymbol(&memhead.syms, a->data.addr)))
            a->data.name = s->data.name;
    if ((memhead.recur == 1) && !(memhead.flags & FLG_NOPROTECT))
        __mp_protectinfo(&memhead, MA_READONLY);
    restoresignals();
    return 1;
}


/* Display a complete memory map of the heap and (optionally) a summary of
 * all mpatrol library settings and statistics.
 */

void __mp_memorymap(int s)
{
    savesignals();
    if (!memhead.init)
        __mp_init();
    if (s != 0)
        __mp_printsummary(&memhead);
    if (memhead.alloc.list.size > 0)
        __mp_printmap(&memhead);
    restoresignals();
}


/* Display a summary of all mpatrol library settings and statistics.
 */

void __mp_summary(void)
{
    savesignals();
    if (!memhead.init)
        __mp_init();
    __mp_printsummary(&memhead);
    restoresignals();
}


/* Check the validity of all memory blocks that have been filled with
 * a predefined pattern.
 */

void __mp_check(void)
{
    savesignals();
    if (!memhead.init)
        __mp_init();
    __mp_checkinfo(&memhead);
    restoresignals();
}


/* Set the prologue function and return the previous setting.
 */

void (*__mp_prologue(void (*h)(void *, size_t)))(void *, size_t)
{
    void (*p)(void *, size_t);

    savesignals();
    if (!memhead.init)
        __mp_init();
    p = memhead.prologue;
    memhead.prologue = h;
    restoresignals();
    return p;
}


/* Set the epilogue function and return the previous setting.
 */

void (*__mp_epilogue(void (*h)(void *)))(void *)
{
    void (*p)(void *);

    savesignals();
    if (!memhead.init)
        __mp_init();
    p = memhead.epilogue;
    memhead.epilogue = h;
    restoresignals();
    return p;
}


/* Set the low-memory handler and return the previous setting.
 */

void (*__mp_nomemory(void (*h)(void)))(void)
{
    void (*p)(void);

    savesignals();
    if (!memhead.init)
        __mp_init();
    p = memhead.nomemory;
    memhead.nomemory = h;
    restoresignals();
    return p;
}


/* Push source level information onto the top of the delete stack.
 */

void __mp_pushdelstack(char *s, char *t, unsigned long u)
{
    if (!memhead.init)
        __mp_init();
    if ((memhead.delpos >= 0) && (memhead.delpos < MP_MAXDELSTACK))
    {
        memhead.dels[memhead.delpos].func = s;
        memhead.dels[memhead.delpos].file = t;
        memhead.dels[memhead.delpos].line = u;
    }
    memhead.delpos++;
}


/* Pop source level information off the top of the delete stack.
 */

void __mp_popdelstack(char **s, char **t, unsigned long *u)
{
    if (!memhead.init)
        __mp_init();
    if ((--memhead.delpos >= 0) && (memhead.delpos < MP_MAXDELSTACK))
    {
        *s = memhead.dels[memhead.delpos].func;
        *t = memhead.dels[memhead.delpos].file;
        *u = memhead.dels[memhead.delpos].line;
    }
    else
    {
        *s = *t = NULL;
        *u = 0;
    }
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
