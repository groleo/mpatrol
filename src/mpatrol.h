#ifndef MP_MPATROL_H
#define MP_MPATROL_H


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


#include <stdlib.h>
#include <string.h>


/* The version of the mpatrol library.  The version is of the format vrrff,
 * where v represents the version number, rr represents the revision number
 * and ff represents the bug fix count.
 */

#define MPATROL_VERSION 10100


/* A macro for determining the current function name.
 */

#ifndef MP_FUNCNAME
#ifdef __GNUC__
#define MP_FUNCNAME __PRETTY_FUNCTION__
#else /* __GNUC__ */
#define MP_FUNCNAME NULL
#endif /* __GNUC__ */
#endif /* MP_FUNCNAME */


/* Options for backwards compatibility with other versions of mallopt().  They
 * are all currently ignored as they have no meaning when used with mpatrol.
 */

#ifdef M_MXFAST
#undef M_MXFAST
#endif /* M_MXFAST */
#ifdef M_NLBLKS
#undef M_NLBLKS
#endif /* M_NLBLKS */
#ifdef M_GRAIN
#undef M_GRAIN
#endif /* M_GRAIN */
#ifdef M_KEEP
#undef M_KEEP
#endif /* M_KEEP */

#define M_MXFAST 1
#define M_NLBLKS 2
#define M_GRAIN  3
#define M_KEEP   4


/* Options that can be set using mallopt().  They all correspond to their
 * environment variable option equivalent except for MP_OPT_SETFLAGS and
 * MP_OPT_UNSETFLAGS.
 */

#define MP_OPT_HELP          0
#define MP_OPT_SETFLAGS      -1
#define MP_OPT_UNSETFLAGS    -2
#define MP_OPT_ALLOCSTOP     -3
#define MP_OPT_REALLOCSTOP   -4
#define MP_OPT_FREESTOP      -5
#define MP_OPT_ALLOCBYTE     -6
#define MP_OPT_FREEBYTE      -7
#define MP_OPT_OFLOWBYTE     -8
#define MP_OPT_OFLOWSIZE     -9
#define MP_OPT_DEFALIGN      -10
#define MP_OPT_LIMIT         -11
#define MP_OPT_FAILFREQ      -12
#define MP_OPT_FAILSEED      -13
#define MP_OPT_UNFREEDABORT  -14
#define MP_OPT_LOGFILE       -15
#define MP_OPT_PROGFILE      -16
#define MP_OPT_CHECK         -17


/* Flags that can be set or unset using mallopt() and MP_OPT_SETFLAGS or
 * MP_OPT_UNSETFLAGS respectively.  They all correspond to their environment
 * variable option equivalent except for MP_FLG_PAGEALLOC and MP_FLG_ALLOCUPPER.
 */

#define MP_FLG_SHOWALL       (MP_FLG_SHOWMAP | MP_FLG_SHOWSYMBOLS | \
                              MP_FLG_SHOWFREED | MP_FLG_SHOWUNFREED)
#define MP_FLG_SHOWMAP       0x00000001
#define MP_FLG_SHOWSYMBOLS   0x00000002
#define MP_FLG_SHOWFREED     0x00000004
#define MP_FLG_SHOWUNFREED   0x00000008
#define MP_FLG_LOGALL        (MP_FLG_LOGALLOCS | MP_FLG_LOGREALLOCS | \
                              MP_FLG_LOGFREES | MP_FLG_LOGMEMORY)
#define MP_FLG_LOGALLOCS     0x00000010
#define MP_FLG_LOGREALLOCS   0x00000020
#define MP_FLG_LOGFREES      0x00000040
#define MP_FLG_LOGMEMORY     0x00000080
#define MP_FLG_CHECKALL      (MP_FLG_CHECKALLOCS | MP_FLG_CHECKREALLOCS | \
                              MP_FLG_CHECKFREES)
#define MP_FLG_CHECKALLOCS   0x00000100
#define MP_FLG_CHECKREALLOCS 0x00000200
#define MP_FLG_CHECKFREES    0x00000400
#define MP_FLG_SAFESIGNALS   0x00000800
#define MP_FLG_NOPROTECT     0x00001000
#define MP_FLG_NOFREE        0x00002000
#define MP_FLG_PRESERVE      0x00004000
#define MP_FLG_OFLOWWATCH    0x00008000
#define MP_FLG_PAGEALLOC     0x00010000
#define MP_FLG_ALLOCUPPER    0x00020000
#define MP_FLG_USEMMAP       0x00040000


/* The different types of memory allocation and operation functions.
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
    MP_AT_EXPAND,    /* expand() */
    MP_AT_FREE,      /* free() */
    MP_AT_CFREE,     /* cfree() */
    MP_AT_NEW,       /* operator new */
    MP_AT_NEWVEC,    /* operator new[] */
    MP_AT_DELETE,    /* operator delete */
    MP_AT_DELETEVEC, /* operator delete[] */
    MP_AT_MEMSET,    /* memset() */
    MP_AT_BZERO,     /* bzero() */
    MP_AT_MEMCPY,    /* memcpy() */
    MP_AT_MEMMOVE,   /* memmove() */
    MP_AT_BCOPY,     /* bcopy() */
    MP_AT_MEMCHR,    /* memchr() */
    MP_AT_MEMMEM,    /* memmem() */
    MP_AT_MEMCMP,    /* memcmp() */
    MP_AT_BCMP,      /* bcmp() */
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
#ifdef expand
#undef expand
#endif /* expand */
#ifdef free
#undef free
#endif /* free */
#ifdef cfree
#undef cfree
#endif /* cfree */
#ifdef memset
#undef memset
#endif /* memset */
#ifdef bzero
#undef bzero
#endif /* bzero */
#ifdef memcpy
#undef memcpy
#endif /* memcpy */
#ifdef memmove
#undef memmove
#endif /* memmove */
#ifdef bcopy
#undef bcopy
#endif /* bcopy */
#ifdef memchr
#undef memchr
#endif /* memchr */
#ifdef memmem
#undef memmem
#endif /* memmem */
#ifdef memcmp
#undef memcmp
#endif /* memcmp */
#ifdef bcmp
#undef bcmp
#endif /* bcmp */


#define malloc(l) __mp_alloc((l), 0, MP_AT_MALLOC, MP_FUNCNAME, __FILE__, \
                             __LINE__, 0)
#define calloc(l, n) __mp_alloc((l) * (n), 0, MP_AT_CALLOC, MP_FUNCNAME, \
                                __FILE__, __LINE__, 0)
#define memalign(a, l) __mp_alloc((l), (a), MP_AT_MEMALIGN, MP_FUNCNAME, \
                                  __FILE__, __LINE__, 0)
#define valloc(l) __mp_alloc((l), 0, MP_AT_VALLOC, MP_FUNCNAME, __FILE__, \
                             __LINE__, 0)
#define pvalloc(l) __mp_alloc((l), 0, MP_AT_PVALLOC, MP_FUNCNAME, __FILE__, \
                              __LINE__, 0)
#define strdup(p) __mp_strdup((p), 0, MP_AT_STRDUP, MP_FUNCNAME, __FILE__, \
                              __LINE__, 0)
#define strndup(p, l) __mp_strdup((p), (l), MP_AT_STRNDUP, MP_FUNCNAME, \
                                  __FILE__, __LINE__, 0)
#define strsave(p) __mp_strdup((p), 0, MP_AT_STRSAVE, MP_FUNCNAME, __FILE__, \
                               __LINE__, 0)
#define strnsave(p, l) __mp_strdup((p), (l), MP_AT_STRNSAVE, MP_FUNCNAME, \
                                   __FILE__, __LINE__, 0)
#define realloc(p, l) __mp_realloc((p), (l), 0, MP_AT_REALLOC, MP_FUNCNAME, \
                                   __FILE__, __LINE__, 0)
#define recalloc(p, l) __mp_realloc((p), (l), 0, MP_AT_RECALLOC, MP_FUNCNAME, \
                                    __FILE__, __LINE__, 0)
#define expand(p, l) __mp_realloc((p), (l), 0, MP_AT_EXPAND, MP_FUNCNAME, \
                                  __FILE__, __LINE__, 0)
#define free(p) __mp_free((p), MP_AT_FREE, MP_FUNCNAME, __FILE__, __LINE__, 0)
#define cfree(p) __mp_free((p), MP_AT_CFREE, MP_FUNCNAME, __FILE__, __LINE__, 0)
#define memset(p, c, l) __mp_setmem((p), (l), (c), MP_AT_MEMSET, MP_FUNCNAME, \
                                    __FILE__, __LINE__, 0)
#define bzero(p, l) (void) __mp_setmem((p), (l), 0, MP_AT_BZERO, MP_FUNCNAME, \
                                       __FILE__, __LINE__, 0)
#define memcpy(q, p, l) __mp_copymem((p), (q), (l), MP_AT_MEMCPY, MP_FUNCNAME, \
                                     __FILE__, __LINE__, 0)
#define memmove(q, p, l) __mp_copymem((p), (q), (l), MP_AT_MEMMOVE, \
                                      MP_FUNCNAME, __FILE__, __LINE__, 0)
#define bcopy(p, q, l) (void) __mp_copymem((p), (q), (l), MP_AT_BCOPY, \
                                           MP_FUNCNAME, __FILE__, __LINE__, 0)
#define memchr(p, c, l) __mp_locatemem((p), (l), NULL, (c), MP_AT_MEMCHR, \
                                       MP_FUNCNAME, __FILE__, __LINE__, 0)
#define memmem(p, l, q, m) __mp_locatemem((p), (l), (q), (m), MP_AT_MEMMEM, \
                                          MP_FUNCNAME, __FILE__, __LINE__, 0)
#define memcmp(p, q, l) __mp_comparemem((p), (q), (l), MP_AT_MEMCMP, \
                                        MP_FUNCNAME, __FILE__, __LINE__, 0)
#define bcmp(p, q, l) __mp_comparemem((p), (q), (l), MP_AT_BCMP, MP_FUNCNAME, \
                                      __FILE__, __LINE__, 0)


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
__asm void *__mp_setmem(register __a0 void *, register __d0 size_t,
                        register __d1 unsigned char,
                        register __d2 __mp_alloctype, register __a1 char *,
                        register __a2 char *, register __d3 unsigned long,
                        register __d4 size_t);
__asm void *__mp_copymem(register __a0 void *, register __a1 void *,
                         register __d0 size_t, register __d1 __mp_alloctype,
                         register __a2 char *, register __a3 char *,
                         register __d2 unsigned long, register __d3 size_t);
__asm void *__mp_locatemem(register __a0 void *, register __d0 size_t,
                           register __a1 void *, register __d1 size_t,
                           register __d2 __mp_alloctype, register __a2 char *,
                           register __a3 char *, register __d3 unsigned long,
                           register __d4 size_t);
__asm int __mp_comparemem(register __a0 void *, register __a1 void *,
                          register __d0 size_t, register __d1 __mp_alloctype,
                          register __a2 char *, register __a3 char *,
                          register __d2 unsigned long, register __d3 size_t);
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
void *__mp_setmem(void *, size_t, unsigned char, __mp_alloctype, char *, char *,
                  unsigned long, size_t);
void *__mp_copymem(void *, void *, size_t, __mp_alloctype, char *, char *,
                   unsigned long, size_t);
void *__mp_locatemem(void *, size_t, void *, size_t, __mp_alloctype, char *,
                     char *, unsigned long, size_t);
int __mp_comparemem(void *, void *, size_t, __mp_alloctype, char *, char *,
                    unsigned long, size_t);
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

static inline void *operator new(size_t l, char *s, char *t, unsigned long u)
{
    return __mp_alloc(l, 0, MP_AT_NEW, s, t, u, 0);
}


/* Override operator new[].
 */

static inline void *operator new[](size_t l, char *s, char *t, unsigned long u)
{
    return __mp_alloc(l, 0, MP_AT_NEWVEC, s, t, u, 0);
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


#define new ::new(MP_FUNCNAME, __FILE__, __LINE__)

#endif /* __cplusplus */


#endif /* MP_MPATROL_H */
