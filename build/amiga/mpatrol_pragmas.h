#ifndef MPatrolBase_INLINE
#define MPatrolBase_INLINE


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
 * Definitions for calling the shared library.
 */


#pragma libcall MPatrolBase __mp_init 1e 0
#pragma libcall MPatrolBase __mp_fini 24 0
#pragma libcall MPatrolBase __mp_alloc 2a 439821007
#pragma libcall MPatrolBase __mp_strdup 30 32A910807
#pragma libcall MPatrolBase __mp_realloc 36 43A9210808
#pragma libcall MPatrolBase __mp_free 3c 21A90806
#pragma libcall MPatrolBase __mp_setmem 42 43A9210808
#pragma libcall MPatrolBase __mp_copymem 48 32BA109808
#pragma libcall MPatrolBase __mp_comparemem 4e 32BA109808
#pragma libcall MPatrolBase __mp_locatemem 54 43BA2190809
#pragma libcall MPatrolBase __mp_info 5a 9802
#pragma libcall MPatrolBase __mp_memorymap 60 001
#pragma libcall MPatrolBase __mp_summary 66 0
#pragma libcall MPatrolBase __mp_check 6c 0
#pragma libcall MPatrolBase __mp_prologue 72 801
#pragma libcall MPatrolBase __mp_epilogue 78 801
#pragma libcall MPatrolBase __mp_nomemory 7e 801


#endif /* MPatrolBase_INLINE */
