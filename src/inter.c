/*
 * mpatrol
 * A library for controlling and tracing dynamic memory allocations.
 * Copyright (C) 1997-1999 Graeme S. Roy <graeme@epc.co.uk>
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
#include "option.h"
#include <stdlib.h>
#include <string.h>


#if MP_IDENT_SUPPORT
#ident "$Id: inter.c,v 1.2 1999-10-03 22:50:22 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* The memory header structure used by the library to hold all of its data
 * and settings.
 */

static infohead memhead;


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


/* Initialise the mpatrol library.
 */

#if TARGET == TARGET_AMIGA
__asm void __mp_init(void)
#else /* TARGET */
void __mp_init(void)
#endif /* TARGET */
{
    savesignals();
    if (memhead.fini)
        /* We currently don't allow the library to be reinitialised.
         */
        __mp_abort();
    if (!memhead.init)
    {
        __mp_newinfo(&memhead);
        atexit(__mp_fini);
        /* Read any options from the specified environment variable.
         */
        __mp_parseoptions(&memhead);
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

#if TARGET == TARGET_AMIGA
__asm void __mp_fini(void)
#else /* TARGET */
void __mp_fini(void)
#endif /* TARGET */
{
    savesignals();
    if (memhead.init)
    {
        if (!memhead.fini)
        {
            /* Firstly, check the integrity of the memory blocks.
             */
            __mp_checkinfo(&memhead);
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

#if TARGET == TARGET_AMIGA
__asm infohead *__mp_memhead(void)
#else /* TARGET */
infohead *__mp_memhead(void)
#endif /* TARGET */
{
    return &memhead;
}


/* Allocate a new block of memory of a specified size and alignment.
 */

#if TARGET == TARGET_AMIGA
__asm void *__mp_alloc(register __d0 size_t l, register __d1 size_t a,
                       register __d2 alloctype f, register __a0 char *s,
                       register __a1 char *t, register __d3 unsigned long u,
                       register __d4 size_t k)
#else /* TARGET */
void *__mp_alloc(size_t l, size_t a, alloctype f, char *s, char *t,
                 unsigned long u, size_t k)
#endif /* TARGET */
{
    void *p;
    stackinfo i;
    int j;

    savesignals();
    if (!memhead.init)
        __mp_init();
    __mp_checkinfo(&memhead);
    if (memhead.prologue)
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
  retry:
    p = __mp_getmemory(&memhead, l, a, f, s, t, u, &i);
    if (memhead.epilogue)
        memhead.epilogue(p);
    /* Call the low-memory handler if no memory block was allocated.
     */
    if ((p == NULL) && memhead.nomemory)
    {
        memhead.nomemory();
        if ((f == AT_NEW) || (f == AT_NEWVEC))
        {
            if (memhead.prologue)
                memhead.prologue((void *) -1, l);
            goto retry;
        }
    }
    restoresignals();
    return p;
}


/* Allocate a new block of memory to duplicate a string.
 */

#if TARGET == TARGET_AMIGA
__asm char *__mp_strdup(register __a0 char *p, register __d0 size_t l,
                        register __d1 alloctype f, register __a1 char *s,
                        register __a2 char *t, register __d2 unsigned long u,
                        register __d3 size_t k)
#else /* TARGET */
char *__mp_strdup(char *p, size_t l, alloctype f, char *s, char *t,
                  unsigned long u, size_t k)
#endif /* TARGET */
{
    char *o;
    stackinfo i;
    size_t n;
    int j;

    savesignals();
    if (!memhead.init)
        __mp_init();
    __mp_checkinfo(&memhead);
    if (memhead.prologue)
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
            memcpy(p, o, n + 1);
    }
    if (memhead.epilogue)
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

#if TARGET == TARGET_AMIGA
__asm void *__mp_realloc(register __a0 void *p, register __d0 size_t l,
                         register __d1 size_t a, register __d2 alloctype f,
                         register __a1 char *s, register __a2 char *t,
                         register __d3 unsigned long u, register __d4 size_t k)
#else /* TARGET */
void *__mp_realloc(void *p, size_t l, size_t a, alloctype f, char *s, char *t,
                   unsigned long u, size_t k)
#endif /* TARGET */
{
    stackinfo i;
    int j;

    savesignals();
    if (!memhead.init)
        __mp_init();
    __mp_checkinfo(&memhead);
    if (memhead.prologue)
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
    p = __mp_resizememory(&memhead, p, l, a, f, s, t, u, &i);
    if (memhead.epilogue)
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

#if TARGET == TARGET_AMIGA
__asm void __mp_free(register __a0 void *p, register __d0 alloctype f,
                     register __a1 char *s, register __a2 char *t,
                     register __d1 unsigned long u, register __d2 size_t k)
#else /* TARGET */
void __mp_free(void *p, alloctype f, char *s, char *t, unsigned long u,
               size_t k)
#endif /* TARGET */
{
    stackinfo i;
    int j;

    if (memhead.fini)
        return;
    savesignals();
    if (!memhead.init)
        __mp_init();
    __mp_checkinfo(&memhead);
    if (memhead.prologue)
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
    __mp_freememory(&memhead, p, f, s, t, u, &i);
    if (memhead.epilogue)
        memhead.epilogue((void *) -1);
    restoresignals();
}


/* Obtain any details about the memory block that contains a given address.
 */

#if TARGET == TARGET_AMIGA
__asm int __mp_info(register __a0 void *p, register __a1 allocinfo *d)
#else /* TARGET */
int __mp_info(void *p, allocinfo *d)
#endif /* TARGET */
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

#if TARGET == TARGET_AMIGA
__asm void __mp_memorymap(register __d0 int s)
#else /* TARGET */
void __mp_memorymap(int s)
#endif /* TARGET */
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

#if TARGET == TARGET_AMIGA
__asm void __mp_summary(void)
#else /* TARGET */
void __mp_summary(void)
#endif /* TARGET */
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

#if TARGET == TARGET_AMIGA
__asm void __mp_check(void)
#else /* TARGET */
void __mp_check(void)
#endif /* TARGET */
{
    savesignals();
    if (!memhead.init)
        __mp_init();
    __mp_checkinfo(&memhead);
    restoresignals();
}


/* Set the prologue function and return the previous setting.
 */

#if TARGET == TARGET_AMIGA
__asm void (*__mp_prologue(register __a0 void (*h)(void *, size_t)))
           (void *, size_t)
#else /* TARGET */
void (*__mp_prologue(void (*h)(void *, size_t)))(void *, size_t)
#endif /* TARGET */
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

#if TARGET == TARGET_AMIGA
__asm void (*__mp_epilogue(register __a0 void (*h)(void *)))(void *)
#else /* TARGET */
void (*__mp_epilogue(void (*h)(void *)))(void *)
#endif /* TARGET */
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

#if TARGET == TARGET_AMIGA
__asm void (*__mp_nomemory(register __a0 void (*h)(void)))(void)
#else /* TARGET */
void (*__mp_nomemory(void (*h)(void)))(void)
#endif /* TARGET */
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


#ifdef __cplusplus
}
#endif /* __cplusplus */
