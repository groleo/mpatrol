#ifndef MP_MPATROL_H
#define MP_MPATROL_H


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
 * $Id: mpatrol.h,v 1.92 2001-02-22 19:46:36 graeme Exp $
 */


#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#if !MP_NOCPLUSPLUS
#ifdef __cplusplus
#include <new>
#endif /* __cplusplus */
#endif /* MP_NOCPLUSPLUS */
#ifdef NDEBUG
#include <mpalloc.h>
#endif /* NDEBUG */


/* The version of the mpatrol library.  The version is of the format vrrff,
 * where v represents the version number, rr represents the revision number
 * and ff represents the bug fix count.
 */

#define MPATROL_VERSION 10400


/* A macro for representing constant function parameters.
 */

#ifndef MP_CONST
#if defined(__STDC__) || defined(__cplusplus)
#define MP_CONST const
#else /* __STDC__ && __cplusplus */
#define MP_CONST
#endif /* __STDC__ && __cplusplus */
#endif /* MP_CONST */


/* A macro for determining the alignment of a type at compile-time.
 * This resolves to 0 if the compiler has no mechanism for doing this.
 */

#ifndef MP_ALIGN
#ifdef __EDG__
#define MP_ALIGN(t) __ALIGNOF__(t)
#elif defined(__GNUC__)
#define MP_ALIGN(t) __alignof__(t)
#else /* __EDG__ && __GNUC__ */
#define MP_ALIGN(t) 0
#endif /* __EDG__ && __GNUC__ */
#endif /* MP_ALIGN */


/* A macro for determining the current function name.
 */

#ifndef MP_FUNCNAME
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ == 199901L)
#define MP_FUNCNAME __func__
#elif defined(__GNUC__)
#define MP_FUNCNAME __PRETTY_FUNCTION__
#else /* __STDC_VERSION__ && __GNUC__ */
#define MP_FUNCNAME NULL
#endif /* __STDC_VERSION__ && __GNUC__ */
#endif /* MP_FUNCNAME */


/* A macro for defining the visibility of the inline C++ operators.  This
 * should be extern inline so that there is no non-inline definition, but
 * most compilers do not support this concept yet.
 */

#ifndef MP_INLINE
#ifdef __GNUC__
#define MP_INLINE extern inline
#else /* __GNUC__ */
#define MP_INLINE static inline
#endif /* __GNUC__ */
#endif /* MP_INLINE */


/* A macro for disabling the definition of replacement C++ operators.
 */

#ifndef MP_NOCPLUSPLUS
#define MP_NOCPLUSPLUS 0
#endif /* MP_NOCPLUSPLUS */


/* A macro for requiring the use of MP_NEW and MP_DELETE instead of new
 * and delete in order to use the mpatrol versions of the C++ operators.
 */

#ifndef MP_NONEWDELETE
#define MP_NONEWDELETE 0
#endif /* MP_NONEWDELETE */


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
#ifdef M_TRIM_THRESHOLD
#undef M_TRIM_THRESHOLD
#endif /* M_TRIM_THRESHOLD */
#ifdef M_TOP_PAD
#undef M_TOP_PAD
#endif /* M_TOP_PAD */
#ifdef M_MMAP_THRESHOLD
#undef M_MMAP_THRESHOLD
#endif /* M_MMAP_THRESHOLD */
#ifdef M_MMAP_MAX
#undef M_MMAP_MAX
#endif /* M_MMAP_MAX */
#ifdef M_CHECK_ACTION
#undef M_CHECK_ACTION
#endif /* M_CHECK_ACTION */

#define M_MXFAST         1
#define M_NLBLKS         2
#define M_GRAIN          3
#define M_KEEP           4
#define M_TRIM_THRESHOLD 5
#define M_TOP_PAD        6
#define M_MMAP_THRESHOLD 7
#define M_MMAP_MAX       8
#define M_CHECK_ACTION   9


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
#define MP_OPT_PROFFILE      -16
#define MP_OPT_TRACEFILE     -17
#define MP_OPT_PROGFILE      -18
#define MP_OPT_AUTOSAVE      -19
#define MP_OPT_CHECKLOWER    -20
#define MP_OPT_CHECKUPPER    -21
#define MP_OPT_CHECKFREQ     -22
#define MP_OPT_NOFREE        -23
#define MP_OPT_SMALLBOUND    -24
#define MP_OPT_MEDIUMBOUND   -25
#define MP_OPT_LARGEBOUND    -26


/* Flags that can be set or unset using mallopt() and MP_OPT_SETFLAGS or
 * MP_OPT_UNSETFLAGS respectively.  They all correspond to their environment
 * variable option equivalent except for MP_FLG_PAGEALLOC and MP_FLG_ALLOCUPPER.
 */

#define MP_FLG_CHECKALL      (MP_FLG_CHECKALLOCS | MP_FLG_CHECKREALLOCS | \
                              MP_FLG_CHECKFREES | MP_FLG_CHECKMEMORY)
#define MP_FLG_CHECKALLOCS   0x00000001
#define MP_FLG_CHECKREALLOCS 0x00000002
#define MP_FLG_CHECKFREES    0x00000004
#define MP_FLG_CHECKMEMORY   0x00000008
#define MP_FLG_LOGALL        (MP_FLG_LOGALLOCS | MP_FLG_LOGREALLOCS | \
                              MP_FLG_LOGFREES | MP_FLG_LOGMEMORY)
#define MP_FLG_LOGALLOCS     0x00000010
#define MP_FLG_LOGREALLOCS   0x00000020
#define MP_FLG_LOGFREES      0x00000040
#define MP_FLG_LOGMEMORY     0x00000080
#define MP_FLG_SHOWALL       (MP_FLG_SHOWMAP | MP_FLG_SHOWSYMBOLS | \
                              MP_FLG_SHOWFREE | MP_FLG_SHOWFREED | \
                              MP_FLG_SHOWUNFREED)
#define MP_FLG_SHOWMAP       0x00000100
#define MP_FLG_SHOWSYMBOLS   0x00000200
#define MP_FLG_SHOWFREE      0x00000400
#define MP_FLG_SHOWFREED     0x00000800
#define MP_FLG_SHOWUNFREED   0x00001000
#define MP_FLG_ALLOWOFLOW    0x00002000
#define MP_FLG_PROF          0x00004000
#define MP_FLG_TRACE         0x00008000
#define MP_FLG_SAFESIGNALS   0x00010000
#define MP_FLG_NOPROTECT     0x00020000
#define MP_FLG_PRESERVE      0x00040000
#define MP_FLG_OFLOWWATCH    0x00080000
#define MP_FLG_PAGEALLOC     0x00100000
#define MP_FLG_ALLOCUPPER    0x00200000
#define MP_FLG_USEMMAP       0x00400000
#define MP_FLG_USEDEBUG      0x00800000
#define MP_FLG_EDIT          0x01000000
#define MP_FLG_LIST          0x02000000


#ifndef MP_MPALLOC_H
/* The type of the allocation failure handler.  This is only defined if
 * mpalloc.h has not already been included.
 */

typedef void *__mp_failhandler;
#endif /* MP_MPALLOC_H */


/* The different types of memory allocation and operation functions.
 */

typedef enum __mp_alloctype
{
    MP_AT_MALLOC,    /* malloc() */
    MP_AT_CALLOC,    /* calloc() */
    MP_AT_MEMALIGN,  /* memalign() */
    MP_AT_VALLOC,    /* valloc() */
    MP_AT_PVALLOC,   /* pvalloc() */
    MP_AT_ALLOCA,    /* alloca() */
    MP_AT_STRDUP,    /* strdup() */
    MP_AT_STRNDUP,   /* strndup() */
    MP_AT_STRSAVE,   /* strsave() */
    MP_AT_STRNSAVE,  /* strnsave() */
    MP_AT_STRDUPA,   /* strdupa() */
    MP_AT_STRNDUPA,  /* strndupa() */
    MP_AT_REALLOC,   /* realloc() */
    MP_AT_REALLOCF,  /* reallocf() */
    MP_AT_RECALLOC,  /* recalloc() */
    MP_AT_EXPAND,    /* expand() */
    MP_AT_FREE,      /* free() */
    MP_AT_CFREE,     /* cfree() */
    MP_AT_DEALLOCA,  /* dealloca() */
    MP_AT_XMALLOC,   /* xmalloc() */
    MP_AT_XCALLOC,   /* xcalloc() */
    MP_AT_XSTRDUP,   /* xstrdup() */
    MP_AT_XREALLOC,  /* xrealloc() */
    MP_AT_XFREE,     /* xfree() */
    MP_AT_NEW,       /* operator new */
    MP_AT_NEWVEC,    /* operator new[] */
    MP_AT_DELETE,    /* operator delete */
    MP_AT_DELETEVEC, /* operator delete[] */
    MP_AT_MEMSET,    /* memset() */
    MP_AT_BZERO,     /* bzero() */
    MP_AT_MEMCCPY,   /* memccpy() */
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
    unsigned long event;    /* event of last modification */
    char *func;             /* calling function name */
    char *file;             /* file name in which call took place */
    unsigned long line;     /* line number at which call took place */
    __mp_allocstack *stack; /* call stack details */
    char *typestr;          /* type stored in allocation */
    size_t typesize;        /* size of type stored in allocation */
    void *userdata;         /* user data associated with allocation */
    char freed;             /* allocation has been freed */
}
__mp_allocinfo;


/* The details of a particular function symbol.
 */

typedef struct __mp_symbolinfo
{
    char *name;         /* symbol name */
    char *object;       /* module symbol located in */
    void *addr;         /* start address */
    size_t size;        /* size of symbol */
    char *file;         /* file name corresponding to address */
    unsigned long line; /* line number corresponding to address */
}
__mp_symbolinfo;


/* The details of the current heap state.
 */

typedef struct __mp_heapinfo
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
__mp_heapinfo;


/* The structure filled by mallinfo().
 */

struct mallinfo
{
    unsigned long arena;    /* total space in arena */
    unsigned long ordblks;  /* number of ordinary blocks */
    unsigned long smblks;   /* number of small blocks */
    unsigned long hblks;    /* number of holding blocks */
    unsigned long hblkhd;   /* space in holding block headers */
    unsigned long usmblks;  /* space in small blocks in use */
    unsigned long fsmblks;  /* space in free small blocks */
    unsigned long uordblks; /* space in ordinary blocks in use */
    unsigned long fordblks; /* space in free ordinary blocks */
    unsigned long keepcost; /* cost of enabling keep option */
};


#ifndef NDEBUG

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
#ifdef alloca
#undef alloca
#endif /* alloca */
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
#ifdef strdupa
#undef strdupa
#endif /* strdupa */
#ifdef strndupa
#undef strndupa
#endif /* strndupa */
#ifdef realloc
#undef realloc
#endif /* realloc */
#ifdef reallocf
#undef reallocf
#endif /* reallocf */
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
#ifdef dealloca
#undef dealloca
#endif /* dealloca */
#ifdef xmalloc
#undef xmalloc
#endif /* xmalloc */
#ifdef xcalloc
#undef xcalloc
#endif /* xcalloc */
#ifdef xstrdup
#undef xstrdup
#endif /* xstrdup */
#ifdef xrealloc
#undef xrealloc
#endif /* xrealloc */
#ifdef xfree
#undef xfree
#endif /* xfree */

#ifdef memset
#undef memset
#endif /* memset */
#ifdef bzero
#undef bzero
#endif /* bzero */
#ifdef memccpy
#undef memccpy
#endif /* memccpy */
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

#ifdef MP_MALLOC
#undef MP_MALLOC
#endif /* MP_MALLOC */
#ifdef MP_CALLOC
#undef MP_CALLOC
#endif /* MP_CALLOC */
#ifdef MP_STRDUP
#undef MP_STRDUP
#endif /* MP_STRDUP */
#ifdef MP_REALLOC
#undef MP_REALLOC
#endif /* MP_REALLOC */
#ifdef MP_FREE
#undef MP_FREE
#endif /* MP_FREE */
#ifdef MP_FAILURE
#undef MP_FAILURE
#endif /* MP_FAILURE */

#if !MP_NOCPLUSPLUS
#ifdef __cplusplus
#if MP_NONEWDELETE
#ifdef MP_NEW
#undef MP_NEW
#endif /* MP_NEW */
#ifdef MP_NEW_NOTHROW
#undef MP_NEW_NOTHROW
#endif /* MP_NEW_NOTHROW */
#ifdef MP_DELETE
#undef MP_DELETE
#endif /* MP_DELETE */
#else /* MP_NONEWDELETE */
#ifdef new
#undef new
#endif /* new */
#ifdef delete
#undef delete
#endif /* delete */
#endif /* MP_NONEWDELETE */
#endif /* __cplusplus */
#endif /* MP_NOCPLUSPLUS */


#define malloc(l) \
    __mp_alloc((l), 0, MP_AT_MALLOC, MP_FUNCNAME, __FILE__, __LINE__, NULL, 0, \
               0)
#define calloc(l, n) \
    __mp_alloc((l) * (n), 0, MP_AT_CALLOC, MP_FUNCNAME, __FILE__, __LINE__, \
               NULL, 0, 0)
#define memalign(a, l) \
    __mp_alloc((l), (a), MP_AT_MEMALIGN, MP_FUNCNAME, __FILE__, __LINE__, \
               NULL, 0, 0)
#define valloc(l) \
    __mp_alloc((l), 0, MP_AT_VALLOC, MP_FUNCNAME, __FILE__, __LINE__, NULL, 0, \
               0)
#define pvalloc(l) \
    __mp_alloc((l), 0, MP_AT_PVALLOC, MP_FUNCNAME, __FILE__, __LINE__, NULL, \
               0, 0)
#define alloca(l) \
    __mp_alloc((l), 0, MP_AT_ALLOCA, MP_FUNCNAME, __FILE__, __LINE__, NULL, 0, \
               0)
#define strdup(p) \
    __mp_strdup((p), 0, MP_AT_STRDUP, MP_FUNCNAME, __FILE__, __LINE__, 0)
#define strndup(p, l) \
    __mp_strdup((p), (l), MP_AT_STRNDUP, MP_FUNCNAME, __FILE__, __LINE__, 0)
#define strsave(p) \
    __mp_strdup((p), 0, MP_AT_STRSAVE, MP_FUNCNAME, __FILE__, __LINE__, 0)
#define strnsave(p, l) \
    __mp_strdup((p), (l), MP_AT_STRNSAVE, MP_FUNCNAME, __FILE__, __LINE__, 0)
#define strdupa(p) \
    __mp_strdup((p), 0, MP_AT_STRDUPA, MP_FUNCNAME, __FILE__, __LINE__, 0)
#define strndupa(p, l) \
    __mp_strdup((p), (l), MP_AT_STRNDUPA, MP_FUNCNAME, __FILE__, __LINE__, 0)
#define realloc(p, l) \
    __mp_realloc((p), (l), 0, MP_AT_REALLOC, MP_FUNCNAME, __FILE__, __LINE__, \
                 NULL, 0, 0)
#define reallocf(p, l) \
    __mp_realloc((p), (l), 0, MP_AT_REALLOCF, MP_FUNCNAME, __FILE__, __LINE__, \
                 NULL, 0, 0)
#define recalloc(p, l, n) \
    __mp_realloc((p), (l) * (n), 0, MP_AT_RECALLOC, MP_FUNCNAME, __FILE__, \
                 __LINE__, NULL, 0, 0)
#define expand(p, l) \
    __mp_realloc((p), (l), 0, MP_AT_EXPAND, MP_FUNCNAME, __FILE__, __LINE__, \
                 NULL, 0, 0)
#define free(p) \
    __mp_free((p), MP_AT_FREE, MP_FUNCNAME, __FILE__, __LINE__, 0)
#define cfree(p, l, n) \
    __mp_free((p), MP_AT_CFREE, MP_FUNCNAME, __FILE__, __LINE__, 0)
#define dealloca(p) \
    __mp_free((p), MP_AT_DEALLOCA, MP_FUNCNAME, __FILE__, __LINE__, 0)
#define xmalloc(l) \
    __mp_alloc((l), 0, MP_AT_XMALLOC, MP_FUNCNAME, __FILE__, __LINE__, NULL, \
               0, 0)
#define xcalloc(l, n) \
    __mp_alloc((l) * (n), 0, MP_AT_XCALLOC, MP_FUNCNAME, __FILE__, __LINE__, \
               NULL, 0, 0)
#define xstrdup(p) \
    __mp_strdup((p), 0, MP_AT_XSTRDUP, MP_FUNCNAME, __FILE__, __LINE__, 0)
#define xrealloc(p, l) \
    __mp_realloc((p), (l), 0, MP_AT_XREALLOC, MP_FUNCNAME, __FILE__, __LINE__, \
                 NULL, 0, 0)
#define xfree(p) \
    __mp_free((p), MP_AT_XFREE, MP_FUNCNAME, __FILE__, __LINE__, 0)

#define memset(p, c, l) \
    __mp_setmem((p), (l), (unsigned char) (c), MP_AT_MEMSET, MP_FUNCNAME, \
                __FILE__, __LINE__, 0)
#define bzero(p, l) \
    (void) __mp_setmem((p), (l), 0, MP_AT_BZERO, MP_FUNCNAME, __FILE__, \
                       __LINE__, 0)
#define memccpy(q, p, c, l) \
    __mp_copymem((p), (q), (l), (unsigned char) (c), MP_AT_MEMCCPY, \
                 MP_FUNCNAME, __FILE__, __LINE__, 0)
#define memcpy(q, p, l) \
    __mp_copymem((p), (q), (l), 0, MP_AT_MEMCPY, MP_FUNCNAME, __FILE__, \
                 __LINE__, 0)
#define memmove(q, p, l) \
    __mp_copymem((p), (q), (l), 0, MP_AT_MEMMOVE, MP_FUNCNAME, __FILE__, \
                 __LINE__, 0)
#define bcopy(p, q, l) \
    (void) __mp_copymem((p), (q), (l), 0, MP_AT_BCOPY, MP_FUNCNAME, __FILE__, \
                        __LINE__, 0)
#define memchr(p, c, l) \
    __mp_locatemem((p), (l), NULL, (size_t) (c), MP_AT_MEMCHR, MP_FUNCNAME, \
                   __FILE__, __LINE__, 0)
#define memmem(p, l, q, m) \
    __mp_locatemem((p), (l), (q), (m), MP_AT_MEMMEM, MP_FUNCNAME, __FILE__, \
                   __LINE__, 0)
#define memcmp(p, q, l) \
    __mp_comparemem((p), (q), (l), MP_AT_MEMCMP, MP_FUNCNAME, __FILE__, \
                    __LINE__, 0)
#define bcmp(p, q, l) \
    __mp_comparemem((p), (q), (l), MP_AT_BCMP, MP_FUNCNAME, __FILE__, \
                    __LINE__, 0)

#define MP_MALLOC(p, l, t) \
    (p = (t *) __mp_alloc((l) * sizeof(t), MP_ALIGN(t), MP_AT_XMALLOC, \
                          MP_FUNCNAME, __FILE__, __LINE__, #t, sizeof(t), 0))
#define MP_CALLOC(p, l, t) \
    (p = (t *) __mp_alloc((l) * sizeof(t), MP_ALIGN(t), MP_AT_XCALLOC, \
                          MP_FUNCNAME, __FILE__, __LINE__, #t, sizeof(t), 0))
#define MP_STRDUP(p, s) \
    (p = __mp_strdup((s), 0, MP_AT_XSTRDUP, MP_FUNCNAME, __FILE__, __LINE__, 0))
#define MP_REALLOC(p, l, t) \
    (p = (t *) __mp_realloc((p), (l) * sizeof(t), MP_ALIGN(t), MP_AT_XREALLOC, \
                            MP_FUNCNAME, __FILE__, __LINE__, #t, sizeof(t), 0))
#define MP_FREE(p) \
    do { __mp_free((p), MP_AT_XFREE, MP_FUNCNAME, __FILE__, __LINE__, 0); \
         p = NULL; } while (0)
#define MP_FAILURE(f) ((__mp_failhandler) NULL)


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


void __mp_init(void);
void __mp_fini(void);
unsigned long __mp_setoption(long, unsigned long);
int __mp_getoption(long, unsigned long *);
void *__mp_alloc(size_t, size_t, __mp_alloctype, MP_CONST char *,
                 MP_CONST char *, unsigned long, MP_CONST char *, size_t,
                 size_t);
char *__mp_strdup(MP_CONST char *, size_t, __mp_alloctype, MP_CONST char *,
                  MP_CONST char *, unsigned long, size_t);
void *__mp_realloc(void *, size_t, size_t, __mp_alloctype, MP_CONST char *,
                   MP_CONST char *, unsigned long, MP_CONST char *, size_t,
                   size_t);
void __mp_free(void *, __mp_alloctype, MP_CONST char *, MP_CONST char *,
               unsigned long, size_t);
void *__mp_setmem(void *, size_t, unsigned char, __mp_alloctype,
                  MP_CONST char *, MP_CONST char *, unsigned long, size_t);
void *__mp_copymem(MP_CONST void *, void *, size_t, unsigned char,
                   __mp_alloctype, MP_CONST char *, MP_CONST char *,
                   unsigned long, size_t);
void *__mp_locatemem(MP_CONST void *, size_t, MP_CONST void *, size_t,
                     __mp_alloctype, MP_CONST char *, MP_CONST char *,
                     unsigned long, size_t);
int __mp_comparemem(MP_CONST void *, MP_CONST void *, size_t, __mp_alloctype,
                    MP_CONST char *, MP_CONST char *, unsigned long, size_t);
char *__mp_function(__mp_alloctype);
int __mp_setuser(MP_CONST void *, MP_CONST void *);
int __mp_info(MP_CONST void *, __mp_allocinfo *);
int __mp_syminfo(MP_CONST void *, __mp_symbolinfo *);
int __mp_printinfo(MP_CONST void *);
unsigned long __mp_snapshot(void);
size_t __mp_iterate(int (*)(MP_CONST void *, void *), void *, unsigned long);
void __mp_memorymap(int);
void __mp_summary(void);
int __mp_stats(__mp_heapinfo *);
void __mp_check(void);
void (*__mp_prologue(void (*)(MP_CONST void *, size_t, MP_CONST void *)))
     (MP_CONST void *, size_t, MP_CONST void *);
void (*__mp_epilogue(void (*)(MP_CONST void *, MP_CONST void *)))
     (MP_CONST void *, MP_CONST void *);
void (*__mp_nomemory(void (*)(void)))(void);
void __mp_pushdelstack(MP_CONST char *, MP_CONST char *, unsigned long);
void __mp_popdelstack(char **, char **, unsigned long *);
int __mp_printf(MP_CONST char *, ...);
int __mp_vprintf(MP_CONST char *, va_list);
void __mp_logmemory(MP_CONST void *, size_t);
int __mp_logstack(size_t);
int __mp_logaddr(MP_CONST void *);
int __mp_edit(MP_CONST char *, unsigned long);
int __mp_list(MP_CONST char *, unsigned long);
int __mp_view(MP_CONST char *, unsigned long);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#else /* NDEBUG */

#define dealloca(p)

#define MP_NEW new
#define MP_NEW_NOTHROW new(std::nothrow)
#define MP_DELETE delete

#define __mp_init() ((void) 0)
#define __mp_fini() ((void) 0)
#define __mp_setoption(o, v) ((unsigned long) ~0L)
#define __mp_getoption(o, v) ((int) 0)
#define __mp_alloc(l, a, f, s, t, u, g, h, k) ((void *) NULL)
#define __mp_strdup(p, l, f, s, t, u, k) ((char *) NULL)
#define __mp_realloc(p, l, a, f, s, t, u, g, h, k) ((void *) NULL)
#define __mp_free(p, f, s, t, u, k) ((void) 0)
#define __mp_setmem(p, l, c, f, s, t, u, k) ((void *) NULL)
#define __mp_copymem(p, q, l, c, f, s, t, u, k) ((void *) NULL)
#define __mp_locatemem(p, l, q, m, f, s, t, u, k) ((void *) NULL)
#define __mp_comparemem(p, q, l, f, s, t, u, k) ((int) 0)
#define __mp_function(f) ((char *) NULL)
#define __mp_setuser(p, d) ((int) 0)
#define __mp_info(p, d) ((int) 0)
#define __mp_syminfo(p, d) ((int) 0)
#define __mp_printinfo(p) ((int) 0)
#define __mp_snapshot() ((unsigned long) 0)
#define __mp_iterate(p, d, s) ((size_t) 0)
#define __mp_memorymap(s) ((void) 0)
#define __mp_summary() ((void) 0)
#define __mp_stats(d) ((int) 0)
#define __mp_check() ((void) 0)
#define __mp_prologue(h) ((void (*)(MP_CONST void *, size_t, MP_CONST void *)) NULL)
#define __mp_epilogue(h) ((void (*)(MP_CONST void *, MP_CONST void *)) NULL)
#define __mp_nomemory(h) ((void (*)(void)) NULL)
#define __mp_pushdelstack(s, t, u) ((void) 0)
#define __mp_popdelstack(s, t, u) ((void) 0)
#define __mp_vprintf(s, v) ((int) 0)
#define __mp_logmemory(p, l) ((void) 0)
#define __mp_logstack(k) ((int) 0)
#define __mp_logaddr(p) ((int) 0)
#define __mp_edit(f, l) ((int) 0)
#define __mp_list(f, l) ((int) 0)
#define __mp_view(f, l) ((int) 0)

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ == 199901L)
#define __mp_printf(s, ...) ((int) 0)
#elif defined(__GNUC__)
#define __mp_printf(s, a...) ((int) 0)
#else /* __STDC_VERSION__ && __GNUC__ */
static
int
__mp_printf(MP_CONST char *s, ...)
{
    return 0;
}
#endif /* __STDC_VERSION__ && __GNUC__ */

#endif /* NDEBUG */


#if !MP_NOCPLUSPLUS
#ifdef __cplusplus
#ifndef NDEBUG


/* Set the low-memory handler.
 */

MP_INLINE
std::new_handler
std::set_new_handler(std::new_handler h) throw()
{
    return (std::new_handler) __mp_nomemory(h);
}


/* Override operator new.
 */

MP_INLINE
void *
operator new(size_t l, MP_CONST char *s, MP_CONST char *t, unsigned long u)
             throw(std::bad_alloc)
{
    void *p;

    if ((p = __mp_alloc(l, 0, MP_AT_NEW, s, t, u, NULL, 0, 0)) == NULL)
        throw std::bad_alloc();
    return p;
}


/* Override nothrow version of operator new.
 */

MP_INLINE
void *
operator new(size_t l, MP_CONST char *s, MP_CONST char *t, unsigned long u,
             MP_CONST std::nothrow_t&) throw()
{
    return __mp_alloc(l, 0, MP_AT_NEW, s, t, u, NULL, 0, 0);
}


/* Override operator new[].
 */

MP_INLINE
void *
operator new[](size_t l, MP_CONST char *s, MP_CONST char *t, unsigned long u)
               throw(std::bad_alloc)
{
    void *p;

    if ((p = __mp_alloc(l, 0, MP_AT_NEWVEC, s, t, u, NULL, 0, 0)) == NULL)
        throw std::bad_alloc();
    return p;
}


/* Override nothrow version of operator new[].
 */

MP_INLINE
void *
operator new[](size_t l, MP_CONST char *s, MP_CONST char *t, unsigned long u,
               MP_CONST std::nothrow_t&) throw()
{
    return __mp_alloc(l, 0, MP_AT_NEWVEC, s, t, u, NULL, 0, 0);
}


/* Override operator delete.
 */

MP_INLINE
void
operator delete(void *p) throw()
{
    char *s, *t;
    unsigned long u;

    __mp_popdelstack(&s, &t, &u);
    __mp_free(p, MP_AT_DELETE, s, t, u, 0);
}


/* Override nothrow version of operator delete.
 */

MP_INLINE
void
operator delete(void *p, MP_CONST std::nothrow_t&) throw()
{
    char *s, *t;
    unsigned long u;

    __mp_popdelstack(&s, &t, &u);
    __mp_free(p, MP_AT_DELETE, s, t, u, 0);
}


/* Override operator delete[].
 */

MP_INLINE
void
operator delete[](void *p) throw()
{
    char *s, *t;
    unsigned long u;

    __mp_popdelstack(&s, &t, &u);
    __mp_free(p, MP_AT_DELETEVEC, s, t, u, 0);
}


/* Override nothrow version of operator delete[].
 */

MP_INLINE
void
operator delete[](void *p, MP_CONST std::nothrow_t&) throw()
{
    char *s, *t;
    unsigned long u;

    __mp_popdelstack(&s, &t, &u);
    __mp_free(p, MP_AT_DELETEVEC, s, t, u, 0);
}


#if MP_NONEWDELETE
#define MP_NEW ::new(MP_FUNCNAME, __FILE__, __LINE__)
#define MP_NEW_NOTHROW ::new(MP_FUNCNAME, __FILE__, __LINE__, std::nothrow)
#define MP_DELETE __mp_pushdelstack(MP_FUNCNAME, __FILE__, __LINE__), ::delete
#else /* MP_NONEWDELETE */
#define new ::new(MP_FUNCNAME, __FILE__, __LINE__)
#define delete __mp_pushdelstack(MP_FUNCNAME, __FILE__, __LINE__), ::delete
#endif /* MP_NONEWDELETE */

#endif /* NDEBUG */
#endif /* __cplusplus */
#endif /* MP_NOCPLUSPLUS */


#endif /* MP_MPATROL_H */
