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
 * Memory allocation profiling.  The call graphs for every memory allocation
 * and deallocation are recorded here along with their memory usage statistics
 * and are written to a file for later processing by a profiling tool.
 */


#include "profile.h"
#include "info.h"


#if MP_IDENT_SUPPORT
#ident "$Id: profile.c,v 1.5 2000-04-19 19:33:25 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* Initialise the fields of a profhead so that the mpatrol library
 * is ready to profile memory allocations.
 */

MP_GLOBAL void __mp_newprofile(profhead *p)
{
    unsigned long i;

    for (i = 0; i < MP_BIN_SIZE; i++)
        p->acounts[i] = p->dcounts[i] = 0;
    p->acountl = p->dcountl = 0;
    p->sbound = MP_SMALLBOUND;
    p->mbound = MP_MEDIUMBOUND;
    p->lbound = MP_LARGEBOUND;
    p->file = MP_PROFFILE;
    p->profiling = 0;
}


/* Forget all existing profiling information.
 */

MP_GLOBAL void __mp_deleteprofile(profhead *p)
{
    unsigned long i;

    for (i = 0; i < MP_BIN_SIZE; i++)
        p->acounts[i] = p->dcounts[i] = 0;
    p->acountl = p->dcountl = 0;
    p->file = NULL;
    p->profiling = 0;
}


/* Record a memory allocation for profiling.
 */

MP_GLOBAL int __mp_profilealloc(profhead *p, size_t l, void *d)
{
    /* Note the size of the allocation in one of the allocation bins.
     * The highest allocation bin stores a count of all the allocations
     * that are larger than the largest bin.
     */
    if (l < MP_BIN_SIZE)
        p->acounts[l - 1]++;
    else
    {
        p->acounts[MP_BIN_SIZE - 1]++;
        p->acountl += l;
    }
    return 1;
}


/* Record a memory deallocation for profiling.
 */

MP_GLOBAL int __mp_profilefree(profhead *p, size_t l, void *d)
{
    /* Note the size of the deallocation in one of the deallocation bins.
     * The highest deallocation bin stores a count of all the deallocations
     * that are larger than the largest bin.
     */
    if (l < MP_BIN_SIZE)
        p->dcounts[l - 1]++;
    else
    {
        p->dcounts[MP_BIN_SIZE - 1]++;
        p->dcountl += l;
    }
    return 1;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
