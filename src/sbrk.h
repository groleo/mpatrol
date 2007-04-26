#ifndef MP_SBRK_H
#define MP_SBRK_H


/*
 * mpatrol
 * A library for controlling and tracing dynamic memory allocations.
 * Copyright (C) 1997-2007 Graeme S. Roy <mpatrol@cbmamiga.demon.co.uk>
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
 * Simulated UNIX heap.  These functions allow manipulation of the size of a
 * simulated heap and as a result can allocate or free memory from that heap.
 */


/*
 * $Id: sbrk.h,v 1.8 2007-04-26 11:27:57 groy Exp $
 */


#include "config.h"


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


MP_API int brk(void *);
MP_API void *sbrk(long);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* MP_SBRK_H */
