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
 * Library version and copyright definitions.  The version strings are
 * in the format most common to a specific operating system.  Note that
 * the dates are release dates, not dates of last compilation.
 */


#include "version.h"


#if MP_IDENT_SUPPORT
#ident "$Id: version.c,v 1.12 2000-01-09 20:35:24 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


#define MP_VERSION "mpatrol 1.0.8" /* library name and version */


#if TARGET == TARGET_UNIX

/* SCCS identification string and American date format.
 */
MP_GLOBAL char *__mp_version = "@(#) " MP_VERSION " (00/12/20)";

#elif TARGET == TARGET_AMIGA

/* Amiga identification string and European date format.
 */
MP_GLOBAL char *__mp_version = "$VER: " MP_VERSION " (20.12.00)";

#elif TARGET == TARGET_WINDOWS || TARGET == TARGET_NETWARE

/* No identification string and English date format.
 */
MP_GLOBAL char *__mp_version = MP_VERSION " (20 December 2000)";

#endif /* TARGET */


MP_GLOBAL char *__mp_copyright = "Copyright (C) 1997-2000 Graeme S. Roy";


#ifdef __cplusplus
}
#endif /* __cplusplus */
