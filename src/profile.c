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


#if MP_IDENT_SUPPORT
#ident "$Id: profile.c,v 1.1 2000-04-18 22:18:49 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* Record a memory allocation for profiling.
 */

MP_GLOBAL int __mp_profilealloc(allocnode *n)
{
    return 1;
}


/* Record a memory deallocation for profiling.
 */

MP_GLOBAL int __mp_profilefree(allocnode *n)
{
    return 1;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
