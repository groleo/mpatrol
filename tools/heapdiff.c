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
 * Heap difference logger.  Uses mpatrol to log the differences in the
 * heap between two points in a program's execution, which can then be
 * used to help detect localised memory leaks.
 */


/*
 * $Id: heapdiff.c,v 1.1 2001-02-15 19:50:11 graeme Exp $
 */


#include "heapdiff.h"


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* The number of calls that have been made to heapdiffstart(), which is also
 * used to uniquely identify each new heapdiff object.  Reading and modifying
 * this variable is not currently threadsafe but this should not really be an
 * issue unless the ids of two heapdiff objects clash.
 */

static unsigned long count;


/* The callback function that is called by __mp_iterate() for every heap
 * allocation that has changed since a specified heap event.
 */

static
int
callback(MP_CONST void *p, void *t)
{
    heapdiff *h;
    char *s;
    __mp_allocinfo d;

    if (!__mp_info(p, &d))
        return 0;
    h = (heapdiff *) t;
    if (((h->flags & HD_FREED) && d.freed) ||
        ((h->flags & HD_UNFREED) && !d.freed))
    {
        if (h->flags & HD_FULL)
            __mp_logaddr(d.block);
        else
        {
            if (d.func != NULL)
                s = d.func;
            else if ((d.stack != NULL) && (d.stack->name != NULL))
                s = d.stack->name;
            else
                s = NULL;
            if (s != NULL)
                __mp_printf("%lu byte%s %s in %s at %s line %lu\n", d.size,
                            (d.size == 1) ? "" : "s",
                            d.freed ? "freed" : "allocated", s,
                            d.file ? d.file : "<unknown>", d.line);
            else
                __mp_printf("%lu byte%s %s at %s line %lu\n", d.size,
                            (d.size == 1) ? "" : "s",
                            d.freed ? "freed" : "allocated",
                            d.file ? d.file : "<unknown>", d.line);
        }
        if ((h->flags & HD_VIEW) && (d.file != NULL) && (d.line != 0))
            __mp_view(d.file, d.line);
        h->count++;
        h->total += d.size;
        return 1;
    }
    return 0;
}


/* Initialise a heapdiff object and take a snapshot of the current heap state.
 */

void
heapdiffstart(heapdiff *h, unsigned long f, MP_CONST char *t, unsigned long u)
{
    h->id = ++count;
    h->event = __mp_snapshot();
    h->flags = f;
    __mp_getoption(MP_OPT_NOFREE, &h->nofree);
    if (f & HD_FREED)
        __mp_setoption(MP_OPT_NOFREE, ~0);
    h->count = h->total = 0;
    if ((t != NULL) && (u != 0))
        __mp_printf("HEAPDIFF %lu STARTING at %s line %lu {\n", h->id, t, u);
    else
        __mp_printf("HEAPDIFF %lu STARTING {\n", h->id);
    __mp_logstack(1);
}


/* Iterate over the all of the heap allocations, logging any that have changed
 * since the given heapdiff object was initialised, and also logging any useful
 * statistics that can be gathered before reinitialising the heapdiff object.
 */

void
heapdiffend(heapdiff *h, MP_CONST char *t, unsigned long u)
{
    unsigned long f;

    if (h->id == 0)
        return;
    if ((t != NULL) && (u != 0))
        __mp_printf("} HEAPDIFF %lu ENDING at %s line %lu\n", h->id, t, u);
    else
        __mp_printf("} HEAPDIFF %lu ENDING\n", h->id);
    __mp_logstack(1);
    f = h->flags;
    if (f & HD_FREED)
    {
        h->flags &= ~HD_UNFREED;
        h->count = h->total = 0;
        __mp_printf("freed allocations:\n\n");
        if (__mp_iterate(callback, h, h->event) && !(f & HD_FULL))
            __mp_printf("\n");
        __mp_printf("total freed: %lu (%lu byte%s)\n\n", h->count, h->total,
                    (h->total == 1) ? "" : "s");
        h->flags = f;
    }
    if (f & HD_UNFREED)
    {
        h->flags &= ~HD_FREED;
        h->count = h->total = 0;
        __mp_printf("unfreed allocations:\n\n");
        if (__mp_iterate(callback, h, h->event) && !(f & HD_FULL))
            __mp_printf("\n");
        __mp_printf("total unfreed: %lu (%lu byte%s)\n\n", h->count, h->total,
                    (h->total == 1) ? "" : "s");
        h->flags = f;
    }
    __mp_setoption(MP_OPT_NOFREE, h->nofree);
    h->id = h->event = 0;
    h->flags = h->nofree = 0;
    h->count = h->total = 0;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
