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


#include "config.h"
#include "info.h"


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
    char *func;            /* calling function name */
    char *file;            /* file name in which call took place */
    unsigned long line;    /* line number at which call took place */
    addrnode *stack;       /* call stack details */
    char *typestr;         /* type stored in allocation */
    size_t typesize;       /* size of type stored in allocation */
    char freed;            /* allocation has been freed */
}
allocinfo;


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


void __mp_init(void);
void __mp_fini(void);
infohead *__mp_memhead(void);
void *__mp_alloc(size_t, size_t, alloctype, char *, char *, unsigned long,
                 size_t);
char *__mp_strdup(char *, size_t, alloctype, char *, char *, unsigned long,
                  size_t);
void *__mp_realloc(void *, size_t, size_t, alloctype, char *, char *,
                   unsigned long, size_t);
void __mp_free(void *, alloctype, char *, char *, unsigned long, size_t);
void *__mp_setmem(void *, size_t, unsigned char, alloctype, char *, char *,
                  unsigned long, size_t);
void *__mp_copymem(void *, void *, size_t, unsigned char, alloctype, char *,
                   char *, unsigned long, size_t);
void *__mp_locatemem(void *, size_t, void *, size_t, alloctype, char *, char *,
                     unsigned long, size_t);
int __mp_comparemem(void *, void *, size_t, alloctype, char *, char *,
                    unsigned long, size_t);
int __mp_info(void *, allocinfo *);
int __mp_printinfo(void *);
void __mp_memorymap(int);
void __mp_summary(void);
void __mp_check(void);
void (*__mp_prologue(void (*)(void *, size_t)))(void *, size_t);
void (*__mp_epilogue(void (*)(void *)))(void *);
void (*__mp_nomemory(void (*)(void)))(void);
void __mp_pushdelstack(char *, char *, unsigned long);
void __mp_popdelstack(char **, char **, unsigned long *);
void chkr_set_right(void *, size_t, unsigned char);
void chkr_copy_bitmap(void *, void *, size_t);
void chkr_check_addr(void *, size_t, unsigned char);
void chkr_check_str(char *, unsigned char);
void chkr_check_exec(void *);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* MP_INTER_H */
