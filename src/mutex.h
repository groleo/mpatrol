#ifndef MP_MUTEX_H
#define MP_MUTEX_H


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
 * Threads interface.  This module provides a common interface for the
 * mpatrol library when making calls for thread-safe facilities.
 */


#include "config.h"


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


MP_EXPORT unsigned long __mp_threadid(void);
MP_EXPORT void __mp_newmutex(void);
MP_EXPORT void __mp_deletemutex(void);
MP_EXPORT void __mp_lockmutex(void);
MP_EXPORT void __mp_unlockmutex(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* MP_MUTEX_H */
