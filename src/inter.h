#ifndef MP_INTER_H
#define MP_INTER_H


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
 * Library interface.  This module defines the visible interface for the
 * mpatrol library.
 */


/*
 * $Id: inter.h,v 1.33 2001-03-02 01:34:19 graeme Exp $
 */


#include "config.h"
#include "info.h"
#include <stdarg.h>


/* An allocinfo structure provides information about a particular memory
 * allocation.  This must be kept up to date with the definition of
 * __mp_allocinfo in mpatrol.h.
 */

typedef struct allocinfo
{
    void *block;           /* pointer to block of memory */
    size_t size;           /* size of block of memory */
    alloctype type;        /* type of memory allocation */
    unsigned long alloc;   /* allocation index */
    unsigned long realloc; /* reallocation index */
    unsigned long thread;  /* thread identifier */
    unsigned long event;   /* event of last modification */
    char *func;            /* calling function name */
    char *file;            /* file name in which call took place */
    unsigned long line;    /* line number at which call took place */
    addrnode *stack;       /* call stack details */
    char *typestr;         /* type stored in allocation */
    size_t typesize;       /* size of type stored in allocation */
    void *userdata;        /* user data associated with allocation */
    int freed : 1;         /* allocation has been freed */
    int marked : 1;        /* allocation has been marked */
    int profiled : 1;      /* allocation has been profiled */
    int traced : 1;        /* allocation has been traced */
    int internal : 1;      /* allocation is internal */
}
allocinfo;


/* A symbolinfo structure provides information about a particular symbol.
 * This must be kept up to date with the definition of __mp_symbolinfo in
 * mpatrol.h.
 */

typedef struct symbolinfo
{
    char *name;         /* symbol name */
    char *object;       /* module symbol located in */
    void *addr;         /* start address */
    size_t size;        /* size of symbol */
    char *file;         /* file name corresponding to address */
    unsigned long line; /* line number corresponding to address */
}
symbolinfo;


/* A heapinfo structure provides statistics about the current state of the
 * heap.  This must be kept up to date with the definition of __mp_heapinfo
 * in mpatrol.h
 */

typedef struct heapinfo
{
    size_t acount; /* total number of allocated blocks */
    size_t atotal; /* total size of allocated blocks */
    size_t fcount; /* total number of free blocks */
    size_t ftotal; /* total size of free blocks */
    size_t gcount; /* total number of freed blocks */
    size_t gtotal; /* total size of freed blocks */
    size_t icount; /* total number of internal blocks */
    size_t itotal; /* total size of internal blocks */
}
heapinfo;


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


void __mp_init(void);
void __mp_fini(void);
void __mp_trap(void);
int __mp_atexit(void (*)(void));
unsigned long __mp_setoption(long, unsigned long);
int __mp_getoption(long, unsigned long *);
infohead *__mp_memhead(void);
void *__mp_alloc(size_t, size_t, alloctype, char *, char *, unsigned long,
                 char *, size_t, size_t);
char *__mp_strdup(char *, size_t, alloctype, char *, char *, unsigned long,
                  size_t);
void *__mp_realloc(void *, size_t, size_t, alloctype, char *, char *,
                   unsigned long, char *, size_t, size_t);
void __mp_free(void *, alloctype, char *, char *, unsigned long, size_t);
void *__mp_setmem(void *, size_t, unsigned char, alloctype, char *, char *,
                  unsigned long, size_t);
void *__mp_copymem(void *, void *, size_t, unsigned char, alloctype, char *,
                   char *, unsigned long, size_t);
void *__mp_locatemem(void *, size_t, void *, size_t, alloctype, char *, char *,
                     unsigned long, size_t);
int __mp_comparemem(void *, void *, size_t, alloctype, char *, char *,
                    unsigned long, size_t);
char *__mp_function(alloctype);
int __mp_setuser(void *, void *);
int __mp_setmark(void *);
int __mp_info(void *, allocinfo *);
int __mp_syminfo(void *, symbolinfo *);
int __mp_printinfo(void *);
unsigned long __mp_snapshot(void);
size_t __mp_iterate(int (*)(void *, void *), void *, unsigned long);
void __mp_memorymap(int);
void __mp_summary(void);
int __mp_stats(heapinfo *);
void __mp_checkheap(char *, char *, unsigned long);
void __mp_check(void);
void (*__mp_prologue(void (*)(void *, size_t, void *)))(void *, size_t, void *);
void (*__mp_epilogue(void (*)(void *, void *)))(void *, void *);
void (*__mp_nomemory(void (*)(void)))(void);
void __mp_pushdelstack(char *, char *, unsigned long);
void __mp_popdelstack(char **, char **, unsigned long *);
int __mp_printf(char *, ...);
int __mp_vprintf(char *, va_list);
void __mp_printfwithloc(char *, char *, unsigned long, char *, ...);
void __mp_vprintfwithloc(char *, char *, unsigned long, char *, va_list);
void __mp_logmemory(void *, size_t);
int __mp_logstack(size_t);
int __mp_logaddr(void *);
int __mp_edit(char *, unsigned long);
int __mp_list(char *, unsigned long);
int __mp_view(char *, unsigned long);
void chkr_set_right(void *, size_t, unsigned char);
void chkr_copy_bitmap(void *, void *, size_t);
void chkr_check_addr(void *, size_t, unsigned char);
void chkr_check_str(char *, unsigned char);
void chkr_check_exec(void *);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* MP_INTER_H */
