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
 * Dynamic memory allocation functions and memory operation functions.
 * Overrides the standard library definitions of these functions if they
 * haven't already been overridden by the mpatrol.h header file.
 */


#include "inter.h"


#if MP_IDENT_SUPPORT
#ident "$Id: malloc.c,v 1.14 2000-05-10 19:54:08 graeme Exp $"
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


#if MP_ALTFUNCNAMES
void *MP_ALTFUNCNAME(malloc)(size_t l)
{
    return __mp_alloc(l, 0, AT_MALLOC, NULL, NULL, 0, 1);
}
#endif /* MP_ALTFUNCNAMES */


/* Allocate a zero-initialised memory block to hold enough space for an
 * array of elements of a given size.
 */

void *calloc(size_t l, size_t n)
{
    return __mp_alloc(l * n, 0, AT_CALLOC, NULL, NULL, 0, 1);
}


#if MP_ALTFUNCNAMES
void *MP_ALTFUNCNAME(calloc)(size_t l, size_t n)
{
    return __mp_alloc(l * n, 0, AT_CALLOC, NULL, NULL, 0, 1);
}
#endif /* MP_ALTFUNCNAMES */


/* Allocate an uninitialised memory block of a given size and alignment.
 */

void *memalign(size_t a, size_t l)
{
    return __mp_alloc(l, a, AT_MEMALIGN, NULL, NULL, 0, 1);
}


#if MP_ALTFUNCNAMES
void *MP_ALTFUNCNAME(memalign)(size_t a, size_t l)
{
    return __mp_alloc(l, a, AT_MEMALIGN, NULL, NULL, 0, 1);
}
#endif /* MP_ALTFUNCNAMES */


/* Allocate an uninitialised memory block of a given size and aligned to
 * the system page size.
 */

void *valloc(size_t l)
{
    return __mp_alloc(l, 0, AT_VALLOC, NULL, NULL, 0, 1);
}


#if MP_ALTFUNCNAMES
void *MP_ALTFUNCNAME(valloc)(size_t l)
{
    return __mp_alloc(l, 0, AT_VALLOC, NULL, NULL, 0, 1);
}
#endif /* MP_ALTFUNCNAMES */


/* Allocate an uninitialised number of pages from the system.
 */

void *pvalloc(size_t l)
{
    return __mp_alloc(l, 0, AT_PVALLOC, NULL, NULL, 0, 1);
}


#if MP_ALTFUNCNAMES
void *MP_ALTFUNCNAME(pvalloc)(size_t l)
{
    return __mp_alloc(l, 0, AT_PVALLOC, NULL, NULL, 0, 1);
}
#endif /* MP_ALTFUNCNAMES */


/* Duplicate an existing string using memory from the heap.
 */

char *strdup(MP_CONST char *p)
{
    return __mp_strdup((char *) p, 0, AT_STRDUP, NULL, NULL, 0, 1);
}


#if MP_ALTFUNCNAMES
char *MP_ALTFUNCNAME(strdup)(MP_CONST char *p)
{
    return __mp_strdup((char *) p, 0, AT_STRDUP, NULL, NULL, 0, 1);
}
#endif /* MP_ALTFUNCNAMES */


/* Duplicate an existing string using memory from the heap, but set a limit
 * on the size of the memory allocated for the new string.
 */

char *strndup(MP_CONST char *p, size_t l)
{
    return __mp_strdup((char *) p, l, AT_STRNDUP, NULL, NULL, 0, 1);
}


#if MP_ALTFUNCNAMES
char *MP_ALTFUNCNAME(strndup)(MP_CONST char *p, size_t l)
{
    return __mp_strdup((char *) p, l, AT_STRNDUP, NULL, NULL, 0, 1);
}
#endif /* MP_ALTFUNCNAMES */


/* Duplicate an existing string using memory from the heap.
 */

char *strsave(MP_CONST char *p)
{
    return __mp_strdup((char *) p, 0, AT_STRSAVE, NULL, NULL, 0, 1);
}


#if MP_ALTFUNCNAMES
char *MP_ALTFUNCNAME(strsave)(MP_CONST char *p)
{
    return __mp_strdup((char *) p, 0, AT_STRSAVE, NULL, NULL, 0, 1);
}
#endif /* MP_ALTFUNCNAMES */


/* Duplicate an existing string using memory from the heap, but set a limit
 * on the size of the memory allocated for the new string.
 */

#if SYSTEM == SYSTEM_DGUX
char *strnsave(MP_CONST char *p, int l)
#else /* SYSTEM */
char *strnsave(MP_CONST char *p, size_t l)
#endif /* SYSTEM */
{
    return __mp_strdup((char *) p, l, AT_STRNSAVE, NULL, NULL, 0, 1);
}


#if MP_ALTFUNCNAMES
#if SYSTEM == SYSTEM_DGUX
char *MP_ALTFUNCNAME(strnsave)(MP_CONST char *p, int l)
#else /* SYSTEM */
char *MP_ALTFUNCNAME(strnsave)(MP_CONST char *p, size_t l)
#endif /* SYSTEM */
{
    return __mp_strdup((char *) p, l, AT_STRNSAVE, NULL, NULL, 0, 1);
}
#endif /* MP_ALTFUNCNAMES */


/* Resize an existing block of memory.
 */

void *realloc(void *p, size_t l)
{
    return __mp_realloc(p, l, 0, AT_REALLOC, NULL, NULL, 0, 1);
}


#if MP_ALTFUNCNAMES
void *MP_ALTFUNCNAME(realloc)(void *p, size_t l)
{
    return __mp_realloc(p, l, 0, AT_REALLOC, NULL, NULL, 0, 1);
}
#endif /* MP_ALTFUNCNAMES */


/* Resize an existing block of memory, usually a block allocated by calloc().
 */

void *recalloc(void *p, size_t l, size_t n)
{
    return __mp_realloc(p, l * n, 0, AT_RECALLOC, NULL, NULL, 0, 1);
}


#if MP_ALTFUNCNAMES
void *MP_ALTFUNCNAME(recalloc)(void *p, size_t l, size_t n)
{
    return __mp_realloc(p, l * n, 0, AT_RECALLOC, NULL, NULL, 0, 1);
}
#endif /* MP_ALTFUNCNAMES */


/* Resize an existing block of memory, but never relocate it.
 */

void *expand(void *p, size_t l)
{
    return __mp_realloc(p, l, 0, AT_EXPAND, NULL, NULL, 0, 1);
}


#if MP_ALTFUNCNAMES
void *MP_ALTFUNCNAME(expand)(void *p, size_t l)
{
    return __mp_realloc(p, l, 0, AT_EXPAND, NULL, NULL, 0, 1);
}
#endif /* MP_ALTFUNCNAMES */


/* Free an existing block of memory.
 */

void free(void *p)
{
    __mp_free(p, AT_FREE, NULL, NULL, 0, 1);
}


#if MP_ALTFUNCNAMES
void MP_ALTFUNCNAME(free)(void *p)
{
    __mp_free(p, AT_FREE, NULL, NULL, 0, 1);
}
#endif /* MP_ALTFUNCNAMES */


/* Free an existing block of memory, usually a block allocated by calloc().
 */

void cfree(void *p, size_t l, size_t n)
{
    __mp_free(p, AT_CFREE, NULL, NULL, 0, 1);
}


#if MP_ALTFUNCNAMES
void MP_ALTFUNCNAME(cfree)(void *p, size_t l, size_t n)
{
    __mp_free(p, AT_CFREE, NULL, NULL, 0, 1);
}
#endif /* MP_ALTFUNCNAMES */


/* Set a block of memory to a specific byte.
 */

void *memset(void *p, int c, size_t l)
{
    return __mp_setmem(p, l, (unsigned char) c, AT_MEMSET, NULL, NULL, 0, 1);
}


#if MP_ALTFUNCNAMES
void *MP_ALTFUNCNAME(memset)(void *p, int c, size_t l)
{
    return __mp_setmem(p, l, (unsigned char) c, AT_MEMSET, NULL, NULL, 0, 1);
}
#endif /* MP_ALTFUNCNAMES */


/* Set a block of memory to the zero byte.
 */

void bzero(void *p, size_t l)
{
    __mp_setmem(p, l, 0, AT_BZERO, NULL, NULL, 0, 1);
}


#if MP_ALTFUNCNAMES
void MP_ALTFUNCNAME(bzero)(void *p, size_t l)
{
    __mp_setmem(p, l, 0, AT_BZERO, NULL, NULL, 0, 1);
}
#endif /* MP_ALTFUNCNAMES */


/* Copy a non-overlapping block of memory from one address to another.
 */

void *memcpy(void *q, MP_CONST void *p, size_t l)
{
    return __mp_copymem((void *) p, q, l, AT_MEMCPY, NULL, NULL, 0, 1);
}


#if MP_ALTFUNCNAMES
void *MP_ALTFUNCNAME(memcpy)(void *q, MP_CONST void *p, size_t l)
{
    return __mp_copymem((void *) p, q, l, AT_MEMCPY, NULL, NULL, 0, 1);
}
#endif /* MP_ALTFUNCNAMES */


/* Copy a possibly-overlapping block of memory from one address to another.
 */

void *memmove(void *q, MP_CONST void *p, size_t l)
{
    return __mp_copymem((void *) p, q, l, AT_MEMMOVE, NULL, NULL, 0, 1);
}


#if MP_ALTFUNCNAMES
void *MP_ALTFUNCNAME(memmove)(void *q, MP_CONST void *p, size_t l)
{
    return __mp_copymem((void *) p, q, l, AT_MEMMOVE, NULL, NULL, 0, 1);
}
#endif /* MP_ALTFUNCNAMES */


/* Copy a possibly-overlapping block of memory from one address to another.
 */

void bcopy(MP_CONST void *p, void *q, size_t l)
{
    __mp_copymem((void *) p, q, l, AT_BCOPY, NULL, NULL, 0, 1);
}


#if MP_ALTFUNCNAMES
void MP_ALTFUNCNAME(bcopy)(MP_CONST void *p, void *q, size_t l)
{
    __mp_copymem((void *) p, q, l, AT_BCOPY, NULL, NULL, 0, 1);
}
#endif /* MP_ALTFUNCNAMES */


/* Look for the first occurrence of a character in a block of memory.
 */

void *memchr(MP_CONST void *p, int c, size_t l)
{
    return __mp_locatemem((void *) p, l, NULL, (size_t) c, AT_MEMCHR, NULL,
                          NULL, 0, 1);
}


#if MP_ALTFUNCNAMES
void *MP_ALTFUNCNAME(memchr)(MP_CONST void *p, int c, size_t l)
{
    return __mp_locatemem((void *) p, l, NULL, (size_t) c, AT_MEMCHR, NULL,
                          NULL, 0, 1);
}
#endif /* MP_ALTFUNCNAMES */


/* Attempt to locate the position of one block of memory in another block.
 */

void *memmem(MP_CONST void *p, size_t l, MP_CONST void *q, size_t m)
{
    return __mp_locatemem((void *) p, l, (void *) q, m, AT_MEMMEM, NULL, NULL,
                          0, 1);
}


#if MP_ALTFUNCNAMES
void *MP_ALTFUNCNAME(memmem)(MP_CONST void *p, size_t l, MP_CONST void *q,
                             size_t m)
{
    return __mp_locatemem((void *) p, l, (void *) q, m, AT_MEMMEM, NULL, NULL,
                          0, 1);
}
#endif /* MP_ALTFUNCNAMES */


/* Compare two blocks of memory.
 */

int memcmp(MP_CONST void *p, MP_CONST void *q, size_t l)
{
    return __mp_comparemem((void *) p, (void *) q, l, AT_MEMCMP, NULL, NULL, 0,
                           1);
}


#if MP_ALTFUNCNAMES
int MP_ALTFUNCNAME(memcmp)(MP_CONST void *p, MP_CONST void *q, size_t l)
{
    return __mp_comparemem((void *) p, (void *) q, l, AT_MEMCMP, NULL, NULL, 0,
                           1);
}
#endif /* MP_ALTFUNCNAMES */


/* Compare two blocks of memory.
 */

int bcmp(MP_CONST void *p, MP_CONST void *q, size_t l)
{
    return __mp_comparemem((void *) p, (void *) q, l, AT_BCMP, NULL, NULL, 0,
                           1);
}


#if MP_ALTFUNCNAMES
int MP_ALTFUNCNAME(bcmp)(MP_CONST void *p, MP_CONST void *q, size_t l)
{
    return __mp_comparemem((void *) p, (void *) q, l, AT_BCMP, NULL, NULL, 0,
                           1);
}
#endif /* MP_ALTFUNCNAMES */


#ifdef __cplusplus
}
#endif /* __cplusplus */


/* Set the low-memory handler and return the previous setting.
 */

#ifdef __cplusplus
#ifdef __GNUC__
extern "C" void (*set_new_handler(void (*h)(void)))(void)
#else /* __GNUC__ */
void (*set_new_handler(void (*h)(void)))(void)
#endif /* __GNUC__ */
#else /* __cplusplus */
#ifdef __GNUC__
void (*set_new_handler(void (*h)(void)))(void)
#else /* __GNUC__ */
void (*set_new_handler__FPFv_v(void (*h)(void)))(void)
#endif /* __GNUC__ */
#endif /* __cplusplus */
{
    return __mp_nomemory(h);
}


/* Allocate an uninitialised memory block of a given size.
 */

#ifdef __cplusplus
void *operator new(size_t l)
#else /* __cplusplus */
#ifdef __GNUC__
void *__builtin_new(size_t l)
#else /* __GNUC__ */
void *__nw__FUi(size_t l)
#endif /* __GNUC__ */
#endif /* __cplusplus */
{
    return __mp_alloc(l, 0, AT_NEW, NULL, NULL, 0, 1);
}


/* Allocate an uninitialised memory block of a given size for use by an array.
 */

#ifdef __cplusplus
void *operator new[](size_t l)
#else /* __cplusplus */
#ifdef __GNUC__
void *__builtin_vec_new(size_t l)
#else /* __GNUC__ */
void *__arr_nw__FUi(size_t l)
#endif /* __GNUC__ */
#endif /* __cplusplus */
{
    return __mp_alloc(l, 0, AT_NEWVEC, NULL, NULL, 0, 1);
}


/* Free an existing block of memory that was allocated by operator new.
 */

#ifdef __cplusplus
void operator delete(void *p)
#else /* __cplusplus */
#ifdef __GNUC__
void __builtin_delete(void *p)
#else /* __GNUC__ */
void __dl__FPv(void *p)
#endif /* __GNUC__ */
#endif /* __cplusplus */
{
    __mp_free(p, AT_DELETE, NULL, NULL, 0, 1);
}


/* Free an existing block of memory that was allocated by operator new[].
 */

#ifdef __cplusplus
void operator delete[](void *p)
#else /* __cplusplus */
#ifdef __GNUC__
void __builtin_vec_delete(void *p)
#else /* __GNUC__ */
void __arr_dl__FPv(void *p)
#endif /* __GNUC__ */
#endif /* __cplusplus */
{
    __mp_free(p, AT_DELETEVEC, NULL, NULL, 0, 1);
}
