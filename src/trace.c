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
#include <stdio.h>


#if MP_IDENT_SUPPORT
#ident "$Id: trace.c,v 1.1 2000-11-30 19:59:33 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* Initialise the fields of a tracehead so that the mpatrol library
 * is ready to trace memory allocations and open the tracing output file.
 */

MP_GLOBAL void __mp_newtrace(tracehead *t, char *f)
{
    t->file = f;
    t->tracing = 0;
}


/* Finish tracing and close the tracing output file.
 */

MP_GLOBAL void __mp_endtrace(tracehead *t)
{
    t->file = NULL;
    t->tracing = 0;
}


/* Record a memory allocation for tracing.
 */

MP_GLOBAL void __mp_tracealloc(tracehead *t, unsigned long n, void *a, size_t l)
{
    fprintf(stdout, "A %lu " MP_POINTER " %lu\n", n, a, l);
}


/* Record a memory deallocation for tracing.
 */

MP_GLOBAL void __mp_tracefree(tracehead *t, unsigned long n)
{
    fprintf(stdout, "F %lu\n", n);
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
