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
 * Option handling.  Run-time modifiable flags are set here by parsing
 * an option string read from an environment variable.  Any warnings
 * or errors that occurred during parsing will be reported.
 */


#include "option.h"
#include "diag.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#if MP_MMAP_SUPPORT && !MP_MMAP_ANONYMOUS
#include <fcntl.h>
#endif /* MP_MMAP_SUPPORT && MP_MMAP_ANONYMOUS */


#if MP_IDENT_SUPPORT
#ident "$Id: option.c,v 1.20 2000-11-11 15:51:05 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* The temporary buffer used to parse the configuration environment variable,
 * since if we change the variable directly then we change the environment.
 * This should not really be a file scope variable as it prevents this module
 * from being re-entrant.
 */

static char options[1024];


/* The table describing a summary of all recognised options.
 */

static char *options_help[] =
{
    "ALLOCBYTE", "unsigned integer",
    "", "Specifies an 8-bit byte pattern with which to prefill newly-allocated",
    "", "memory.",
    "ALLOCSTOP", "unsigned integer",
    "", "Specifies an allocation index at which to stop the program when it is",
    "", "being allocated.",
    "ALLOWOFLOW", NULL,
    "", "Specifies that a warning rather than an error should be produced if",
    "", "any memory operation function overflows the boundaries of a memory",
    "", "allocation, and that the operation should still be performed.",
    "AUTOSAVE", "unsigned integer",
    "", "Specifies the frequency at which to periodically write the profiling",
    "", "data to the profiling output file.",
    "CHECK", "unsigned range",
    "", "Specifies a range of allocation indices at which to check the",
    "", "integrity of free memory and overflow buffers.",
    "CHECKALL", NULL,
    "", "Equivalent to the CHECKALLOCS, CHECKREALLOCS, CHECKFREES and",
    "", "CHECKMEMORY options specified together.",
    "CHECKALLOCS", NULL,
    "", "Checks that no attempt is made to allocate a block of memory of size",
    "", "zero.",
    "CHECKFREES", NULL,
    "", "Checks that no attempt is made to deallocate a NULL pointer.",
    "CHECKMEMORY", NULL,
    "", "Checks that no attempt is made to perform a zero-length memory",
    "", "operation on a NULL pointer.",
    "CHECKREALLOCS", NULL,
    "", "Checks that no attempt is made to reallocate a NULL pointer or resize",
    "", "an existing block of memory to size zero.",
    "DEFALIGN", "unsigned integer",
    "", "Specifies the default alignment for general-purpose memory",
    "", "allocations, which must be a power of two.",
    "FAILFREQ", "unsigned integer",
    "", "Specifies the frequency at which all memory allocations will randomly",
    "", "fail.",
    "FAILSEED", "unsigned integer",
    "", "Specifies the random number seed which will be used when determining",
    "", "which memory allocations will randomly fail.",
    "FREEBYTE", "unsigned integer",
    "", "Specifies an 8-bit byte pattern with which to prefill newly-freed",
    "", "memory.",
    "FREESTOP", "unsigned integer",
    "", "Specifies an allocation index at which to stop the program when it is",
    "", "being freed.",
    "HELP", NULL,
    "", "Displays this quick-reference option summary.",
    "LARGEBOUND", "unsigned integer",
    "", "Specifies the limit in bytes up to which memory allocations should be",
    "", "classified as large allocations for profiling purposes.",
    "LIMIT", "unsigned integer",
    "", "Specifies the limit in bytes at which all memory allocations should",
    "", "fail if the total allocated memory should increase beyond this.",
    "LOGALL", NULL,
    "", "Equivalent to the LOGALLOCS, LOGREALLOCS, LOGFREES and LOGMEMORY",
    "", "options specified together.",
    "LOGALLOCS", NULL,
    "", "Specifies that all memory allocations are to be logged and sent to",
    "", "the log file.",
    "LOGFILE", "string",
    "", "Specifies an alternative file in which to place all diagnostics from",
    "", "the mpatrol library.",
    "LOGFREES", NULL,
    "", "Specifies that all memory deallocations are to be logged and sent to",
    "", "the log file.",
    "LOGMEMORY", NULL,
    "", "Specifies that all memory operations are to be logged and sent to the",
    "", "log file.",
    "LOGREALLOCS", NULL,
    "", "Specifies that all memory reallocations are to be logged and sent to",
    "", "the log file.",
    "MEDIUMBOUND", "unsigned integer",
    "", "Specifies the limit in bytes up to which memory allocations should be",
    "", "classified as medium allocations for profiling purposes.",
    "NOFREE", "unsigned integer",
    "", "Specifies that a number of recently-freed memory allocations should",
    "", "be prevented from being returned to the free memory pool.",
    "NOPROTECT", NULL,
    "", "Specifies that the mpatrol library's internal data structures should",
    "", "not be made read-only after every memory allocation, reallocation or",
    "", "deallocation.",
    "OFLOWBYTE", "unsigned integer",
    "", "Specifies an 8-bit byte pattern with which to fill the overflow",
    "", "buffers of all memory allocations.",
    "OFLOWSIZE", "unsigned integer",
    "", "Specifies the size in bytes to use for all overflow buffers, which",
    "", "must be a power of two.",
    "OFLOWWATCH", NULL,
    "", "Specifies that watch point areas should be used for overflow buffers",
    "", "rather than filling with the overflow byte.",
    "PAGEALLOC", "LOWER|UPPER",
    "", "Specifies that each individual memory allocation should occupy at",
    "", "least one page of virtual memory and should be placed at the lowest",
    "", "or highest point within these pages.",
    "PRESERVE", NULL,
    "", "Specifies that any reallocated or freed memory allocations should",
    "", "preserve their original contents.",
    "PROF", NULL,
    "", "Specifies that all memory allocations are to be profiled and sent to",
    "", "the profiling output file.",
    "PROFFILE", "string",
    "", "Specifies an alternative file in which to place all memory allocation",
    "", "profiling information from the mpatrol library.",
    "PROGFILE", "string",
    "", "Specifies an alternative filename with which to locate the executable",
    "", "file containing the program's symbols.",
    "REALLOCSTOP", "unsigned integer",
    "", "Specifies an allocation index at which to stop the program when a",
    "", "memory allocation is being reallocated.",
    "SAFESIGNALS", NULL,
    "", "Instructs the library to save and replace certain signal handlers",
    "", "during the execution of library code and to restore them afterwards.",
    "SHOWALL", NULL,
    "", "Equivalent to the SHOWFREED, SHOWUNFREED, SHOWMAP and SHOWSYMBOLS",
    "", "options specified together.",
    "SHOWFREED", NULL,
    "", "Specifies that a summary of all of the freed memory allocations",
    "", "should be displayed at the end of program execution.",
    "SHOWMAP", NULL,
    "", "Specifies that a memory map of the entire heap should be displayed at",
    "", "the end of program execution.",
    "SHOWSYMBOLS", NULL,
    "", "Specifies that a summary of all of the function symbols read from the",
    "", "program's executable file should be displayed at the end of program",
    "", "execution.",
    "SHOWUNFREED", NULL,
    "", "Specifies that a summary of all of the unfreed memory allocations",
    "", "should be displayed at the end of program execution.",
    "SMALLBOUND", "unsigned integer",
    "", "Specifies the limit in bytes up to which memory allocations should be",
    "", "classified as small allocations for profiling purposes.",
    "UNFREEDABORT", "unsigned integer",
    "", "Specifies the minimum number of unfreed allocations at which to abort",
    "", "the program just before program termination.",
    "USEDEBUG", NULL,
    "", "Specifies that any debugging information in the executable file",
    "", "should be used to obtain additional source-level information.",
    "USEMMAP", NULL,
    "", "Specifies that the library should use mmap() instead of sbrk() to",
    "", "allocate system memory on UNIX platforms.",
    NULL
};


/* Perform a case-insensitive comparison between two option keywords.
 */

static int matchoption(char *s, char *t)
{
#if TARGET == TARGET_UNIX
    int d;
#endif /* TARGET */

#if TARGET == TARGET_UNIX
    while (((d = toupper(*s) - toupper(*t)) == 0) && (*s != '\0'))
    {
        s++;
        t++;
    }
    return (d == 0);
#elif TARGET == TARGET_AMIGA || TARGET == TARGET_WINDOWS || \
      TARGET == TARGET_NETWARE
    return !stricmp(s, t);
#endif /* TARGET */
}


/* Convert a string representation of a number to an integer,
 * reporting any errors that occur during the conversion.
 */

static size_t readnumber(char *s, long *n, int u)
{
    char *t;
    int e;

    e = errno;
    errno = 0;
    while (isspace(*s))
        s++;
    if ((u == 1) && (*s == '-'))
    {
        __mp_error(ET_MAX, AT_MAX, "ignoring negative number `%s'\n", s);
        t = s;
    }
    else if ((u == 0) && (*s == '-') && (s[1] == '0') && ((s[2] == 'b') ||
             (s[2] == 'B')))
        /* This is a negative binary number.
         */
        *n = -strtol(s + 3, &t, 2);
    else if ((*s == '0') && ((s[1] == 'b') || (s[1] == 'B')))
    {
        /* This is a positive binary number.
         */
        if (u == 0)
            *n = strtol(s + 2, &t, 2);
        else
            *n = strtoul(s + 2, &t, 2);
    }
    /* Otherwise let the conversion function work out the number base
     * from the prefix.
     */
    else if (u == 0)
        *n = strtol(s, &t, 0);
    else
        *n = strtoul(s, &t, 0);
    if (errno == ERANGE)
        __mp_warn(ET_MAX, AT_MAX, "%s number overflow in `%s'\n", ((u == 0) &&
                   (*n == LONG_MIN)) ? "negative" : "positive", s);
    errno = e;
    return (size_t) (t - s);
}


/* Convert a string representation of a numeric range to two unsigned integers,
 * ensuring that the first integer is less than or equal to the second.  An
 * open range at either end is represented by -1.
 */

static int readrange(char *s, unsigned long *l, unsigned long *u)
{
    char *t;
    unsigned long n;
    int w;
    char c;

    w = 0;
    *l = *u = (unsigned long) -1;
    for (t = s; (*t != '-') && (*t != '\0'); t++);
    c = *t;
    *t = '\0';
    /* If there was a number before the minus sign then read it.
     */
    if ((*s != '\0') && (s[readnumber(s, (long *) l, 1)] != '\0'))
    {
        *l = (unsigned long) -1;
        w = 1;
    }
    else if (c == '\0')
        *u = *l;
    else
    {
        s = t + 1;
        /* If there was a number after the minus sign then read it too.
         */
        if ((*s != '\0') && (s[readnumber(s, (long *) u, 1)] != '\0'))
        {
            *u = (unsigned long) -1;
            w = 1;
        }
    }
    if (w != 0)
        return 0;
    /* If one or the other of the integers was zero (but not both) then convert
     * it to an open range.
     */
    if ((*l == 0) && (*u != 0))
        *l = (unsigned long) -1;
    else if ((*l != 0) && (*u == 0))
        *u = (unsigned long) -1;
    /* Swap the integers if the first number is greater than the second.
     */
    if ((*l != (unsigned long) -1) && (*u != (unsigned long) -1) && (*l > *u))
    {
        n = *l;
        *l = *u;
        *u = n;
    }
    return 1;
}


/* Display the quick-reference help summary.
 */

static void showoptions(void)
{
    char **s, **t;

    __mp_diag("Available options:\n\n");
    for (s = options_help, t = s + 1; *s != NULL; s += 2, t += 2)
        if (**s != '\0')
        {
            __mp_diag("    %s", *s);
            if (*t != NULL)
                __mp_diag("=<%s>", *t);
            __mp_diag("\n");
        }
        else
            __mp_diag("\t%s\n", *t);
    __mp_diag("\n");
}


/* The main option parsing routine.
 */

MP_GLOBAL void __mp_parseoptions(infohead *h)
{
    char *a, *f, *o, *p, *s;
    unsigned long m, n;
    int i, l, q;

    l = 0;
    f = p = NULL;
    if (((s = getenv(MP_OPTIONS)) == NULL) || (*s == '\0'))
        return;
    if (strlen(s) + 1 > sizeof(options))
    {
        __mp_error(ET_MAX, AT_MAX, "%s: environment variable too long\n",
                   MP_OPTIONS);
        return;
    }
    /* We shouldn't modify the original string returned by getenv() since
     * that would modify the environment, and it may be placed in read-only
     * memory anyway.
     */
    strcpy(options, s);
    s = options;
    while (*s != '\0')
    {
        i = 0;
        while (isspace(*s))
            s++;
        if (*s == '\0')
            break;
        if (*s != '=')
        {
            /* Scan the option keyword.
             */
            for (o = s, q = 0; ((q == 1) || !isspace(*s)) && (*s != '\0') &&
                 (*s != '='); s++)
                if (*s == '"')
                {
                    /* Remove any quotes from the keyword.
                     */
                    __mp_memcopy(s, s + 1, strlen(s));
                    q = (q == 0) ? 1 : 0;
                    s--;
                }
            if ((*s != '\0') && (*s != '='))
            {
                *s++ = '\0';
                i = 1;
            }
        }
        else
            o = "";
        if ((*s == '=') && (i == 0))
        {
            /* Scan the option value.
             */
            *s++ = '\0';
            for (a = s, q = 0; ((q == 1) || !isspace(*s)) && (*s != '\0'); s++)
                if (*s == '"')
                {
                    /* Remove any quotes from the value.
                     */
                    __mp_memcopy(s, s + 1, strlen(s));
                    q = (q == 0) ? 1 : 0;
                    s--;
                }
            if (*s != '\0')
                *s++ = '\0';
        }
        else
            a = "";
        if (*o != '\0')
        {
            /* We now have an option keyword and possibly an associated
             * value, so we can now check for valid keywords.
             */
            i = OE_UNRECOGNISED;
            switch (toupper(*o))
            {
              case 'A':
                if (matchoption(o, "ALLOCBYTE"))
                    if (*a == '\0')
                        i = OE_NOARGUMENT;
                    else if (a[readnumber(a, (long *) &n, 1)] != '\0')
                        i = OE_BADNUMBER;
                    else if (n > 0xFF)
                        i = OE_BIGNUMBER;
                    else
                    {
                        h->alloc.abyte = n;
                        i = OE_RECOGNISED;
                    }
                else if (matchoption(o, "ALLOCSTOP"))
                    if (*a == '\0')
                        i = OE_NOARGUMENT;
                    else if (a[readnumber(a, (long *) &n, 1)] != '\0')
                        i = OE_BADNUMBER;
                    else
                    {
                        h->astop = n;
                        i = OE_RECOGNISED;
                    }
                else if (matchoption(o, "ALLOWOFLOW"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    h->flags |= FLG_ALLOWOFLOW;
                }
                else if (matchoption(o, "AUTOSAVE"))
                    if (*a == '\0')
                        i = OE_NOARGUMENT;
                    else if (a[readnumber(a, (long *) &n, 1)] != '\0')
                        i = OE_BADNUMBER;
                    else
                    {
                        h->prof.autosave = n;
                        i = OE_RECOGNISED;
                    }
                break;
              case 'C':
                if (matchoption(o, "CHECK"))
                    if (*a == '\0')
                        i = OE_NOARGUMENT;
                    else if (!readrange(a, &m, &n))
                        i = OE_BADRANGE;
                    else
                    {
                        h->lrange = m;
                        h->urange = n;
                        i = OE_RECOGNISED;
                    }
                else if (matchoption(o, "CHECKALL"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    h->flags |= FLG_CHECKALLOCS | FLG_CHECKREALLOCS |
                                FLG_CHECKFREES | FLG_CHECKMEMORY;
                }
                else if (matchoption(o, "CHECKALLOCS"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    h->flags |= FLG_CHECKALLOCS;
                }
                else if (matchoption(o, "CHECKFREES"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    h->flags |= FLG_CHECKFREES;
                }
                else if (matchoption(o, "CHECKMEMORY"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    h->flags |= FLG_CHECKMEMORY;
                }
                else if (matchoption(o, "CHECKREALLOCS"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    h->flags |= FLG_CHECKREALLOCS;
                }
                break;
              case 'D':
                if (matchoption(o, "DEFALIGN"))
                    if (*a == '\0')
                        i = OE_NOARGUMENT;
                    else if ((a[readnumber(a, (long *) &n, 1)] != '\0') ||
                             (n == 0))
                        i = OE_BADNUMBER;
                    else if (n > h->alloc.heap.memory.page)
                        i = OE_BIGNUMBER;
                    else
                    {
                        h->alloc.heap.memory.align = __mp_poweroftwo(n);
                        i = OE_RECOGNISED;
                    }
                break;
              case 'F':
                if (matchoption(o, "FAILFREQ"))
                    if (*a == '\0')
                        i = OE_NOARGUMENT;
                    else if (a[readnumber(a, (long *) &n, 1)] != '\0')
                        i = OE_BADNUMBER;
                    else
                    {
                        h->ffreq = n;
                        i = OE_RECOGNISED;
                    }
                else if (matchoption(o, "FAILSEED"))
                    if (*a == '\0')
                        i = OE_NOARGUMENT;
                    else if (a[readnumber(a, (long *) &n, 1)] != '\0')
                        i = OE_BADNUMBER;
                    else
                    {
                        h->fseed = n;
                        i = OE_RECOGNISED;
                    }
                else if (matchoption(o, "FREEBYTE"))
                    if (*a == '\0')
                        i = OE_NOARGUMENT;
                    else if (a[readnumber(a, (long *) &n, 1)] != '\0')
                        i = OE_BADNUMBER;
                    else if (n > 0xFF)
                        i = OE_BIGNUMBER;
                    else
                    {
                        h->alloc.fbyte = n;
                        i = OE_RECOGNISED;
                    }
                else if (matchoption(o, "FREESTOP"))
                    if (*a == '\0')
                        i = OE_NOARGUMENT;
                    else if (a[readnumber(a, (long *) &n, 1)] != '\0')
                        i = OE_BADNUMBER;
                    else
                    {
                        h->fstop = n;
                        i = OE_RECOGNISED;
                    }
                break;
              case 'H':
                if (matchoption(o, "HELP"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    l = 1;
                }
                break;
              case 'L':
                if (matchoption(o, "LARGEBOUND"))
                    if (*a == '\0')
                        i = OE_NOARGUMENT;
                    else if (a[readnumber(a, (long *) &n, 1)] != '\0')
                        i = OE_BADNUMBER;
                    else
                    {
                        if (n == 0)
                            h->prof.lbound = MP_LARGEBOUND;
                        else
                            h->prof.lbound = n;
                        i = OE_RECOGNISED;
                    }
                else if (matchoption(o, "LIMIT"))
                    if (*a == '\0')
                        i = OE_NOARGUMENT;
                    else if (a[readnumber(a, (long *) &n, 1)] != '\0')
                        i = OE_BADNUMBER;
                    else
                    {
                        h->limit = n;
                        i = OE_RECOGNISED;
                    }
                else if (matchoption(o, "LOGALL"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    h->flags |= FLG_LOGALLOCS | FLG_LOGREALLOCS | FLG_LOGFREES |
                                FLG_LOGMEMORY;
                }
                else if (matchoption(o, "LOGALLOCS"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    h->flags |= FLG_LOGALLOCS;
                }
                else if (matchoption(o, "LOGFILE"))
                    if (*a == '\0')
                        i = OE_NOARGUMENT;
                    else
                    {
                        f = a;
                        i = OE_RECOGNISED;
                    }
                else if (matchoption(o, "LOGFREES"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    h->flags |= FLG_LOGFREES;
                }
                else if (matchoption(o, "LOGMEMORY"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    h->flags |= FLG_LOGMEMORY;
                }
                else if (matchoption(o, "LOGREALLOCS"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    h->flags |= FLG_LOGREALLOCS;
                }
                break;
              case 'M':
                if (matchoption(o, "MEDIUMBOUND"))
                    if (*a == '\0')
                        i = OE_NOARGUMENT;
                    else if (a[readnumber(a, (long *) &n, 1)] != '\0')
                        i = OE_BADNUMBER;
                    else
                    {
                        if (n == 0)
                            h->prof.mbound = MP_MEDIUMBOUND;
                        else
                            h->prof.mbound = n;
                        i = OE_RECOGNISED;
                    }
                break;
              case 'N':
                if (matchoption(o, "NOFREE"))
                    if (*a == '\0')
                        i = OE_NOARGUMENT;
                    else if (a[readnumber(a, (long *) &n, 1)] != '\0')
                        i = OE_BADNUMBER;
                    else
                    {
                        h->alloc.fmax = n;
                        h->alloc.flags |= FLG_NOFREE;
                        i = OE_RECOGNISED;
                    }
                else if (matchoption(o, "NOPROTECT"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    h->flags |= FLG_NOPROTECT;
                }
                break;
              case 'O':
                if (matchoption(o, "OFLOWBYTE"))
                    if (*a == '\0')
                        i = OE_NOARGUMENT;
                    else if (a[readnumber(a, (long *) &n, 1)] != '\0')
                        i = OE_BADNUMBER;
                    else if (n > 0xFF)
                        i = OE_BIGNUMBER;
                    else
                    {
                        h->alloc.obyte = n;
                        i = OE_RECOGNISED;
                    }
                else if (matchoption(o, "OFLOWSIZE"))
                    if (*a == '\0')
                        i = OE_NOARGUMENT;
                    else if ((a[readnumber(a, (long *) &n, 1)] != '\0') ||
                             ((n == 0) && (h->alloc.flags & FLG_PAGEALLOC)))
                        i = OE_BADNUMBER;
                    else
                    {
                        h->alloc.oflow = __mp_poweroftwo(n);
                        if (h->alloc.flags & FLG_PAGEALLOC)
                            h->alloc.oflow = __mp_roundup(h->alloc.oflow,
                                             h->alloc.heap.memory.page);
                        i = OE_RECOGNISED;
                    }
                else if (matchoption(o, "OFLOWWATCH"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
#if MP_WATCH_SUPPORT
                    h->alloc.flags |= FLG_OFLOWWATCH;
#endif /* MP_WATCH_SUPPORT */
                }
                break;
              case 'P':
                if (matchoption(o, "PAGEALLOC"))
                    if (*a == '\0')
                        i = OE_NOARGUMENT;
                    else if (matchoption(a, "LOWER"))
                    {
#if MP_PROTECT_SUPPORT
                        h->alloc.flags |= FLG_PAGEALLOC;
                        if (h->alloc.oflow == 0)
                            h->alloc.oflow = 1;
                        h->alloc.oflow = __mp_roundup(h->alloc.oflow,
                                         h->alloc.heap.memory.page);
#endif /* MP_PROTECT_SUPPORT */
                        i = OE_RECOGNISED;
                    }
                    else if (matchoption(a, "UPPER"))
                    {
#if MP_PROTECT_SUPPORT
                        h->alloc.flags |= FLG_PAGEALLOC | FLG_ALLOCUPPER;
                        if (h->alloc.oflow == 0)
                            h->alloc.oflow = 1;
                        h->alloc.oflow = __mp_roundup(h->alloc.oflow,
                                         h->alloc.heap.memory.page);
#endif /* MP_PROTECT_SUPPORT */
                        i = OE_RECOGNISED;
                    }
                    else
                        i = OE_LOWERORUPPER;
                else if (matchoption(o, "PRESERVE"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    h->alloc.flags |= FLG_PRESERVE;
                }
                else if (matchoption(o, "PROF"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    h->prof.profiling = 1;
                }
                else if (matchoption(o, "PROFFILE"))
                    if (*a == '\0')
                        i = OE_NOARGUMENT;
                    else
                    {
                        p = a;
                        i = OE_RECOGNISED;
                    }
                else if (matchoption(o, "PROGFILE"))
                    if (*a == '\0')
                        i = OE_NOARGUMENT;
                    else
                    {
                        h->alloc.heap.memory.prog = a;
                        i = OE_RECOGNISED;
                    }
                break;
              case 'R':
                if (matchoption(o, "REALLOCSTOP"))
                    if (*a == '\0')
                        i = OE_NOARGUMENT;
                    else if (a[readnumber(a, (long *) &n, 1)] != '\0')
                        i = OE_BADNUMBER;
                    else
                    {
                        h->rstop = n;
                        i = OE_RECOGNISED;
                    }
                break;
              case 'S':
                if (matchoption(o, "SAFESIGNALS"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    h->flags |= FLG_SAFESIGNALS;
                }
                else if (matchoption(o, "SHOWALL"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    h->flags |= FLG_SHOWFREED | FLG_SHOWUNFREED |
                                FLG_SHOWMAP | FLG_SHOWSYMBOLS;
                }
                else if (matchoption(o, "SHOWFREED"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    h->flags |= FLG_SHOWFREED;
                }
                else if (matchoption(o, "SHOWMAP"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    h->flags |= FLG_SHOWMAP;
                }
                else if (matchoption(o, "SHOWSYMBOLS"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    h->flags |= FLG_SHOWSYMBOLS;
                }
                else if (matchoption(o, "SHOWUNFREED"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    h->flags |= FLG_SHOWUNFREED;
                }
                else if (matchoption(o, "SMALLBOUND"))
                    if (*a == '\0')
                        i = OE_NOARGUMENT;
                    else if (a[readnumber(a, (long *) &n, 1)] != '\0')
                        i = OE_BADNUMBER;
                    else
                    {
                        if (n == 0)
                            h->prof.sbound = MP_SMALLBOUND;
                        else
                            h->prof.sbound = n;
                        i = OE_RECOGNISED;
                    }
                break;
              case 'U':
                if (matchoption(o, "UNFREEDABORT"))
                    if (*a == '\0')
                        i = OE_NOARGUMENT;
                    else if (a[readnumber(a, (long *) &n, 1)] != '\0')
                        i = OE_BADNUMBER;
                    else
                    {
                        h->uabort = n;
                        i = OE_RECOGNISED;
                    }
                else if (matchoption(o, "USEDEBUG"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    h->syms.lineinfo = 1;
                }
                else if (matchoption(o, "USEMMAP"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
#if MP_MMAP_SUPPORT
                    if (h->alloc.list.size == 0)
#if MP_MMAP_ANONYMOUS
                        h->alloc.heap.memory.mfile = 0;
#else /* MP_MMAP_ANONYMOUS */
                        h->alloc.heap.memory.mfile = open(MP_MMAP_FILENAME,
                                                          O_RDWR);
#endif /* MP_MMAP_ANONYMOUS */
#endif /* MP_MMAP_SUPPORT */
                }
                break;
              default:
                break;
            }
            /* Now check the error code returned from attempting to match
             * the keyword and report if anything went wrong.
             */
            switch (i)
            {
              case OE_UNRECOGNISED:
                if (*a == '\0')
                    __mp_error(ET_MAX, AT_MAX, "unrecognised option `%s'\n", o);
                else
                    __mp_error(ET_MAX, AT_MAX, "unrecognised option `%s=%s'\n",
                               o, a);
                break;
              case OE_NOARGUMENT:
                __mp_error(ET_MAX, AT_MAX, "missing argument for option `%s'\n",
                           o);
                break;
              case OE_BADNUMBER:
                __mp_error(ET_MAX, AT_MAX, "bad numeric argument `%s' for "
                           "option `%s'\n", a, o);
                break;
              case OE_BADRANGE:
                __mp_error(ET_MAX, AT_MAX, "bad numeric range `%s' for option "
                           "`%s'\n", a, o);
                break;
              case OE_BIGNUMBER:
                __mp_error(ET_MAX, AT_MAX, "numeric argument `%s' is too large "
                           "for option `%s'\n", a, o);
                break;
              case OE_LOWERORUPPER:
                __mp_error(ET_MAX, AT_MAX, "must specify `LOWER' or `UPPER' "
                           "for option `%s'\n", o);
                break;
              case OE_IGNARGUMENT:
                __mp_warn(ET_MAX, AT_MAX, "ignoring argument `%s' for option "
                          "`%s'\n", a, o);
                break;
              default:
                break;
            }
        }
        else if (*a != '\0')
            __mp_warn(ET_MAX, AT_MAX, "missing option for argument `%s'\n", a);
    }
    /* Check the validity of the profiling allocation boundaries.  There is
     * potential for error if either of the small or large bounds overlap the
     * medium bound and the medium bound is either 1 or the maximum sized
     * integer, but it will just result in wrong profiling and nothing more.
     */
    if (h->prof.sbound >= h->prof.mbound)
    {
        __mp_error(ET_MAX, AT_MAX, "small allocation boundary `%lu' overlaps "
                   "medium allocation boundary `%lu'\n", h->prof.sbound,
                   h->prof.mbound);
        h->prof.sbound = h->prof.mbound - 1;
    }
    if (h->prof.lbound <= h->prof.mbound)
    {
        __mp_error(ET_MAX, AT_MAX, "large allocation boundary `%lu' overlaps "
                   "medium allocation boundary `%lu'\n", h->prof.lbound,
                   h->prof.mbound);
        h->prof.lbound = h->prof.mbound + 1;
    }
    if (l != 0)
        showoptions();
    if (f != NULL)
        h->log = __mp_logfile(&h->alloc.heap.memory, f);
    if (p != NULL)
        h->prof.file = __mp_proffile(&h->alloc.heap.memory, p);
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
