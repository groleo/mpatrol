#ifndef MPT_HEAPDIFF_H
#define MPT_HEAPDIFF_H


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
 * $Id: heapdiff.h,v 1.5 2001-02-26 22:50:39 graeme Exp $
 */


/*
 * This file defines heapdiffstart() and heapdiffend(), which must be
 * called in matching pairs.  They both take a heapdiff object as their
 * first parameter, which must still be in scope when the matching call
 * to heapdiffend() is made.  The heapdiff object is initialised at the
 * call to heapdiffstart() and is finalised when heapdiffend() is called.
 * It must not be modified in between and should be treated as an opaque
 * type.  heapdiffend() can only be called once per heapdiff object
 * before requiring that the heapdiff object be reinitialised through a
 * call to heapdiffstart().
 *
 * The second parameter to heapdiffstart() specifies a set of flags that
 * can be used to control what is written to the mpatrol log.  A list of
 * all unfreed memory allocations can be logged with the HD_UNFREED flag
 * and a list of all freed memory allocations can be logged with the
 * HD_FREED flag, although the latter makes use of the NOFREE option and
 * can incur a large performance and space penalty, and also relies on
 * the NOFREE option being unmodified between the calls to heapdiffstart()
 * and heapdiffend().  Note that marked allocations are not normally
 * logged but this can be changed by adding the HD_MARKED flag.
 *
 * By default, only a minimal amount of detail is logged for each
 * allocation, but this can be changed with the HD_FULL flag to log full
 * details for each allocation.  If the filename and line number for an
 * allocation is known and the EDIT or LIST option is being used then
 * using HD_VIEW will edit or list the relevant source file at the correct
 * line number, but only if the EDIT or LIST options are supported.
 */


#include <mpatrol.h>


#define HD_FREED   1  /* log all freed allocations */
#define HD_UNFREED 2  /* log all unfreed allocations */
#define HD_MARKED  4  /* include marked allocations */
#define HD_FULL    8  /* log full details of each allocation */
#define HD_VIEW    16 /* view each allocation */


/* The structure used to store the current state of the heap and any
 * useful statistics gathered.
 */

typedef struct heapdiff
{
    unsigned long id;     /* heapdiff identifier */
    unsigned long event;  /* event at time of snapshot */
    unsigned long flags;  /* flags determining behaviour */
    unsigned long nofree; /* previous NOFREE setting */
    unsigned long count;  /* total number of allocations */
    unsigned long total;  /* total bytes in allocations */
}
heapdiff;


#define heapdiffstart(h, f) __mpt_heapdiffstart(&(h), (f), __FILE__, __LINE__)
#define heapdiffend(h) __mpt_heapdiffend(&(h), __FILE__, __LINE__)


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


void __mpt_heapdiffstart(heapdiff *, unsigned long, MP_CONST char *,
                         unsigned long);
void __mpt_heapdiffend(heapdiff *, MP_CONST char *, unsigned long);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* MPT_HEAPDIFF_H */
