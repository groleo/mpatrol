#ifndef MP_STACK_H
#define MP_STACK_H


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
 * Call stacks.  This module attempts to provide a common interface for
 * traversing the function call stack at a specific point during execution.
 */


#include "config.h"
#include <stddef.h>


/* A stackinfo structure provides information about the currently selected
 * stack frame.
 */

typedef struct stackinfo
{
    void *frame;               /* current frame handle */
    void *addr;                /* current return address */
#if MP_BUILTINSTACK_SUPPORT
    void *frames[MP_MAXSTACK]; /* array of frame pointers */
    void *addrs[MP_MAXSTACK];  /* array of return addresses */
    size_t index;              /* current stack index */
#else /* MP_BUILTINSTACK_SUPPORT */
    void *next;                /* next frame handle */
#endif /* MP_BUILTINSTACK_SUPPORT */
}
stackinfo;


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


MP_EXPORT void __mp_newframe(stackinfo *);
MP_EXPORT int __mp_getframe(stackinfo *);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* MP_STACK_H */
