#ifndef MP_MPATROL_H
#define MP_MPATROL_H


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


#include <stdlib.h>
#include <string.h>


/* The version of the mpatrol library.  The version is of the format vrrff,
 * where v represents the version number, rr represents the revision number
 * and ff represents the bug fix count.
 */

#define MPATROL_VERSION 10005


/* The different types of memory allocation functions.
 */

typedef enum __mp_alloctype
{
    MP_AT_MALLOC,    /* malloc() */
    MP_AT_CALLOC,    /* calloc() */
    MP_AT_MEMALIGN,  /* memalign() */
    MP_AT_VALLOC,    /* valloc() */
    MP_AT_PVALLOC,   /* pvalloc() */
    MP_AT_STRDUP,    /* strdup() */
    MP_AT_STRNDUP,   /* strndup() */
    MP_AT_STRSAVE,   /* strsave() */
    MP_AT_STRNSAVE,  /* strnsave() */
    MP_AT_REALLOC,   /* realloc() */
    MP_AT_RECALLOC,  /* recalloc() */
    MP_AT_FREE,      /* free() */
    MP_AT_CFREE,     /* cfree() */
    MP_AT_NEW,       /* operator new */
    MP_AT_NEWVEC,    /* operator new[] */
    MP_AT_DELETE,    /* operator delete */
    MP_AT_DELETEVEC, /* operator delete[] */
    MP_AT_MAX
}
__mp_alloctype;


/* The details of a single function in a call stack.
 */

typedef struct __mp_allocstack
{
    struct __mp_allocstack *next; /* next address node in call stack */
    char *name;                   /* name of function */
    void *addr;                   /* return address in function */
}
__mp_allocstack;


/* The details of a single memory allocation.
 */

typedef struct __mp_allocinfo
{
    void *block;            /* pointer to block of memory */
    size_t size;            /* size of block of memory */
    __mp_alloctype type;    /* type of memory allocation */
    unsigned long alloc;    /* allocation index */
    unsigned long realloc;  /* reallocation index */
    unsigned long thread;   /* thread identifier */
    char *func;             /* calling function name */
    char *file;             /* file name in which call took place */
    unsigned long line;     /* line number at which call took place */
    __mp_allocstack *stack; /* call stack details */
    char freed;             /* allocation has been freed */
}
__mp_allocinfo;


#ifdef malloc
#undef malloc
#endif /* malloc */
#ifdef calloc
#undef calloc
#endif /* calloc */
#ifdef memalign
#undef memalign
#endif /* memalign */
#ifdef valloc
#undef valloc
#endif /* valloc */
#ifdef pvalloc
#undef pvalloc
#endif /* pvalloc */
#ifdef strdup
#undef strdup
#endif /* strdup */
#ifdef strndup
#undef strndup
#endif /* strndup */
#ifdef strsave
#undef strsave
#endif /* strsave */
#ifdef strnsave
#undef strnsave
#endif /* strnsave */
#ifdef realloc
#undef realloc
#endif /* realloc */
#ifdef recalloc
#undef recalloc
#endif /* recalloc */
#ifdef free
#undef free
#endif /* free */
#ifdef cfree
#undef cfree
#endif /* cfree */


#ifdef __GNUC__

#define malloc(l) __mp_alloc((l), 0, MP_AT_MALLOC, __PRETTY_FUNCTION__, \
                             __FILE__, __LINE__, 0)
#define calloc(l, n) __mp_alloc((l) * (n), 0, MP_AT_CALLOC, \
                                __PRETTY_FUNCTION__, __FILE__, __LINE__, 0)
#define memalign(a, l) __mp_alloc((l), (a), MP_AT_MEMALIGN, \
                                  __PRETTY_FUNCTION__, __FILE__, __LINE__, 0)
#define valloc(l) __mp_alloc((l), 0, MP_AT_VALLOC, __PRETTY_FUNCTION__, \
                             __FILE__, __LINE__, 0)
#define pvalloc(l) __mp_alloc((l), 0, MP_AT_PVALLOC, __PRETTY_FUNCTION__, \
                              __FILE__, __LINE__, 0)
#define strdup(p) __mp_strdup((p), 0, MP_AT_STRDUP, __PRETTY_FUNCTION__, \
                              __FILE__, __LINE__, 0)
#define strndup(p, l) __mp_strdup((p), (l), MP_AT_STRNDUP, \
                                  __PRETTY_FUNCTION__, __FILE__, __LINE__, 0)
#define strsave(p) __mp_strdup((p), 0, MP_AT_STRSAVE, __PRETTY_FUNCTION__, \
                               __FILE__, __LINE__, 0)
#define strnsave(p, l) __mp_strdup((p), (l), MP_AT_STRNSAVE, \
                                   __PRETTY_FUNCTION__, __FILE__, __LINE__, 0)
#define realloc(p, l) __mp_realloc((p), (l), 0, MP_AT_REALLOC, \
                                   __PRETTY_FUNCTION__, __FILE__, __LINE__, 0)
#define recalloc(p, l) __mp_realloc((p), (l), 0, MP_AT_RECALLOC, \
                                    __PRETTY_FUNCTION__, __FILE__, __LINE__, 0)
#define free(p) __mp_free((p), MP_AT_FREE, __PRETTY_FUNCTION__, __FILE__, \
                          __LINE__, 0)
#define cfree(p) __mp_free((p), MP_AT_CFREE, __PRETTY_FUNCTION__, __FILE__, \
                           __LINE__, 0)

#else /* __GNUC__ */

#define malloc(l) __mp_alloc((l), 0, MP_AT_MALLOC, NULL, __FILE__, __LINE__, \
                             0)
#define calloc(l, n) __mp_alloc((l) * (n), 0, MP_AT_CALLOC, NULL, __FILE__, \
                                __LINE__, 0)
#define memalign(a, l) __mp_alloc((l), (a), MP_AT_MEMALIGN, NULL, __FILE__, \
                                  __LINE__, 0)
#define valloc(l) __mp_alloc((l), 0, MP_AT_VALLOC, NULL, __FILE__, __LINE__, \
                             0)
#define pvalloc(l) __mp_alloc((l), 0, MP_AT_PVALLOC, NULL, __FILE__, __LINE__, \
                              0)
#define strdup(p) __mp_strdup((p), 0, MP_AT_STRDUP, NULL, __FILE__, __LINE__, 0)
#define strndup(p, l) __mp_strdup((p), (l), MP_AT_STRNDUP, NULL, __FILE__, \
                                  __LINE__, 0)
#define strsave(p) __mp_strdup((p), 0, MP_AT_STRSAVE, NULL, __FILE__, \
                               __LINE__, 0)
#define strnsave(p, l) __mp_strdup((p), (l), MP_AT_STRNSAVE, NULL, __FILE__, \
                                   __LINE__, 0)
#define realloc(p, l) __mp_realloc((p), (l), 0, MP_AT_REALLOC, NULL, __FILE__, \
                                   __LINE__, 0)
#define recalloc(p, l) __mp_realloc((p), (l), 0, MP_AT_RECALLOC, NULL, \
                                    __FILE__, __LINE__, 0)
#define free(p) __mp_free((p), MP_AT_FREE, NULL, __FILE__, __LINE__, 0)
#define cfree(p) __mp_free((p), MP_AT_CFREE, NULL, __FILE__, __LINE__, 0)

#endif /* __GNUC__ */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


#ifdef AMIGA

__asm void __mp_init(void);
__asm void __mp_fini(void);
__asm void *__mp_alloc(register __d0 size_t, register __d1 size_t,
                       register __d2 __mp_alloctype, register __a0 char *,
                       register __a1 char *, register __d3 unsigned long,
                       register __d4 size_t);
__asm char *__mp_strdup(register __a0 char *, register __d0 size_t,
                        register __d1 __mp_alloctype, register __a1 char *,
                        register __a2 char *, register __d2 unsigned long,
                        register __d3 size_t);
__asm void *__mp_realloc(register __a0 void *, register __d0 size_t,
                         register __d1 size_t, register __d2 __mp_alloctype,
                         register __a1 char *, register __a2 char *,
                         register __d3 unsigned long, register __d4 size_t);
__asm void __mp_free(register __a0 void *, register __d0 __mp_alloctype,
                     register __a1 char *, register __a2 char *,
                     register __d1 unsigned long, register __d2 size_t);
__asm int __mp_info(register __a0 void *, register __a1 __mp_allocinfo *);
__asm void __mp_memorymap(register __d0 int);
__asm void __mp_summary(void);
__asm void __mp_check(void);
__asm void (*__mp_prologue(register __a0 void (*)(void *, size_t)))
           (void *, size_t);
__asm void (*__mp_epilogue(register __a0 void (*)(void *)))(void *);
__asm void (*__mp_nomemory(register __a0 void (*)(void)))(void);

#else /* AMIGA */

void __mp_init(void);
void __mp_fini(void);
void *__mp_alloc(size_t, size_t, __mp_alloctype, char *, char *, unsigned long,
                 size_t);
char *__mp_strdup(char *, size_t, __mp_alloctype, char *, char *, unsigned long,
                  size_t);
void *__mp_realloc(void *, size_t, size_t, __mp_alloctype, char *, char *,
                   unsigned long, size_t);
void __mp_free(void *, __mp_alloctype, char *, char *, unsigned long, size_t);
int __mp_info(void *, __mp_allocinfo *);
void __mp_memorymap(int);
void __mp_summary(void);
void __mp_check(void);
void (*__mp_prologue(void (*)(void *, size_t)))(void *, size_t);
void (*__mp_epilogue(void (*)(void *)))(void *);
void (*__mp_nomemory(void (*)(void)))(void);

#endif /* AMIGA */


#ifdef __cplusplus
}
#endif /* __cplusplus */


#ifdef __cplusplus

typedef void (*new_handler)(void);


/* Set the low-memory handler.
 */

static inline new_handler set_new_handler(new_handler h)
{
    return __mp_nomemory(h);
}


/* Override operator new.
 */

static inline void *operator new(size_t l)
{
    return __mp_alloc(l, 0, MP_AT_NEW, NULL, NULL, 0, 0);
}


/* Override operator new[].
 */

static inline void *operator new[](size_t l)
{
    return __mp_alloc(l, 0, MP_AT_NEWVEC, NULL, NULL, 0, 0);
}


/* Override operator delete.
 */

static inline void operator delete(void *p)
{
    __mp_free(p, MP_AT_DELETE, NULL, NULL, 0, 0);
}


/* Override operator delete[].
 */

static inline void operator delete[](void *p)
{
    __mp_free(p, MP_AT_DELETEVEC, NULL, NULL, 0, 0);
}

#endif /* __cplusplus */


#endif /* MP_MPATROL_H */
