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
#if MP_MMAP_SUPPORT
#include <fcntl.h>
#endif /* MP_MMAP_SUPPORT */


#if MP_IDENT_SUPPORT
#ident "$Id: option.c,v 1.6 1999-12-21 20:14:30 graeme Exp $"
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
    "CHECK", "unsigned range",
    "", "Specifies a range of allocation indices at which to check the",
    "", "integrity of free memory and overflow buffers.",
    "CHECKALL", NULL,
    "", "Equivalent to the CHECKALLOCS, CHECKREALLOCS and CHECKFREES options",
    "", "specified together.",
    "CHECKALLOCS", NULL,
    "", "Checks that no attempt is made to allocate a block of memory of size",
    "", "zero.",
    "CHECKFREES", NULL,
    "", "Checks that no attempt is made to deallocate a NULL pointer.",
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
    "LIMIT", "unsigned integer",
    "", "Specifies the limit in bytes at which all memory allocations should",
    "", "fail if the total allocated memory should increase beyond this.",
    "LOGALL", NULL,
    "", "Equivalent to the LOGALLOCS, LOGREALLOCS and LOGFREES options",
    "", "specified together.",
    "LOGALLOCS", NULL,
    "", "Specifies that all memory allocations are to be logged and sent to",
    "", "the log file.",
    "LOGFILE", "string",
    "", "Specifies an alternative file in which to place all diagnostics from",
    "", "the mpatrol library.",
    "LOGFREES", NULL,
    "", "Specifies that all memory deallocations are to be logged and sent to",
    "", "the log file.",
    "LOGREALLOCS", NULL,
    "", "Specifies that all memory reallocations are to be logged and sent to",
    "", "the log file.",
    "NOFREE", NULL,
    "", "Specifies that the mpatrol library should keep all reallocated and",
    "", "freed memory allocations.",
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
    "PROGFILE", "string",
    "", "Specifies an alternative filename with which to locate the executable",
    "", "file containing the program's symbols.",
    "REALLOCSTOP", "unsigned integer",
    "", "Specifies an allocation index at which to stop the program when a",
    "", "memory allocation is being reallocated.",
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
    "UNFREEDABORT", "unsigned integer",
    "", "Specifies the minimum number of unfreed allocations at which to abort",
    "", "the program just before program termination.",
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
        __mp_error(AT_MAX, "ignoring negative number `%s'\n", s);
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
        __mp_warn(AT_MAX, "%s number overflow in `%s'\n", ((u == 0) &&
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
    if ((*s != '\0') && (s[readnumber(s, l, 1)] != '\0'))
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
        if ((*s != '\0') && (s[readnumber(s, u, 1)] != '\0'))
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
    char *a, *f, *o, *s;
    unsigned long m, n;
    int i, l, q;

    l = 0;
    f = NULL;
    if (((s = getenv(MP_OPTIONS)) == NULL) || (*s == '\0'))
        return;
    if (strlen(s) + 1 > sizeof(options))
    {
        __mp_error(AT_MAX, "%s: environment variable too long\n", MP_OPTIONS);
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
                                FLG_CHECKFREES;
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
                if (matchoption(o, "LIMIT"))
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
                    h->flags |= FLG_LOGALLOCS | FLG_LOGREALLOCS | FLG_LOGFREES;
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
                else if (matchoption(o, "LOGREALLOCS"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    h->flags |= FLG_LOGREALLOCS;
                }
                break;
              case 'N':
                if (matchoption(o, "NOFREE"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
                    h->alloc.flags |= FLG_NOFREE;
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
                if (matchoption(o, "SHOWALL"))
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
                else if (matchoption(o, "USEMMAP"))
                {
                    if (*a != '\0')
                        i = OE_IGNARGUMENT;
                    else
                        i = OE_RECOGNISED;
#if MP_MMAP_SUPPORT
                    if (h->alloc.list.size == 0)
                        h->alloc.heap.memory.mfile = open(MP_MMAP_FILENAME,
                                                          O_RDWR);
#endif /* MP_MMAP_SUPPORT */
                }
                break;
              default:
                break;
            }
            /* Now check the error code returned from attempting to match
             * the keyword and report if anything went wrong.
             */
            if (i == OE_UNRECOGNISED)
            {
                if (*a == '\0')
                    __mp_error(AT_MAX, "unrecognised option `%s'\n", o);
                else
                    __mp_error(AT_MAX, "unrecognised option `%s=%s'\n", o, a);
            }
            else if (i == OE_NOARGUMENT)
                __mp_error(AT_MAX, "missing argument for option `%s'\n", o);
            else if (i == OE_BADNUMBER)
                __mp_error(AT_MAX, "bad numeric argument `%s' for option "
                           "`%s'\n", a, o);
            else if (i == OE_BADRANGE)
                __mp_error(AT_MAX, "bad numeric range `%s' for option `%s'\n",
                           a, o);
            else if (i == OE_BIGNUMBER)
                __mp_error(AT_MAX, "numeric argument `%s' is too large for "
                           "option `%s'\n", a, o);
            else if (i == OE_LOWERORUPPER)
                __mp_error(AT_MAX, "must specify `LOWER' or `UPPER' for "
                           "option `%s'\n", o);
            else if (i == OE_IGNARGUMENT)
                __mp_warn(AT_MAX, "ignoring argument `%s' for option `%s'\n",
                          a, o);
        }
        else if (*a != '\0')
            __mp_warn(AT_MAX, "missing option for argument `%s'\n", a);
    }
    if (l != 0)
        showoptions();
    if (f != NULL)
        h->log = __mp_logfile(f);
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
