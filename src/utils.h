#ifndef MP_UTILS_H
#define MP_UTILS_H


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
 * Mathematical support routines.
 */


#include "config.h"


#if MP_MACROROUTINES
#define __mp_ispoweroftwo(n) \
    (((unsigned long) (n) > 0) && (((unsigned long) (n) & \
       ((unsigned long) (n) - 1)) == 0))
#define __mp_rounddown(n, a) \
    ((unsigned long) (n) & ~((unsigned long) (a) - 1))
#define __mp_roundup(n, a) \
    ((((unsigned long) (n) - 1) & ~((unsigned long) (a) - 1)) + \
     (unsigned long) (a))
#endif /* MP_MACROROUTINES */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


MP_EXPORT unsigned char __mp_logtwo(unsigned long);
MP_EXPORT unsigned long __mp_squareroot(unsigned long);
#if !MP_MACROROUTINES
MP_EXPORT int __mp_ispoweroftwo(unsigned long);
#endif /* MP_MACROROUTINES */
MP_EXPORT unsigned long __mp_poweroftwo(unsigned long);
#if !MP_MACROROUTINES
MP_EXPORT unsigned long __mp_rounddown(unsigned long, unsigned long);
MP_EXPORT unsigned long __mp_roundup(unsigned long, unsigned long);
#endif /* MP_MACROROUTINES */


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* MP_UTILS_H */
