#ifndef MPT_DBMALLOC_H
#define MPT_DBMALLOC_H


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
 * Dbmalloc-compatible interface.  Implements Dbmalloc functions using
 * mpatrol.  Dbmalloc is copyright (C) 1990-1992 Conor P. Cahill.
 */


/*
 * $Id: dbmalloc.h,v 1.1 2001-02-26 23:40:22 graeme Exp $
 */


/*
 *
 */


#include <mpatrol.h>


/* Commands for dbmallopt().  Some of them are ignored as they have no
 * meaning when used with the mpatrol library.
 */

#define MALLOC_WARN      100
#define MALLOC_FATAL     101
#define MALLOC_ERRFILE   102
#define MALLOC_CKCHAIN   103
#define MALLOC_FILLAREA  104
#define MALLOC_LOWFRAG   105
#define MALLOC_CKDATA    106
#define MALLOC_REUSE     107
#define MALLOC_SHOWLINKS 108
#define MALLOC_DETAIL    109
#define MALLOC_FREEMARK  110
#define MALLOC_ZERO      111


/* The union used to supply a command argument to dbmallopt().
 */

union dbmalloptarg
{
    int i;     /* integer value */
    char *str; /* string value */
};


#ifndef NDEBUG

#define dbmallopt(c, v) __mpt_dbmallocoption((c), (v))
#define malloc_chain_check(f) __mpt_dbmallocchaincheck(MP_FUNCNAME, __FILE__, \
                                                       __LINE__)
#define malloc_dump(f) __mpt_dbmallocdump(f)
#define malloc_list(f, l, u) __mpt_dbmalloclist((f), (l), (u))
#define malloc_inuse(h) __mpt_dbmallocinuse(h)
#define malloc_size(p) __mpt_dbmallocsize(p)
#define malloc_mark(p) (void) __mp_setmark(p)
#define malloc_enter(f) ((void) 0)
#define malloc_leave(f) ((void) 0)


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


int __mpt_dbmallocoption(int, union dbmalloptarg *);
int __mpt_dbmallocchaincheck(int, MP_CONST char *, MP_CONST char *,
                             unsigned long);
void __mpt_dbmallocdump(int);
void __mpt_dbmalloclist(int, unsigned long, unsigned long);
unsigned long __mpt_dbmallocinuse(unsigned long *);
size_t __mpt_dbmallocsize(MP_CONST void *);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#else /* NDEBUG */

#define dbmallopt(c, v) ((int) 1)
#define malloc_chain_check(f) ((int) 0)
#define malloc_dump(f) ((void) 0)
#define malloc_list(f, l, u) ((void) 0)
#define malloc_inuse(h) (*(h) = 0, (unsigned long) 0)
#define malloc_size(p) ((size_t) -1)
#define malloc_mark(p) ((void) 0)
#define malloc_enter(f) ((void) 0)
#define malloc_leave(f) ((void) 0)

#endif /* NDEBUG */


#endif /* MPT_DBMALLOC_H */
