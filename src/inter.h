#ifndef MP_INTER_H
#define MP_INTER_H


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
 * Library interface.  The module defines the visible interface for the
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
    char freed;            /* allocation has been freed */
}
allocinfo;


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


#if TARGET == TARGET_AMIGA

__asm void __mp_init(void);
__asm void __mp_fini(void);
__asm infohead *__mp_memhead(void);
__asm void *__mp_alloc(register __d0 size_t, register __d1 size_t,
                       register __d2 alloctype, register __a0 char *,
                       register __a1 char *, register __d3 unsigned long,
                       register __d4 size_t);
__asm char *__mp_strdup(register __a0 char *, register __d0 size_t,
                        register __d1 alloctype, register __a1 char *,
                        register __a2 char *, register __d2 unsigned long,
                        register __d3 size_t);
__asm void *__mp_realloc(register __a0 void *, register __d0 size_t,
                         register __d1 size_t, register __d2 alloctype,
                         register __a1 char *, register __a2 char *,
                         register __d3 unsigned long, register __d4 size_t);
__asm void __mp_free(register __a0 void *, register __d0 alloctype,
                     register __a1 char *, register __a2 char *,
                     register __d1 unsigned long, register __d2 size_t);
__asm void *__mp_setmem(register __a0 void *, register __d0 size_t,
                        register __d1 unsigned char, register __d2 alloctype,
                        register __a1 char *, register __a2 char *,
                        register __d3 unsigned long, register __d4 size_t);
__asm void *__mp_copymem(register __a0 void *, register __a1 void *,
                         register __d0 size_t, register __d1 alloctype,
                         register __a2 char *, register __a3 char *,
                         register __d2 unsigned long, register __d3 size_t);
__asm void *__mp_locatemem(register __a0 void *, register __d0 size_t,
                           register __a1 void *, register __d1 size_t,
                           register __d2 alloctype, register __a2 char *,
                           register __a3 char *, register __d3 unsigned long,
                           register __d4 size_t);
__asm int __mp_comparemem(register __a0 void *, register __a1 void *,
                          register __d0 size_t, register __d1 alloctype,
                          register __a2 char *, register __a3 char *,
                          register __d2 unsigned long, register __d3 size_t);
__asm int __mp_info(register __a0 void *, register __a1 allocinfo *);
__asm void __mp_memorymap(register __d0 int);
__asm void __mp_summary(void);
__asm void __mp_check(void);
__asm void (*__mp_prologue(register __a0 void (*)(void *, size_t)))
           (void *, size_t);
__asm void (*__mp_epilogue(register __a0 void (*)(void *)))(void *);
__asm void (*__mp_nomemory(register __a0 void (*)(void)))(void);
__asm void __mp_pushdelstack(register __a0 char *, register __a1 char *,
                             register __d0 unsigned long);
__asm void __mp_popdelstack(register __a0 char **, register __a1 char **,
                            register __a2 unsigned long *);

#else /* TARGET */

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
void *__mp_copymem(void *, void *, size_t, alloctype, char *, char *,
                   unsigned long, size_t);
void *__mp_locatemem(void *, size_t, void *, size_t, alloctype, char *, char *,
                     unsigned long, size_t);
int __mp_comparemem(void *, void *, size_t, alloctype, char *, char *,
                    unsigned long, size_t);
int __mp_info(void *, allocinfo *);
void __mp_memorymap(int);
void __mp_summary(void);
void __mp_check(void);
void (*__mp_prologue(void (*)(void *, size_t)))(void *, size_t);
void (*__mp_epilogue(void (*)(void *)))(void *);
void (*__mp_nomemory(void (*)(void)))(void);
void __mp_pushdelstack(char *, char *, unsigned long);
void __mp_popdelstack(char **, char **, unsigned long *);

#endif /* TARGET */


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* MP_INTER_H */
