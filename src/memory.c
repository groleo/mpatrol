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
 * Memory handling.  All memory access or handling routines that directly
 * access the system memory are interfaced from this module.
 */


#include "memory.h"
#include "stack.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#if TARGET == TARGET_UNIX
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#if MP_WATCH_SUPPORT
#include <procfs.h>
#endif /* MP_WATCH_SUPPORT */
#elif TARGET == TARGET_AMIGA
#include <proto/dos.h>
#include <proto/exec.h>
#include <exec/memory.h>
#elif TARGET == TARGET_WINDOWS
#include <windows.h>
#include <winbase.h>
#include <process.h>
#elif TARGET == TARGET_NETWARE
#include <nwthread.h>
#include <nks/memory.h>
#endif /* TARGET */


#if MP_IDENT_SUPPORT
#ident "$Id: memory.c,v 1.2 1999-10-14 18:58:39 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#if MP_WATCH_SUPPORT
/* This structure is used to simplify the building of the watch command before
 * it is written to the control file of the /proc filesystem.
 */

typedef struct watchcmd
{
    long cmd;       /* always PCWATCH */
    prwatch_t data; /* details of addresses to watch */
}
watchcmd;
#endif /* MP_WATCH_SUPPORT */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* Determine the minimum alignment for a general-purpose memory allocation
 * on this system.
 */

static size_t minalign(void)
{
    size_t a;
    long n;

    {
        /* Hopefully the largest integral type.  If the compiler supports
         * long long, it doesn't necessarily mean that it will have a more
         * restrictive alignment than a long integer, but we allow that
         * check anyway.
         */
#if MP_LONGLONG_SUPPORT
        struct { char x; long long y; } z;
#else /* MP_LONGLONG_SUPPORT */
        struct { char x; long y; } z;
#endif /* MP_LONGLONG_SUPPORT */
        n = (char *) &z.y - &z.x;
    }
    a = n;
    {
        /* Hopefully the largest floating point type.  The long double
         * type appeared with the ANSI standard and this code is written
         * in ANSI C so we shouldn't need to worry about not supporting it.
         */
        struct { char x; long double y; } z;
        n = (char *) &z.y - &z.x;
    }
    if (a < n)
        a = n;
    {
        /* A generic pointer type.  The assumption in this case is that
         * a pointer to void is the most restrictive pointer type on this
         * system.
         */
        struct { char x; void *y; } z;
        n = (char *) &z.y - &z.x;
    }
    if (a < n)
        a = n;
    return __mp_poweroftwo(a);
}


/* Return the system page size.
 */

static size_t pagesize(void)
{
#if TARGET == TARGET_WINDOWS
    SYSTEM_INFO i;
#endif /* TARGET */

#if TARGET == TARGET_UNIX
    /* This call could also be getpagesize() but it is more POSIX-conforming
     * to call sysconf().
     */
    return sysconf(_SC_PAGESIZE);
#elif TARGET == TARGET_AMIGA
    /* The Amiga has no virtual memory system (at least not supplied with
     * AmigaOS), so we return a fixed value here because it doesn't really
     * matter what the page size is.
     */
    return 4096;
#elif TARGET == TARGET_WINDOWS
    GetSystemInfo(&i);
    return i.dwPageSize;
#elif TARGET == TARGET_NETWARE
    return NXGetPageSize();
#endif /* TARGET */
}


/* Return the executable file name that the program was invoked with.
 * Note that this function will not be reentrant if the return value is
 * a pointer to a local static string buffer.
 */

static char *progname(void)
{
#if TARGET == TARGET_UNIX
#if MP_PROCFS_SUPPORT
    static char b[64];
#elif ARCH == ARCH_IX86 || ARCH == ARCH_M68K || ARCH == ARCH_SPARC
    unsigned int *p;
    stackinfo s;
#endif /* MP_PROCFS_SUPPORT && ARCH */
#elif TARGET == TARGET_AMIGA || TARGET == TARGET_WINDOWS
    static char p[256];
#elif TARGET == TARGET_NETWARE
    char *p, *t;
#endif /* TARGET */

#if TARGET == TARGET_UNIX
#if MP_PROCFS_SUPPORT
    /* If the /proc filesystem is supported then we can usually access the
     * actual executable file that contains the current program through a
     * special file in the current /proc entry.
     */
    sprintf(b, "%s/%lu/%s", MP_PROCFS_DIRNAME, __mp_processid(),
            MP_PROCFS_EXENAME);
    return b;
#elif (ARCH == ARCH_IX86 || ARCH == ARCH_M68K || ARCH == ARCH_SPARC) && \
      !MP_BUILTINSTACK_SUPPORT
    /* Because there is no function to return the executable filename
     * of a process on UNIX, we need to cheat and rely on the ABI by walking
     * up the process stack till we reach the startup code and then find
     * argv[0].  This is very OS-specific and is not my first choice for
     * doing this, but unfortunately it seemed to be the only way.
     */
    __mp_newframe(&s);
    for (p = NULL; __mp_getframe(&s); p = (unsigned int *) s.frame);
    if (p != NULL)
#if ARCH == ARCH_IX86
        if (p = (unsigned int *) p[3])
            return (char *) p;
#elif ARCH == ARCH_M68K
        if (p = (unsigned int *) p[3])
            return (char *) *p;
#elif ARCH == ARCH_SPARC
        if (p = (unsigned int *) *(((unsigned int *) *p) + 1))
            return (char *) *p;
#endif /* ARCH */
#endif /* MP_PROCFS_SUPPORT && ARCH && MP_BUILTINSTACK_SUPPORT */
#elif TARGET == TARGET_AMIGA
    if (GetProgramName(p, sizeof(p)))
        return p;
#elif TARGET == TARGET_WINDOWS
    if (GetModuleFileName(NULL, p, sizeof(p)))
        return p;
#elif TARGET == TARGET_NETWARE
    if (GetNLMNameFromNLMID(GetNLMID(), &p, &t) == 0)
        return p;
#endif /* TARGET */
    return NULL;
}


/* Initialise the fields of a meminfo structure to describe the details
 * of the underlying memory architecture.
 */

MP_GLOBAL void __mp_newmemory(meminfo *i)
{
#if MP_WATCH_SUPPORT
    char b[64];
#endif /* MP_WATCH_SUPPORT */

    i->align = minalign();
    i->page = pagesize();
    i->prog = progname();
    /* On UNIX, we initially set the memory mapped file handle to be -1 as we
     * default to using sbrk(), even on systems that support the mmap() function
     * call.  We only set this to point to the memory mapped file if the USEMMAP
     * option has been found when parsing the library options.
     */
    i->mfile = -1;
#if MP_WATCH_SUPPORT
    sprintf(b, "%s/%lu/%s", MP_PROCFS_DIRNAME, __mp_processid(),
            MP_PROCFS_CTLNAME);
    i->wfile = open(b, O_WRONLY);
#else /* MP_WATCH_SUPPORT */
    i->wfile = -1;
#endif /* MP_WATCH_SUPPORT */
}


/* Free up any resources used by the meminfo structure.
 */

MP_GLOBAL void __mp_endmemory(meminfo *i)
{
#if MP_MMAP_SUPPORT
    if (i->mfile != -1)
    {
        close(i->mfile);
        i->mfile = -1;
    }
#endif /* MP_MMAP_SUPPORT */
#if MP_WATCH_SUPPORT
    if (i->wfile != -1)
    {
        close(i->wfile);
        i->wfile = -1;
    }
#endif /* MP_WATCH_SUPPORT */
}


/* Return the process identifier.
 */

MP_GLOBAL unsigned long __mp_processid(void)
{
#if TARGET == TARGET_UNIX || TARGET == TARGET_WINDOWS
    return (unsigned long) getpid();
#elif TARGET == TARGET_AMIGA
    return (unsigned long) FindTask(NULL);
#elif TARGET == TARGET_NETWARE
    return (unsigned long) GetThreadId();
#endif /* TARGET */
}


/* Allocate a specified size of general-purpose memory from the system
 * with a required alignment.
 */

MP_GLOBAL void *__mp_memalloc(meminfo *i, size_t *l, size_t a)
{
    void *p;
#if TARGET == TARGET_UNIX
    void *t;
    unsigned long n;
#endif /* TARGET */

    if (*l == 0)
        *l = 1;
#if TARGET == TARGET_UNIX || TARGET == TARGET_WINDOWS || \
    TARGET == TARGET_NETWARE
    *l = __mp_roundup(*l, i->page);
#elif TARGET == TARGET_AMIGA
    /* We aren't guaranteed to allocate a block of memory that is page
     * aligned on the Amiga, so we have to assume the worst case scenario
     * and allocate more memory for the specified alignment.
     */
    if (a > i->page)
        a = i->page;
    if (a > MEM_BLOCKSIZE)
        *l += __mp_poweroftwo(a) - MEM_BLOCKSIZE;
#endif /* TARGET */
#if TARGET == TARGET_UNIX
    /* UNIX has a contiguous heap for a process, but we are not guaranteed to
     * have full control over it, so we must assume that each separate memory
     * allocation is independent.  If we are using sbrk() to allocate memory
     * then we also try to ensure that all of our memory allocations are blocks
     * of pages.
     */
#if MP_MMAP_SUPPORT
    /* Decide if we are using mmap() or sbrk() to allocate the memory.  It's
     * not recommended that we do both as they could conflict in nasty ways.
     */
    if (i->mfile != -1)
    {
        if ((p = mmap(NULL, *l, PROT_READ | PROT_WRITE, MAP_PRIVATE, i->mfile,
              0)) == (void *) -1)
            p = NULL;
    }
    else
#endif /* MP_MMAP_SUPPORT */
    {
        if (((t = sbrk(0)) == (void *) -1) || ((p = sbrk(*l)) == (void *) -1))
            p = NULL;
        else
        {
            if (p < t)
                /* The heap has grown down, which is quite unusual except on
                 * some weird systems where the stack grows up.
                 */
                n = (unsigned long) p - __mp_rounddown((unsigned long) p,
                                                       i->page);
            else
                n = __mp_roundup((unsigned long) p, i->page) -
                    (unsigned long) p;
            if (n > 0)
                /* We need to allocate a little more memory in order to make the
                 * allocation page-aligned.
                 */
                if ((p = sbrk(n)) == (void *) -1)
                {
                    /* We failed to allocate more memory, but we try to be nice
                     * and return our original allocation.
                     */
                    sbrk(-*l);
                    p = NULL;
                }
                else if (p >= t)
                    p = (char *) p + n - i->page;
        }
    }
#elif TARGET == TARGET_AMIGA
    p = AllocMem(*l, MEMF_ANY | MEMF_CLEAR);
#elif TARGET == TARGET_WINDOWS
    p = VirtualAlloc(NULL, *l, MEM_COMMIT, PAGE_READWRITE);
#elif TARGET == TARGET_NETWARE
    p = NXPageAlloc(*l / i->page, 0);
#endif /* TARGET */
#if TARGET == TARGET_UNIX || TARGET == TARGET_NETWARE
    /* UNIX's sbrk() and Netware's NXPageAlloc() do not zero the allocated
     * memory, so we do this here for predictable behaviour.
     */
    if ((i->mfile == -1) && (p != NULL))
        memset(p, 0, *l);
#endif /* TARGET */
    if (p == NULL)
        errno = ENOMEM;
    return p;
}


/* Return a block of allocated memory back to the system.
 */

MP_GLOBAL void __mp_memfree(meminfo *i, void *p, size_t l)
{
#if TARGET == TARGET_UNIX || TARGET == TARGET_WINDOWS || \
    TARGET == TARGET_NETWARE
    void *t;
#endif /* TARGET */

    /* This function is hardly ever called except when the process is
     * terminating as the heap manager will take care of reusing unused
     * memory.
     */
    if (l == 0)
        return;
#if TARGET == TARGET_UNIX || TARGET == TARGET_WINDOWS || \
    TARGET == TARGET_NETWARE
    t = (void *) __mp_rounddown((unsigned long) p, i->page);
#endif /* TARGET */
#if TARGET == TARGET_UNIX
    /* If we used sbrk() to allocate this memory then we can't shrink the
     * break point since someone else might have allocated memory in between
     * our allocations.  The next best thing is to unmap our freed allocations
     * so that they no longer need to be handled by the virtual memory system.
     * If we used mmap() to allocate this memory then we don't need to worry
     * about the above problem.
     */
    l = __mp_roundup(l + ((char *) p - (char *) t), i->page);
    mprotect(t, l, PROT_NONE);
    munmap(t, l);
#elif TARGET == TARGET_AMIGA
    FreeMem(p, l);
#elif TARGET == TARGET_WINDOWS
    VirtualFree(t, 0, MEM_RELEASE);
#elif TARGET == TARGET_NETWARE
    NXPageFree(t);
#endif /* TARGET */
}


/* Protect a block of allocated memory with the supplied access permission.
 */

MP_GLOBAL int __mp_memprotect(meminfo *i, void *p, size_t l, memaccess a)
{
#if TARGET == TARGET_UNIX || TARGET == TARGET_WINDOWS
    void *t;
    int n;
#endif /* TARGET */

#if TARGET == TARGET_UNIX || TARGET == TARGET_WINDOWS
    if (l == 0)
        return 1;
    t = (void *) __mp_rounddown((unsigned long) p, i->page);
    l = __mp_roundup(l + ((char *) p - (char *) t), i->page);
#if TARGET == TARGET_UNIX
    if (a == MA_NOACCESS)
        n = PROT_NONE;
    else if (a == MA_READONLY)
        n = PROT_READ;
    else
        n = PROT_READ | PROT_WRITE;
    if (mprotect(t, l, n) == -1)
        return 0;
#elif TARGET == TARGET_WINDOWS
    if (a == MA_NOACCESS)
        n = PAGE_NOACCESS;
    else if (a == MA_READONLY)
        n = PAGE_READONLY;
    else
        n = PAGE_READWRITE;
    if (!VirtualProtect(t, l, n, (unsigned long *) &n))
        return 0;
#endif /* TARGET */
#endif /* TARGET */
    return 1;
}


/* Notify the operating system to watch a specified group of bytes with the
 * supplied access permission.
 */

MP_GLOBAL int __mp_memwatch(meminfo *i, void *p, size_t l, memaccess a)
{
#if MP_WATCH_SUPPORT
    watchcmd w;
#endif /* MP_WATCH_SUPPORT */

#if MP_WATCH_SUPPORT
    if (l == 0)
        return 1;
    w.cmd = PCWATCH;
    w.data.pr_vaddr = (uintptr_t) p;
    w.data.pr_size = l;
    if (a == MA_NOACCESS)
        w.data.pr_wflags = WA_READ | WA_WRITE | WA_TRAPAFTER;
    else if (a == MA_READONLY)
        w.data.pr_wflags = WA_WRITE | WA_TRAPAFTER;
    else
        w.data.pr_wflags = 0;
    if ((i->wfile == -1) ||
        (write(i->wfile, (void *) &w, sizeof(watchcmd)) != sizeof(watchcmd)))
        return 0;
#endif /* MP_WATCH_SUPPORT */
    return 1;
}


/* Check that a block of memory only contains a specific byte.
 */

MP_GLOBAL void *__mp_memcheck(void *t, char c, size_t l)
{
    char *p;

    for (p = (char *) t, t = (char *) t + l; p < (char *) t; p++)
        if (*p != c)
            return p;
    return NULL;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
