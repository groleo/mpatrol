/*
 * mpatrol
 * A library for controlling and tracing dynamic memory allocations.
 * Copyright (C) 1997-2001 Graeme S. Roy <graeme@epc.co.uk>
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
 * Dmalloc-compatible interface.  Implements Dmalloc functions using
 * mpatrol.  Dmalloc is copyright (C) 1992-2001 Gray Watson.
 */


#include "config.h"
#include "dmalloc.h"
#include <stdio.h>
#include <time.h>


#if MP_IDENT_SUPPORT
#ident "$Id: dmalloc.c,v 1.1 2001-03-01 00:51:28 graeme Exp $"
#else /* MP_IDENT_SUPPORT */
static MP_CONST MP_VOLATILE char *dmalloc_id = "$Id: dmalloc.c,v 1.1 2001-03-01 00:51:28 graeme Exp $";
#endif /* MP_IDENT_SUPPORT */


/* Specify whether to prefix every log message produced by dmalloc_message()
 * and dmalloc_vmessage() with the current time in numerical form and/or
 * string form.  The current event number can also be logged as well.
 */

#define LOG_TIME_NUMBER     1
#define LOG_CTIME_STRING    0
#define LOG_ITERATION_COUNT 1


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* Indicates if this module has been initialised.
 */

static int malloc_initialised;


/* The library debug flags.
 */

static unsigned long malloc_flags;


/* The global variables which control the behaviour of the library and are
 * part of the Dmalloc library interface.  The last two are intended for use
 * within a debugger.
 */

char *dmalloc_logpath;
int dmalloc_errno;
void *dmalloc_address;
unsigned long dmalloc_address_count;


/* Map the library debug flags to mpatrol options.
 */

static
void
setoptions(void)
{
    unsigned long v;

    v = MP_FLG_SHOWFREE;
    if (malloc_flags & DMALLOC_LOG_STATS)
        __mp_setoption(MP_OPT_SETFLAGS, v);
    else
        __mp_setoption(MP_OPT_UNSETFLAGS, v);
    v = MP_FLG_SHOWUNFREED;
    if (malloc_flags & DMALLOC_LOG_NONFREE)
        __mp_setoption(MP_OPT_SETFLAGS, v);
    else
        __mp_setoption(MP_OPT_UNSETFLAGS, v);
    v = MP_FLG_LOGALLOCS | MP_FLG_LOGREALLOCS | MP_FLG_LOGFREES;
    if (malloc_flags & DMALLOC_LOG_TRANS)
        __mp_setoption(MP_OPT_SETFLAGS, v);
    else
        __mp_setoption(MP_OPT_UNSETFLAGS, v);
    if (malloc_flags & DMALLOC_CHECK_FENCE)
    {
        if (!__mp_getoption(MP_OPT_OFLOWSIZE, &v) || (v == 0))
            v = sizeof(void *);
    }
    else
        v = 0;
    __mp_setoption(MP_OPT_OFLOWSIZE, v);
    if (malloc_flags & DMALLOC_CHECK_HEAP)
        v = (unsigned long) -1;
    else
        v = 0;
    __mp_setoption(MP_OPT_CHECKLOWER, v);
    __mp_setoption(MP_OPT_CHECKUPPER, v);
    __mp_setoption(MP_OPT_CHECKFREQ, 1);
    v = MP_FLG_SAFESIGNALS;
    if (malloc_flags & DMALLOC_CATCH_SIGNALS)
        __mp_setoption(MP_OPT_SETFLAGS, v);
    else
        __mp_setoption(MP_OPT_UNSETFLAGS, v);
    v = MP_FLG_PRESERVE;
    if (malloc_flags & DMALLOC_FREE_BLANK)
        __mp_setoption(MP_OPT_UNSETFLAGS, v);
    else
        __mp_setoption(MP_OPT_SETFLAGS, v);
    v = MP_FLG_SHOWMAP;
    if (malloc_flags & DMALLOC_HEAP_CHECK_MAP)
        __mp_setoption(MP_OPT_SETFLAGS, v);
    else
        __mp_setoption(MP_OPT_UNSETFLAGS, v);
    if (malloc_flags & DMALLOC_NEVER_REUSE)
        v = ~0L;
    else
        v = 0;
    __mp_setoption(MP_OPT_NOFREE, v);
    v = MP_FLG_CHECKFREES;
    if (malloc_flags & DMALLOC_ALLOW_FREE_NULL)
        __mp_setoption(MP_OPT_UNSETFLAGS, v);
    else
        __mp_setoption(MP_OPT_SETFLAGS, v);
}


/* Terminate the dmalloc module.
 */

void
__mpt_dmallocshutdown(void)
{
    if (malloc_initialised)
    {
        malloc_flags = 0;
        malloc_initialised = 0;
    }
}


/* Log the details of any unfreed memory allocations.
 */

void
__mpt_dmalloclogunfreed(void)
{
    if (!malloc_initialised)
        __mp_init_dmalloc();
}


/* Check the integrity of the memory allocation containing a given pointer,
 * or the entire heap if the pointer is NULL.
 */

int
__mpt_dmallocverify(MP_CONST void *p, MP_CONST char *s, MP_CONST char *t,
                    unsigned long u)
{
    __mp_allocinfo i;
    int r;

    if (!malloc_initialised)
        __mp_init_dmalloc();
    if ((p == NULL) || (__mp_info(p, &i) && (p == i.block) && !i.freed))
        r = 1;
    else
        r = 0;
    __mp_checkheap(s, t, u);
    return r;
}


/* Set the library debug flags and return the previous setting.
 */

unsigned long
__mpt_dmallocdebug(unsigned long f)
{
    unsigned long r;

    if (!malloc_initialised)
        __mp_init_dmalloc();
    r = malloc_flags;
    malloc_flags = f;
    setoptions();
    return r;
}


/* Return the current library debug flags.
 */

unsigned long
__mpt_dmallocdebugcurrent(void)
{
    if (!malloc_initialised)
        __mp_init_dmalloc();
    return malloc_flags;
}


/* Examine a pointer in the heap and return information about the memory
 * allocation it belongs to.
 */

int
__mpt_dmallocexamine(MP_CONST void *p, size_t *l, char **t, unsigned long *u,
                     void **a)
{
    __mp_allocinfo i;

    if (!malloc_initialised)
        __mp_init_dmalloc();
    if (__mp_info(p, &i))
    {
        if (l != NULL)
            *l = i.size;
        if (t != NULL)
            *t = i.file;
        if (u != NULL)
            *u = i.line;
        if (a != NULL)
            if (i.stack != NULL)
                *a = i.stack->addr;
            else
                *a = NULL;
        return 1;
    }
    return 0;
}


/* Write a message to the log file.
 */

void
__mpt_dmallocmessage(MP_CONST char *s, ...)
{
    va_list v;

    if (!malloc_initialised)
        __mp_init_dmalloc();
    va_start(v, s);
    __mpt_dmallocvmessage(s, v);
    va_end(v);
}


/* Write a message to the log file.
 */

void
__mpt_dmallocvmessage(MP_CONST char *s, va_list v)
{
    char b[1024];
    char m[64];
    char *c, *p;
#if LOG_TIME_NUMBER || LOG_CTIME_STRING
    time_t t;
#endif /* LOG_TIME_NUMBER && LOG_CTIME_STRING */
    size_t l;

    if (!malloc_initialised)
        __mp_init_dmalloc();
    l = 0;
#if LOG_TIME_NUMBER || LOG_CTIME_STRING
    if ((t = time(NULL)) == (time_t) -1)
        t = (time_t) 0;
#endif /* LOG_TIME_NUMBER && LOG_CTIME_STRING */
#if LOG_TIME_NUMBER
    sprintf(m + l, "%lu: ", (unsigned long) t);
    l += strlen(m + l);
#endif /* LOG_TIME_NUMBER */
#if LOG_CTIME_STRING
    sprintf(m + l, "%24.24s: ", ctime(&t));
    l += strlen(m + l);
#endif /* LOG_CTIME_STRING */
#if LOG_ITERATION_COUNT
    sprintf(m + l, "%lu: ", __mp_snapshot());
    l += strlen(m + l);
#endif /* LOG_ITERATION_COUNT */
    m[l] = '\0';
    vsprintf(b, s, v);
    for (c = b; p = strchr(c, '\n'); c = p + 1)
    {
        *p = '\0';
        if (*c != '\0')
        {
            __mp_printf("%s%s\n", m, c);
            if (malloc_flags & DMALLOC_PRINT_MESSAGES)
                fprintf(stderr, "%s%s\n", m, c);
        }
        else
        {
            __mp_printf("%s\n", m);
            if (malloc_flags & DMALLOC_PRINT_MESSAGES)
                fprintf(stderr, "%s\n", m);
        }
    }
    if (*c != '\0')
    {
        __mp_printf("%s%s\n", m, c);
        if (malloc_flags & DMALLOC_PRINT_MESSAGES)
            fprintf(stderr, "%s%s\n", m, c);
    }
}


/* Register a callback function that will be called for each memory
 * allocation event.
 */

void
__mpt_dmalloctrack(dmalloc_track_t h)
{
    if (!malloc_initialised)
        __mp_init_dmalloc();
}


/* Log the details of any changes to the heap since a certain point.
 */

void
__mpt_dmalloclogchanged(unsigned long m, int u, int f, int d)
{
    if (!malloc_initialised)
        __mp_init_dmalloc();
}


/* Initialise the dmalloc module.
 */

void
__mp_init_dmalloc(void)
{
    if (!malloc_initialised)
    {
        malloc_initialised = 1;
        malloc_flags = 0;
        __mp_atexit(__mpt_dmallocshutdown);
    }
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
