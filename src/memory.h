#ifndef MP_MEMORY_H
#define MP_MEMORY_H


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
 * Memory handling.  All memory allocations made by the mpatrol library
 * are performed by this module.  This provides an interface which shields
 * the rest of the library source code from the system calls required to
 * allocate and manipulate memory on different operating systems.
 */


#include "config.h"
#include <stddef.h>


/* The different types of memory access permissions.
 */

typedef enum memaccess
{
    MA_NOACCESS, /* no read or write */
    MA_READONLY, /* no write */
    MA_READWRITE /* both read and write */
}
memaccess;


/* A meminfo structure contains details about the underlying memory
 * architecture.
 */

typedef struct meminfo
{
    size_t align; /* most restrictive alignment */
    size_t page;  /* system page size */
    char *prog;   /* program filename */
    int file;     /* control file handle */
}
meminfo;


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


MP_EXPORT void __mp_newmemory(meminfo *);
MP_EXPORT void __mp_endmemory(meminfo *);
MP_EXPORT unsigned long __mp_processid(void);
MP_EXPORT void *__mp_memalloc(meminfo *, size_t *, size_t);
MP_EXPORT void __mp_memfree(meminfo *, void *, size_t);
MP_EXPORT int __mp_memprotect(meminfo *, void *, size_t, memaccess);
MP_EXPORT int __mp_memwatch(meminfo *, void *, size_t, memaccess);
MP_EXPORT void *__mp_memcheck(void *, char, size_t);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* MP_MEMORY_H */
