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
 * C++ dynamic memory allocation functions.  Overrides the standard library
 * definitions of these functions if they haven't already been overridden by
 * the mpatrol.h header file.  This source file should be compiled with a
 * C++ compiler but best-guess definitions of the functions will be made if
 * it is compiled with a C compiler.
 */


#include "inter.h"
#ifdef __cplusplus
#include <new>
#endif /* __cplusplus */


#if MP_IDENT_SUPPORT
#ident "$Id: cplus.c,v 1.2 2001-02-05 22:58:33 graeme Exp $"
#else /* MP_IDENT_SUPPORT */
static MP_CONST MP_VOLATILE char *cplus_id = "$Id: cplus.c,v 1.2 2001-02-05 22:58:33 graeme Exp $";
#endif /* MP_IDENT_SUPPORT */


/* Set the low-memory handler and return the previous setting.
 */

#ifdef __cplusplus
std::new_handler
std::set_new_handler(std::new_handler h) throw()
#elif defined(__GNUC__)
void
(*set_new_handler__FPFv_v(void (*h)(void)))(void)
#else /* __cplusplus && __GNUC__ */
void
(*set_new_handler__3stdFPFv_v(void (*h)(void)))(void)
#endif /* __cplusplus && __GNUC__ */
{
#ifdef __cplusplus
    return (std::new_handler) __mp_nomemory(h);
#else /* __cplusplus */
    return __mp_nomemory(h);
#endif /* __cplusplus */
}


/* Allocate an uninitialised memory block of a given size, throwing an
 * exception if the allocation cannot be made.
 */

#ifdef __cplusplus
void *
operator new(size_t l) throw(std::bad_alloc)
#elif defined(__GNUC__)
void *
__builtin_new(size_t l)
#else /* __cplusplus && __GNUC__ */
void *
__nw__FUi(size_t l)
#endif /* __cplusplus && __GNUC__ */
{
    void *p;

    if ((p = __mp_alloc(l, 0, AT_NEW, NULL, NULL, 0, NULL, 0, 1)) == NULL)
#ifdef __cplusplus
        throw std::bad_alloc();
#else /* __cplusplus */
        abort();
#endif /* __cplusplus */
    return p;
}


/* Allocate an uninitialised memory block of a given size, returning NULL
 * if the allocation cannot be made.
 */

#ifdef __cplusplus
void *
operator new(size_t l, MP_CONST std::nothrow_t&) throw()
#elif defined(__GNUC__)
void *
__nw__FuiRC9nothrow_t(size_t l)
#else /* __cplusplus && __GNUC__ */
void *
__nw__FUiRCQ2_3std9nothrow_t(size_t l)
#endif /* __cplusplus && __GNUC__ */
{
    return __mp_alloc(l, 0, AT_NEW, NULL, NULL, 0, NULL, 0, 1);
}


/* Allocate an uninitialised memory block of a given size for use by an array,
 * throwing an exception if the allocation cannot be made.
 */

#ifdef __cplusplus
void *
operator new[](size_t l) throw(std::bad_alloc)
#elif defined(__GNUC__)
void *
__builtin_vec_new(size_t l)
#else /* __cplusplus && __GNUC__ */
void *
__nwa__FUi(size_t l)
#endif /* __cplusplus && __GNUC__ */
{
    void *p;

    if ((p = __mp_alloc(l, 0, AT_NEWVEC, NULL, NULL, 0, NULL, 0, 1)) == NULL)
#ifdef __cplusplus
        throw std::bad_alloc();
#else /* __cplusplus */
        abort();
#endif /* __cplusplus */
    return p;
}


/* Allocate an uninitialised memory block of a given size for use by an array,
 * returning NULL if the allocation cannot be made.
 */

#ifdef __cplusplus
void *
operator new[](size_t l, MP_CONST std::nothrow_t&) throw()
#elif defined(__GNUC__)
void *
__vn__FuiRC9nothrow_t(size_t l)
#else /* __cplusplus && __GNUC__ */
void *
__nwa__FUiRCQ2_3std9nothrow_t(size_t l)
#endif /* __cplusplus && __GNUC__ */
{
    return __mp_alloc(l, 0, AT_NEWVEC, NULL, NULL, 0, NULL, 0, 1);
}


/* Free an existing block of memory that was allocated by operator new.
 */

#ifdef __cplusplus
void
operator delete(void *p) throw()
#elif defined(__GNUC__)
void
__builtin_delete(void *p)
#else /* __cplusplus && __GNUC__ */
void
__dl__FPv(void *p)
#endif /* __cplusplus && __GNUC__ */
{
    __mp_free(p, AT_DELETE, NULL, NULL, 0, 1);
}


/* Free an existing block of memory that was allocated by operator new.
 */

#ifdef __cplusplus
void
operator delete(void *p, MP_CONST std::nothrow_t&) throw()
#elif defined(__GNUC__)
void
__dl__FPvRC9nothrow_t(void *p)
#else /* __cplusplus && __GNUC__ */
void
__dl__FPvRCQ2_3std9nothrow_t(void *p)
#endif /* __cplusplus && __GNUC__ */
{
    __mp_free(p, AT_DELETE, NULL, NULL, 0, 1);
}


/* Free an existing block of memory that was allocated by operator new[].
 */

#ifdef __cplusplus
void
operator delete[](void *p) throw()
#elif defined(__GNUC__)
void
__builtin_vec_delete(void *p)
#else /* __cplusplus && __GNUC__ */
void
__dla__FPv(void *p)
#endif /* __cplusplus && __GNUC__ */
{
    __mp_free(p, AT_DELETEVEC, NULL, NULL, 0, 1);
}


/* Free an existing block of memory that was allocated by operator new[].
 */

#ifdef __cplusplus
void
operator delete[](void *p, MP_CONST std::nothrow_t&) throw()
#elif defined(__GNUC__)
void
__vd__FPvRC9nothrow_t(void *p)
#else /* __cplusplus && __GNUC__ */
void
__dla__FPvRCQ2_3std9nothrow_t(void *p)
#endif /* __cplusplus && __GNUC__ */
{
    __mp_free(p, AT_DELETEVEC, NULL, NULL, 0, 1);
}
