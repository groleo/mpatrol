#ifndef MP_INFO_H
#define MP_INFO_H


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
 * Allocation information.  The high-level details of every memory
 * allocation are stored by this module, while the low-level details
 * are dealt with by the memory allocation module.
 */


#include "config.h"
#include "alloc.h"
#include "addr.h"
#include "profile.h"
#include "signals.h"


#define FLG_CHECKALLOCS   1    /* check all memory allocations */
#define FLG_CHECKREALLOCS 2    /* check all memory reallocations */
#define FLG_CHECKFREES    4    /* check all memory deallocations */
#define FLG_LOGALLOCS     8    /* log all memory allocations */
#define FLG_LOGREALLOCS   16   /* log all memory reallocations */
#define FLG_LOGFREES      32   /* log all memory deallocations */
#define FLG_LOGMEMORY     64   /* log all memory operations */
#define FLG_SHOWFREED     128  /* show all freed allocations */
#define FLG_SHOWUNFREED   256  /* show all unfreed allocations */
#define FLG_SHOWMAP       512  /* show memory map of heap */
#define FLG_SHOWSYMBOLS   1024 /* show all symbols read */
#define FLG_SAFESIGNALS   2048 /* save and restore signal handlers */
#define FLG_NOPROTECT     4096 /* do not protect internal structures */

#define FLG_FREED         1    /* allocation has been freed */
#define FLG_PROFILED      2    /* allocation has been profiled */
#define FLG_INTERNAL      4    /* allocation was made from within the library */


/* The different types of memory allocation and operation functions.
 */

typedef enum alloctype
{
    AT_MALLOC,    /* malloc() */
    AT_CALLOC,    /* calloc() */
    AT_MEMALIGN,  /* memalign() */
    AT_VALLOC,    /* valloc() */
    AT_PVALLOC,   /* pvalloc() */
    AT_STRDUP,    /* strdup() */
    AT_STRNDUP,   /* strndup() */
    AT_STRSAVE,   /* strsave() */
    AT_STRNSAVE,  /* strnsave() */
    AT_REALLOC,   /* realloc() */
    AT_RECALLOC,  /* recalloc() */
    AT_EXPAND,    /* expand() */
    AT_FREE,      /* free() */
    AT_CFREE,     /* cfree() */
    AT_NEW,       /* operator new */
    AT_NEWVEC,    /* operator new[] */
    AT_DELETE,    /* operator delete */
    AT_DELETEVEC, /* operator delete[] */
    AT_MEMSET,    /* memset() */
    AT_BZERO,     /* bzero() */
    AT_MEMCPY,    /* memcpy() */
    AT_MEMMOVE,   /* memmove() */
    AT_BCOPY,     /* bcopy() */
    AT_MEMCHR,    /* memchr() */
    AT_MEMMEM,    /* memmem() */
    AT_MEMCMP,    /* memcmp() */
    AT_BCMP,      /* bcmp() */
    AT_MAX
}
alloctype;


/* The structure used to record source level information about recursive
 * calls to C++ operator delete and operator delete[].
 */

typedef struct delstack
{
    char *func;         /* calling function name */
    char *file;         /* file name in which call took place */
    unsigned long line; /* line number at which call took place */
}
delstack;


/* An allocation information node belongs to a table of nodes, although
 * details of internal memory allocations are also stored in allocation
 * information nodes as part of a list.
 */

typedef union infonode
{
    struct
    {
        listnode node;         /* internal list node */
        void *block;           /* pointer to block of memory */
        size_t size;           /* size of block of memory */
    }
    index;
    struct
    {
        alloctype type;        /* type of memory allocation */
        unsigned long alloc;   /* allocation index */
        unsigned long realloc; /* reallocation index */
#if MP_THREADS_SUPPORT
        unsigned long thread;  /* thread identifier */
#endif /* MP_THREADS_SUPPORT */
        char *func;            /* calling function name */
        char *file;            /* file name in which call took place */
        unsigned long line;    /* line number at which call took place */
        addrnode *stack;       /* call stack details */
        unsigned long flags;   /* allocation flags */
    }
    data;
}
infonode;


/* An infohead holds the table of allocation information nodes as well
 * as all of the other data structures used by the mpatrol library.
 */

typedef struct infohead
{
    allochead alloc;                  /* allocation table */
    addrhead addr;                    /* stack address table */
    symhead syms;                     /* symbol table */
    sighead signals;                  /* signal handler table */
    profhead prof;                    /* profiling information */
    slottable table;                  /* table of information nodes */
    listhead list;                    /* internal allocation list */
    size_t size;                      /* internal allocation total */
    size_t count;                     /* allocation count */
    size_t peak;                      /* allocation peak */
    size_t limit;                     /* allocation limit */
    size_t astop;                     /* allocation stop index */
    size_t rstop;                     /* reallocation stop index */
    size_t fstop;                     /* free stop index */
    size_t uabort;                    /* unfreed abort minimum */
    size_t lrange;                    /* lower check range */
    size_t urange;                    /* upper check range */
    size_t dtotal;                    /* total bytes compared */
    size_t ltotal;                    /* total bytes located */
    size_t ctotal;                    /* total bytes copied */
    size_t stotal;                    /* total bytes set */
    unsigned long ffreq;              /* failure frequency */
    unsigned long fseed;              /* failure seed */
    void (*prologue)(void *, size_t); /* prologue function */
    void (*epilogue)(void *);         /* epilogue function */
    void (*nomemory)(void);           /* low-memory handler function */
    char *log;                        /* log filename */
    delstack dels[MP_MAXDELSTACK];    /* delete stack */
    long delpos;                      /* delete stack pointer */
    unsigned long flags;              /* global flags */
    memaccess prot;                   /* protection status */
    size_t recur;                     /* recursion count */
    char init;                        /* initialisation flag */
    char fini;                        /* finalisation flag */
}
infohead;


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


MP_EXPORT void __mp_newinfo(infohead *);
MP_EXPORT void __mp_deleteinfo(infohead *);
MP_EXPORT void *__mp_getmemory(infohead *, size_t, size_t, alloctype, char *,
                               char *, unsigned long, stackinfo *);
MP_EXPORT void *__mp_resizememory(infohead *, void *, size_t, size_t, alloctype,
                                  char *, char *, unsigned long, stackinfo *);
MP_EXPORT void __mp_freememory(infohead *, void *, alloctype, char *, char *,
                               unsigned long, stackinfo *);
MP_EXPORT void __mp_setmemory(infohead *, void *, size_t, unsigned char,
                              alloctype, char *, char *, unsigned long,
                              stackinfo *);
MP_EXPORT void __mp_copymemory(infohead *, void *, void *, size_t, alloctype,
                               char *, char *, unsigned long, stackinfo *);
MP_EXPORT void *__mp_locatememory(infohead *, void *, size_t, void *, size_t,
                                  alloctype, char *, char *, unsigned long,
                                  stackinfo *);
MP_EXPORT int __mp_comparememory(infohead *, void *, void *, size_t, alloctype,
                                 char *, char *, unsigned long, stackinfo *);
MP_EXPORT int __mp_protectinfo(infohead *, memaccess);
MP_EXPORT void __mp_checkinfo(infohead *);
MP_EXPORT int __mp_checkrange(infohead *, void *, size_t, alloctype);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* MP_INFO_H */
