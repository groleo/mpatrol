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
 * Memory allocation tracing.  If tracing is enabled then all memory
 * allocations and deallocations are written to a file in a compact
 * format for later processing by a tracing tool.
 */


#include "trace.h"
#include "diag.h"
#include <stdio.h>
#include <string.h>


#if MP_IDENT_SUPPORT
#ident "$Id: trace.c,v 1.2 2000-11-30 20:38:04 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* The file pointer to the tracing output file.  This should not really be
 * a file scope variable as it prevents this module from being re-entrant.
 */

static FILE *tracefile;


/* Initialise the fields of a tracehead so that the mpatrol library
 * is ready to trace memory allocations.
 */

MP_GLOBAL void __mp_newtrace(tracehead *t, meminfo *m)
{
    t->file = __mp_tracefile(m, NULL);
    t->tracing = 0;
    tracefile = NULL;
}


/* Finish tracing and attempt to close the tracing output file.
 */

MP_GLOBAL int __mp_endtrace(tracehead *t)
{
    int r;

    r = 1;
    if ((tracefile == NULL) || (tracefile == stderr) || (tracefile == stdout))
    {
        /* We don't want to close the stderr or stdout file streams so
         * we just flush them instead.  If the tracing file hasn't been set,
         * this will just flush all open output files.
         */
        if (fflush(tracefile))
            r = 0;
    }
    else if (fclose(tracefile))
        r = 0;
    tracefile = NULL;
    t->file = NULL;
    t->tracing = 0;
    return r;
}


/* Attempt to open the tracing output file.
 */

static int opentracefile(tracehead *t)
{
    /* The tracing file name can also be named as stderr and stdout which
     * will go to the standard error and standard output streams respectively.
     */
    if (t->file == NULL)
        return 0;
    else if (strcmp(t->file, "stderr") == 0)
        tracefile = stderr;
    else if (strcmp(t->file, "stdout") == 0)
        tracefile = stdout;
    else if ((tracefile = fopen(t->file, "wb")) == NULL)
    {
        __mp_error(ET_MAX, AT_MAX, NULL, 0, "%s: cannot open file\n", t->file);
        t->file = NULL;
        return 0;
    }
    return 1;
}


/* Record a memory allocation for tracing.
 */

MP_GLOBAL void __mp_tracealloc(tracehead *t, unsigned long n, void *a, size_t l)
{
    if (tracefile == NULL)
        opentracefile(t);
    fprintf(tracefile, "A %lu " MP_POINTER " %lu\n", n, a, l);
}


/* Record a memory deallocation for tracing.
 */

MP_GLOBAL void __mp_tracefree(tracehead *t, unsigned long n)
{
    if (tracefile == NULL)
        opentracefile(t);
    fprintf(tracefile, "F %lu\n", n);
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
