#ifndef MP_PROFILE_H
#define MP_PROFILE_H


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
 * Memory allocation profiling.  This interface is used by the mpatrol
 * library to enable or disable the profiling of dynamic memory allocations
 * to a file for later processing.
 */


#include "config.h"
#include <stddef.h>


/* A profhead contains all the profiling information including the
 * filename of the output file and the current profiling state.
 */

typedef struct profhead
{
    unsigned long acounts[MP_BIN_SIZE]; /* allocation bins */
    unsigned long dcounts[MP_BIN_SIZE]; /* deallocation bins */
    unsigned long acountl;              /* total bytes of large allocations */
    unsigned long dcountl;              /* total bytes of large deallocations */
    char *file;                         /* profiling filename */
    char profiling;                     /* profiling status */
}
profhead;


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


MP_EXPORT void __mp_newprofile(profhead *);
MP_EXPORT void __mp_deleteprofile(profhead *);
MP_EXPORT int __mp_profilealloc(profhead *, size_t, void *);
MP_EXPORT int __mp_profilefree(profhead *, size_t, void *);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* MP_PROFILE_H */
