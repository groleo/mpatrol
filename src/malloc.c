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
 * Dynamic memory allocation functions.  Overrides the standard library
 * definitions of these functions if they haven't already been overridden
 * by the mpatrol.h header file.
 */


#include "inter.h"


#if MP_IDENT_SUPPORT
#ident "$Id: malloc.c,v 1.1.1.1 1999-10-03 11:25:22 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* Allocate an uninitialised memory block of a given size.
 */

void *malloc(size_t l)
{
    return __mp_alloc(l, 0, AT_MALLOC, NULL, NULL, 0, 1);
}


/* Allocate a zero-initialised memory block to hold enough space for an
 * array of elements of a given size.
 */

void *calloc(size_t l, size_t n)
{
    return __mp_alloc(l * n, 0, AT_CALLOC, NULL, NULL, 0, 1);
}


/* Allocate an uninitialised memory block of a given size and alignment.
 */

void *memalign(size_t a, size_t l)
{
    return __mp_alloc(l, a, AT_MEMALIGN, NULL, NULL, 0, 1);
}


/* Allocate an uninitialised memory block of a given size and aligned to
 * the system page size.
 */

void *valloc(size_t l)
{
    return __mp_alloc(l, 0, AT_VALLOC, NULL, NULL, 0, 1);
}


/* Allocate an uninitialised number of pages from the system.
 */

void *pvalloc(size_t l)
{
    return __mp_alloc(l, 0, AT_PVALLOC, NULL, NULL, 0, 1);
}


/* Duplicate an existing string using memory from the heap.
 */

char *strdup(const char *p)
{
    return __mp_strdup((char *) p, 0, AT_STRDUP, NULL, NULL, 0, 1);
}


/* Duplicate an existing string using memory from the heap, but set a limit
 * on the size of the memory allocated for the new string.
 */

char *strndup(const char *p, size_t l)
{
    return __mp_strdup((char *) p, l, AT_STRNDUP, NULL, NULL, 0, 1);
}


/* Duplicate an existing string using memory from the heap.
 */

char *strsave(const char *p)
{
    return __mp_strdup((char *) p, 0, AT_STRSAVE, NULL, NULL, 0, 1);
}


/* Duplicate an existing string using memory from the heap, but set a limit
 * on the size of the memory allocated for the new string.
 */

#if SYSTEM == SYSTEM_DGUX
char *strnsave(const char *p, int l)
#else /* SYSTEM */
char *strnsave(const char *p, size_t l)
#endif /* SYSTEM */
{
    return __mp_strdup((char *) p, l, AT_STRNSAVE, NULL, NULL, 0, 1);
}


/* Resize an existing block of memory.
 */

void *realloc(void *p, size_t l)
{
    return __mp_realloc(p, l, 0, AT_REALLOC, NULL, NULL, 0, 1);
}


/* Resize an existing block of memory, usually a block allocated by calloc().
 */

void *recalloc(void *p, size_t l)
{
    return __mp_realloc(p, l, 0, AT_RECALLOC, NULL, NULL, 0, 1);
}


/* Free an existing block of memory.
 */

void free(void *p)
{
    __mp_free(p, AT_FREE, NULL, NULL, 0, 1);
}


/* Free an existing block of memory, usually a block allocated by calloc().
 */

void cfree(void *p)
{
    __mp_free(p, AT_CFREE, NULL, NULL, 0, 1);
}


#ifdef __cplusplus
}
#endif /* __cplusplus */


/* Set the low-memory handler and return the previous setting.
 */

#ifdef __cplusplus
void (*set_new_handler(void (*h)(void)))(void)
#else /* __cplusplus */
void (*set_new_handler__FPFv_v(void (*h)(void)))(void)
#endif /* __cplusplus */
{
    return __mp_nomemory(h);
}


/* Allocate an uninitialised memory block of a given size.
 */

#ifdef __cplusplus
void *operator new(size_t l)
#else /* __cplusplus */
void *__nw__FUi(size_t l)
#endif /* __cplusplus */
{
    return __mp_alloc(l, 0, AT_NEW, NULL, NULL, 0, 1);
}


/* Allocate an uninitialised memory block of a given size for use by an array.
 */

#ifdef __cplusplus
void *operator new[](size_t l)
#else /* __cplusplus */
void *__arr_nw__FUi(size_t l)
#endif /* __cplusplus */
{
    return __mp_alloc(l, 0, AT_NEWVEC, NULL, NULL, 0, 1);
}


/* Free an existing block of memory that was allocated by operator new.
 */

#ifdef __cplusplus
void operator delete(void *p)
#else /* __cplusplus */
void __dl__FPv(void *p)
#endif /* __cplusplus */
{
    __mp_free(p, AT_DELETE, NULL, NULL, 0, 1);
}


/* Free an existing block of memory that was allocated by operator new[].
 */

#ifdef __cplusplus
void operator delete[](void *p)
#else /* __cplusplus */
void __arr_dl__FPv(void *p)
#endif /* __cplusplus */
{
    __mp_free(p, AT_DELETEVEC, NULL, NULL, 0, 1);
}
