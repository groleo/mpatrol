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
 * Mathematical support routines.
 */


#include "utils.h"


#if MP_IDENT_SUPPORT
#ident "$Id: utils.c,v 1.1.1.1 1999-10-03 11:25:23 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* Return the base-two logarithm of an unsigned integer.
 */

MP_GLOBAL unsigned char __mp_logtwo(unsigned long n)
{
    unsigned char l;

    for (l = 0; n > 0; l++, n >>= 1);
    return (unsigned char) ((l == 0) ? 0 : l - 1);
}


/* Determine if an unsigned integer is a power of two.
 */

MP_GLOBAL int __mp_ispoweroftwo(unsigned long n)
{
    return ((n > 0) && ((n & (n - 1)) == 0));
}


/* Round an unsigned integer up to the nearest power of two.
 */

MP_GLOBAL unsigned long __mp_poweroftwo(unsigned long n)
{
    if ((n == 0) || __mp_ispoweroftwo(n))
        return n;
    return (unsigned long) (2 << __mp_logtwo(n));
}


/* Round an unsigned integer down to a specified power of two alignment.
 */

MP_GLOBAL unsigned long __mp_rounddown(unsigned long n, unsigned long a)
{
    return n & ~(a - 1);
}


/* Round an unsigned integer up to a specified power of two alignment.
 */

MP_GLOBAL unsigned long __mp_roundup(unsigned long n, unsigned long a)
{
    return ((n - 1) & ~(a - 1)) + a;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
