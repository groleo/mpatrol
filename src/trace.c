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
 * Memory allocation tracing.  If tracing is enabled then all memory
 * allocations and deallocations are written to a file in a compact
 * format for later processing by a tracing tool.
 */


#include "trace.h"
#include "diag.h"
#include "utils.h"
#include "version.h"
#include <stdio.h>
#include <string.h>


#if MP_IDENT_SUPPORT
#ident "$Id: trace.c,v 1.11 2001-02-05 22:58:34 graeme Exp $"
#else /* MP_IDENT_SUPPORT */
static MP_CONST MP_VOLATILE char *trace_id = "$Id: trace.c,v 1.11 2001-02-05 22:58:34 graeme Exp $";
#endif /* MP_IDENT_SUPPORT */


/* A rescache structure stores information about memory reservations on the
 * heap before the tracing output file has been opened.
 */

typedef struct rescache
{
    void *block;   /* pointer to block of memory */
    size_t size;   /* size of block of memory */
    char internal; /* allocation is internal */
}
rescache;


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* The cache that stores information about memory reservations on the heap
 * before the tracing output file has been opened.  The cachecounter variable
 * keeps a count of the number of entries stored in the cache.
 */

static rescache cache[MP_RESCACHE_SIZE];
static size_t cachecounter;


/* The file pointer to the tracing output file.  This should not really be
 * a file scope variable as it prevents this module from being re-entrant.
 * The traceready variable indicates when the tracing output file is ready
 * to be used.
 */

static FILE *tracefile;
static int traceready;


/* Initialise the fields of a tracehead so that the mpatrol library
 * is ready to trace memory allocations.
 */

MP_GLOBAL
void
__mp_newtrace(tracehead *t, meminfo *m)
{
    t->file = __mp_tracefile(m, NULL);
    t->tracing = 0;
    tracefile = NULL;
    traceready = 0;
}


/* Finish tracing and attempt to close the tracing output file.
 */

MP_GLOBAL
int
__mp_endtrace(tracehead *t)
{
    char s[4];
    int r;

    r = 1;
    traceready = 0;
    if ((t->tracing) && (tracefile != NULL))
    {
        __mp_memcopy(s, (char *) MP_TRACEMAGIC, 4);
        fwrite(s, sizeof(char), 4, tracefile);
    }
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

static
int
opentracefile(tracehead *t)
{
    char s[4];
    void *b;
    size_t i, l;
    unsigned long v;

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
    i = 1;
    v = MP_VERNUM;
    __mp_memcopy(s, (char *) MP_TRACEMAGIC, 4);
    fwrite(s, sizeof(char), 4, tracefile);
    fwrite(&i, sizeof(size_t), 1, tracefile);
    fwrite(&v, sizeof(unsigned long), 1, tracefile);
    traceready = 1;
    /* Write out all of the entries in the memory reservation cache.  This
     * only needs to be done when the tracing output file is opened since all
     * subsequent tracing events will be written out directly.
     */
    for (i = 0; i < cachecounter; i++)
    {
        if (cache[i].internal)
            fputc('I', tracefile);
        else
            fputc('H', tracefile);
        b = __mp_encodeuleb128((unsigned long) cache[i].block, &l);
        fwrite(b, l, 1, tracefile);
        b = __mp_encodeuleb128(cache[i].size, &l);
        fwrite(b, l, 1, tracefile);
    }
    cachecounter = 0;
    return 1;
}


/* Record a heap memory reservation for tracing.
 */

MP_GLOBAL
void
__mp_traceheap(void *a, size_t l, int i)
{
    void *b;
    size_t s;

    if (!traceready)
    {
        /* If the tracing output file has not yet been opened then it is
         * likely that the mpatrol library is still being initialised, in
         * which case it is unsafe to open the file due to the possibility
         * of recursion.  As a precautionary measure, we store the current
         * information in a cache that will be written out when the file is
         * finally opened.  If the cache is full, simply discard the current
         * information.
         */
        if (cachecounter < MP_RESCACHE_SIZE)
        {
            cache[cachecounter].block = a;
            cache[cachecounter].size = l;
            cache[cachecounter].internal = i;
            cachecounter++;
        }
        return;
    }
    if (i != 0)
        fputc('I', tracefile);
    else
        fputc('H', tracefile);
    /* Some of the following values are written as LEB128 numbers.  This is so
     * that the size of the tracing output file can be kept to a minimum.
     */
    b = __mp_encodeuleb128((unsigned long) a, &s);
    fwrite(b, s, 1, tracefile);
    b = __mp_encodeuleb128(l, &s);
    fwrite(b, s, 1, tracefile);
}


/* Record a memory allocation for tracing.
 */

MP_GLOBAL
void
__mp_tracealloc(tracehead *t, unsigned long n, void *a, size_t l)
{
    void *b;
    size_t s;

    if ((tracefile == NULL) && !opentracefile(t))
        return;
    fputc('A', tracefile);
    /* Some of the following values are written as LEB128 numbers.  This is so
     * that the size of the tracing output file can be kept to a minimum.
     */
    b = __mp_encodeuleb128(n, &s);
    fwrite(b, s, 1, tracefile);
    b = __mp_encodeuleb128((unsigned long) a, &s);
    fwrite(b, s, 1, tracefile);
    b = __mp_encodeuleb128(l, &s);
    fwrite(b, s, 1, tracefile);
}


/* Record a memory deallocation for tracing.
 */

MP_GLOBAL
void
__mp_tracefree(tracehead *t, unsigned long n)
{
    void *b;
    size_t s;

    if ((tracefile == NULL) && !opentracefile(t))
        return;
    fputc('F', tracefile);
    /* Some of the following values are written as LEB128 numbers.  This is so
     * that the size of the tracing output file can be kept to a minimum.
     */
    b = __mp_encodeuleb128(n, &s);
    fwrite(b, s, 1, tracefile);
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
