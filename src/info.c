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
 * Allocation information.  The functions in this module deal primarily with
 * the secondary information associated with memory allocations.
 */


#include "info.h"
#include "diag.h"
#if MP_THREADS_SUPPORT
#include "mutex.h"
#endif /* MP_THREADS_SUPPORT */
#include "utils.h"
#include <string.h>
#include <errno.h>


#if MP_IDENT_SUPPORT
#ident "$Id: info.c,v 1.1.1.1 1999-10-03 11:25:21 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* Initialise the fields of an infohead so that the mpatrol library
 * is ready to perform dynamic memory allocations.
 */

MP_GLOBAL void __mp_newinfo(infohead *h)
{
    struct { char x; infonode y; } z;
    long n;

    /* The signal table is initialised before this function is called
     * because we have already entered the library at this point.  The
     * same goes for the recur field.
     */
    __mp_newallocs(&h->alloc, MP_OVERFLOW, MP_OVERBYTE, MP_ALLOCBYTE,
                   MP_FREEBYTE, 0);
    __mp_newaddrs(&h->addr, &h->alloc.heap);
    __mp_newsymbols(&h->syms, &h->alloc.heap);
    /* Determine the minimum alignment for an allocation information node
     * on this system and force the alignment to be a power of two.  This
     * information is used when initialising the slot table.
     */
    n = (char *) &z.y - &z.x;
    __mp_newslots(&h->table, sizeof(infonode), __mp_poweroftwo(n));
    __mp_newlist(&h->list);
    /* Initialise the settings to their default values.
     */
    h->size = h->count = h->peak = h->limit = 0;
    h->astop = h->rstop = h->fstop = h->uabort = 0;
    h->prologue = NULL;
    h->epilogue = NULL;
    h->nomemory = NULL;
    h->log = __mp_logfile(NULL);
#if MP_PROTECT_SUPPORT
    h->flags = 0;
#else /* MP_PROTECT_SUPPORT */
    /* If the system does not support memory protection then we just set the
     * NOPROTECT flag here, which saves us calling a function which does nothing
     * each time we want to protect the library's internal structures.
     */
    h->flags = FLG_NOPROTECT;
#endif /* MP_PROTECT_SUPPORT */
    h->prot = MA_READWRITE;
    /* Now that the infohead has valid fields we can now set the initialised
     * flag.  This means that the library can now recursively call malloc()
     * or another memory allocation function without any problems.  It just
     * means that there will not be a log entry at that point, but generally
     * we don't need one as the user will only want to see their memory
     * allocations.
     */
    h->init = 1;
    h->fini = 0;
}


/* Free up all memory used by the infohead.
 */

MP_GLOBAL void __mp_deleteinfo(infohead *h)
{
    /* We should close the log file first in case that calls a dynamic
     * memory allocation function, since once __mp_deleteallocs() is
     * called the heap no longer exists.
     */
    __mp_closelogfile();
    h->log = NULL;
    __mp_deleteallocs(&h->alloc);
    __mp_deleteaddrs(&h->addr);
    __mp_deletesymbols(&h->syms);
    h->table.free = NULL;
    h->table.size = 0;
    __mp_newlist(&h->list);
    h->size = h->count = h->peak = 0;
}


/* Allocate a new allocation information node.
 */

static infonode *getinfonode(infohead *h)
{
    infonode *n;
    heapnode *p;

    /* If we have no more allocation information node slots left then we
     * must allocate some more memory for them.  An extra page of memory
     * should suffice.
     */
    if ((n = (infonode *) __mp_getslot(&h->table)) == NULL)
    {
        if ((p = __mp_heapalloc(&h->alloc.heap, h->alloc.heap.memory.page,
              h->table.entalign)) == NULL)
            return NULL;
        __mp_initslots(&h->table, p->block, p->size);
        n = (infonode *) __mp_getslot(&h->table);
        __mp_addtail(&h->list, &n->index.node);
        n->index.block = p->block;
        n->index.size = p->size;
        h->size += p->size;
        n = (infonode *) __mp_getslot(&h->table);
    }
    return n;
}


/* Allocate a new block of memory of a specified size and alignment.
 */

MP_GLOBAL void *__mp_getmemory(infohead *h, size_t l, size_t a, alloctype f,
                               char *s, char *t, unsigned long u, stackinfo *v)
{
    allocnode *n;
    infonode *m;
    void *p;

    p = NULL;
    h->count++;
    if ((h->flags & FLG_LOGALLOCS) && (h->recur == 1))
    {
        /* Write an entry into the log file.
         */
        __mp_diag("ALLOC: %s (%lu, ", __mp_alloctypenames[f], h->count);
        __mp_printsize(l);
        __mp_diag(", ");
        if (a == 0)
            __mp_printsize(h->alloc.heap.memory.align);
        else
            __mp_printsize(a);
        __mp_diag(") [");
#if MP_THREADS_SUPPORT
        __mp_diag("%lu|", __mp_threadid());
#endif /* MP_THREADS_SUPPORT */
        __mp_diag("%s|%s|", (s ? s : "-"), (t ? t : "-"));
        if (u == 0)
            __mp_diag("-");
        else
            __mp_diag("%lu", u);
        __mp_diag("]\n");
        __mp_printstack(&h->syms, v);
        __mp_diag("\n");
    }
    if ((h->count == h->astop) && (h->rstop == 0))
    {
        /* Abort at the specified allocation index.
         */
        __mp_printsummary(h);
        __mp_diag("\n");
        __mp_diag("stopping at allocation %lu\n", h->astop);
        __mp_trap();
    }
    if ((h->flags & FLG_CHECKALLOCS) && (l == 0))
        __mp_warn(f, "attempt to create an allocation of size 0\n");
    if (f == AT_MEMALIGN)
    {
        /* Check that the specified alignment is valid.  This is only
         * performed for memalign() so that we can report any problems
         * in the log file.  All other cases are checked silently.
         */
        if (a == 0)
        {
            if (h->flags & FLG_CHECKALLOCS)
                __mp_warn(f, "alignment 0 is invalid\n");
            a = h->alloc.heap.memory.page;
        }
        else if (!__mp_ispoweroftwo(a))
        {
            if (h->flags & FLG_CHECKALLOCS)
                __mp_warn(f, "alignment %lu is not a power of two\n", a);
            a = __mp_poweroftwo(a);
        }
        else if (a > h->alloc.heap.memory.page)
        {
            if (h->flags & FLG_CHECKALLOCS)
                __mp_warn(f, "alignment %lu is greater than the system page "
                          "size\n", a);
            a = h->alloc.heap.memory.page;
        }
    }
    else if ((f == AT_VALLOC) || (f == AT_PVALLOC))
    {
        /* Check that the specified size and alignment for valloc() and
         * pvalloc() are valid.
         */
        if (f == AT_PVALLOC)
        {
            if (l == 0)
                l = 1;
            l = __mp_roundup(l, h->alloc.heap.memory.page);
        }
        a = h->alloc.heap.memory.page;
    }
    if ((h->limit > 0) && (h->alloc.asize + l > h->limit))
        errno = ENOMEM;
    else
    {
        if (!(h->flags & FLG_NOPROTECT))
            __mp_protectinfo(h, MA_READWRITE);
        if (m = getinfonode(h))
            if (n = __mp_getalloc(&h->alloc, l, a, m))
            {
                /* Fill in the details of the allocation information node.
                 */
                m->data.type = f;
                m->data.alloc = h->count;
                m->data.realloc = 0;
#if MP_THREADS_SUPPORT
                m->data.thread = __mp_threadid();
#endif /* MP_THREADS_SUPPORT */
                m->data.func = s;
                m->data.file = t;
                m->data.line = u;
                m->data.stack = __mp_getaddrs(&h->addr, v);
                m->data.freed = 0;
                p = n->block;
                if ((f == AT_CALLOC) || (f == AT_RECALLOC))
                    memset(p, 0, l);
                else
                    memset(p, h->alloc.abyte, l);
            }
            else
                __mp_freeslot(&h->table, m);
        if ((h->recur == 1) && !(h->flags & FLG_NOPROTECT))
            __mp_protectinfo(h, MA_READONLY);
        if (h->peak < h->alloc.asize)
            h->peak = h->alloc.asize;
    }
    if ((h->flags & FLG_LOGALLOCS) && (h->recur == 1))
        __mp_diag("returns " MP_POINTER "\n\n", p);
    return p;
}


/* Resize an existing block of memory to a new size and alignment.
 */

MP_GLOBAL void *__mp_resizememory(infohead *h, void *p, size_t l, size_t a,
                                  alloctype f, char *s, char *t,
                                  unsigned long u, stackinfo *v)
{
    allocnode *n, *r;
    infonode *i, *m;
    size_t d;

    if ((h->flags & FLG_LOGREALLOCS) && (h->recur == 1))
    {
        /* Write an entry into the log file.
         */
        __mp_diag("REALLOC: %s (" MP_POINTER ", ", __mp_alloctypenames[f], p);
        __mp_printsize(l);
        __mp_diag(", ");
        if (a == 0)
            __mp_printsize(h->alloc.heap.memory.align);
        else
            __mp_printsize(a);
        __mp_diag(") [");
#if MP_THREADS_SUPPORT
        __mp_diag("%lu|", __mp_threadid());
#endif /* MP_THREADS_SUPPORT */
        __mp_diag("%s|%s|", (s ? s : "-"), (t ? t : "-"));
        if (u == 0)
            __mp_diag("-");
        else
            __mp_diag("%lu", u);
        __mp_diag("]\n");
        __mp_printstack(&h->syms, v);
        __mp_diag("\n");
    }
    if (p == NULL)
    {
        if (h->flags & FLG_CHECKREALLOCS)
            __mp_warn(f, "attempt to resize a NULL pointer\n");
        p = __mp_getmemory(h, l, a, f, s, t, u, v);
    }
    else if (n = __mp_findfreed(&h->alloc, p))
    {
        /* This block of memory has already been freed but has not been
         * returned to the free tree.
         */
        m = (infonode *) n->info;
        __mp_error(f, MP_POINTER " was freed with %s", p,
                   __mp_alloctypenames[m->data.type]);
        __mp_printalloc(&h->syms, n);
        __mp_diag("\n");
        p = NULL;
    }
    else if (((n = __mp_findalloc(&h->alloc, p)) == NULL) ||
             ((m = (infonode *) n->info) == NULL))
    {
        /* We know nothing about this block of memory.
         */
        __mp_error(f, MP_POINTER " has not been allocated\n", p);
        p = NULL;
    }
    else if (p != n->block)
    {
        /* The address of the block passed in does not match the start
         * address of the block we know about.
         */
        __mp_error(f, MP_POINTER " does not match allocation of " MP_POINTER, p,
                   n->block);
        __mp_printalloc(&h->syms, n);
        __mp_diag("\n");
        p = NULL;
    }
    else if ((m->data.type == AT_NEW) || (m->data.type == AT_NEWVEC))
    {
        /* The function used to allocate the block is incompatible with
         * operator new or operator new[].
         */
        __mp_error(f, MP_POINTER " was allocated with %s", p,
                   __mp_alloctypenames[m->data.type]);
        __mp_printalloc(&h->syms, n);
        __mp_diag("\n");
        p = NULL;
    }
    else if (l == 0)
    {
        if (h->flags & FLG_CHECKREALLOCS)
            __mp_warn(f, "attempt to resize an allocation to size 0\n");
        __mp_freememory(h, p, f, s, t, u, v);
        p = NULL;
    }
    else
    {
        if ((h->flags & FLG_LOGREALLOCS) && (h->recur == 1))
        {
            __mp_printalloc(&h->syms, n);
            __mp_diag("\n");
        }
        if ((m->data.realloc + 1 == h->rstop) && ((h->astop == 0) ||
             (m->data.alloc == h->astop)))
        {
            /* Abort at the specified reallocation index.
             */
            __mp_printsummary(h);
            __mp_diag("\n");
            if (h->astop == 0)
                __mp_diag("stopping at reallocation %lu\n", h->rstop);
            else
                __mp_diag("stopping at reallocation %lu of allocation %lu\n",
                          h->rstop, h->astop);
            __mp_trap();
        }
        if ((h->limit > 0) && (l > n->size) &&
            (h->alloc.asize + l - n->size > h->limit))
        {
            errno = ENOMEM;
            p = NULL;
        }
        else
        {
            d = n->size;
            if (!(h->flags & FLG_NOPROTECT))
                __mp_protectinfo(h, MA_READWRITE);
            m->data.realloc++;
            if (h->alloc.flags & FLG_NOFREE)
                /* We are not going to even attempt to resize the memory if
                 * we are preserving free blocks, and instead we will just
                 * create a new block all the time and preserve the old block.
                 */
                if ((i = getinfonode(h)) &&
                    (r = __mp_getalloc(&h->alloc, l, a, m)))
                {
                    /* Fill in the details of the allocation information node.
                     */
                    i->data.type = f;
                    i->data.alloc = m->data.alloc;
                    i->data.realloc = m->data.realloc - 1;
#if MP_THREADS_SUPPORT
                    i->data.thread = __mp_threadid();
#endif /* MP_THREADS_SUPPORT */
                    i->data.func = s;
                    i->data.file = t;
                    i->data.line = u;
                    i->data.stack = __mp_getaddrs(&h->addr, v);
                    i->data.freed = 1;
                    memcpy(r->block, n->block, (l > d) ? d : l);
                    __mp_freealloc(&h->alloc, n, i);
                    p = r->block;
                }
                else
                {
                    if (i != NULL)
                        __mp_freeslot(&h->table, i);
                    p = NULL;
                }
            else if (l == d)
                /* The old size is the same as the new size, so we just
                 * return an address to the start of the existing block.
                 */
                p = n->block;
            else if (!__mp_resizealloc(&h->alloc, n, l))
                /* If __mp_resizealloc() failed and all allocations are to
                 * be aligned to the end of pages or the size requested is
                 * greater than the existing size then we must allocate a
                 * new block, copy the contents and free the old block.
                 */
                if ((((h->alloc.flags & FLG_PAGEALLOC) &&
                      (h->alloc.flags & FLG_ALLOCUPPER)) || (l > d)) &&
                    (r = __mp_getalloc(&h->alloc, l, a, m)))
                {
                    memcpy(r->block, n->block, (l > d) ? d : l);
                    __mp_freealloc(&h->alloc, n, NULL);
                    p = r->block;
                }
                else
                    p = NULL;
            if ((h->recur == 1) && !(h->flags & FLG_NOPROTECT))
                __mp_protectinfo(h, MA_READONLY);
            if ((p != NULL) && (l > d))
                if (f == AT_RECALLOC)
                    memset((char *) p + d, 0, l - d);
                else
                    memset((char *) p + d, h->alloc.abyte, l - d);
            if (h->peak < h->alloc.asize)
                h->peak = h->alloc.asize;
        }
    }
    if ((h->flags & FLG_LOGREALLOCS) && (h->recur == 1))
        __mp_diag("returns " MP_POINTER "\n\n", p);
    return p;
}


/* Free an existing block of memory.
 */

MP_GLOBAL void __mp_freememory(infohead *h, void *p, alloctype f, char *s,
                               char *t, unsigned long u, stackinfo *v)
{
    allocnode *n;
    infonode *m;

    if ((h->flags & FLG_LOGFREES) && (h->recur == 1))
    {
        /* Write an entry into the log file.
         */
        __mp_diag("FREE: %s (" MP_POINTER, __mp_alloctypenames[f], p);
        __mp_diag(") [");
#if MP_THREADS_SUPPORT
        __mp_diag("%lu|", __mp_threadid());
#endif /* MP_THREADS_SUPPORT */
        __mp_diag("%s|%s|", (s ? s : "-"), (t ? t : "-"));
        if (u == 0)
            __mp_diag("-");
        else
            __mp_diag("%lu", u);
        __mp_diag("]\n");
        __mp_printstack(&h->syms, v);
        __mp_diag("\n");
    }
    if (p == NULL)
    {
        if (h->flags & FLG_CHECKFREES)
            __mp_warn(f, "attempt to free a NULL pointer\n");
        return;
    }
    if (n = __mp_findfreed(&h->alloc, p))
    {
        /* This block of memory has already been freed but has not been
         * returned to the free tree.
         */
        m = (infonode *) n->info;
        __mp_error(f, MP_POINTER " was freed with %s", p,
                   __mp_alloctypenames[m->data.type]);
        __mp_printalloc(&h->syms, n);
        __mp_diag("\n");
    }
    else if (((n = __mp_findalloc(&h->alloc, p)) == NULL) ||
             ((m = (infonode *) n->info) == NULL))
        /* We know nothing about this block of memory.
         */
        __mp_error(f, MP_POINTER " has not been allocated\n", p);
    else if (p != n->block)
    {
        /* The address of the block passed in does not match the start
         * address of the block we know about.
         */
        __mp_error(f, MP_POINTER " does not match allocation of " MP_POINTER, p,
                   n->block);
        __mp_printalloc(&h->syms, n);
        __mp_diag("\n");
    }
    else if (((m->data.type == AT_NEW) && (f != AT_DELETE)) ||
             ((m->data.type != AT_NEW) && (f == AT_DELETE)) ||
             ((m->data.type == AT_NEWVEC) && (f != AT_DELETEVEC)) ||
             ((m->data.type != AT_NEWVEC) && (f == AT_DELETEVEC)))
    {
        /* The function used to allocate the block is incompatible with
         * the function used to free the block.
         */
        __mp_error(f, MP_POINTER " was allocated with %s", p,
                   __mp_alloctypenames[m->data.type]);
        __mp_printalloc(&h->syms, n);
        __mp_diag("\n");
    }
    else
    {
        if ((h->flags & FLG_LOGFREES) && (h->recur == 1))
        {
            __mp_printalloc(&h->syms, n);
            __mp_diag("\n");
        }
        if (m->data.alloc == h->fstop)
        {
            /* Abort at the specified allocation index.
             */
            __mp_printsummary(h);
            __mp_diag("\n");
            __mp_diag("stopping at freeing of allocation %lu\n", h->fstop);
            __mp_trap();
        }
        if (!(h->flags & FLG_NOPROTECT))
            __mp_protectinfo(h, MA_READWRITE);
        __mp_freeaddrs(&h->addr, m->data.stack);
        if (h->alloc.flags & FLG_NOFREE)
        {
            /* Fill in the details of the allocation information node but only
             * if we are keeping the freed block.
             */
            m->data.type = f;
#if MP_THREADS_SUPPORT
            m->data.thread = __mp_threadid();
#endif /* MP_THREADS_SUPPORT */
            m->data.func = s;
            m->data.file = t;
            m->data.line = u;
            m->data.stack = __mp_getaddrs(&h->addr, v);
            m->data.freed = 1;
        }
        else
        {
            __mp_freeslot(&h->table, m);
            m = NULL;
        }
        __mp_freealloc(&h->alloc, n, m);
        if ((h->recur == 1) && !(h->flags & FLG_NOPROTECT))
            __mp_protectinfo(h, MA_READONLY);
    }
}


/* Protect the internal memory blocks used by the mpatrol library
 * with the supplied access permission.
 */

MP_GLOBAL int __mp_protectinfo(infohead *h, memaccess a)
{
    infonode *n;

    /* The library already knows what its protection status is so we don't
     * need to do anything if the request has already been done.
     */
    if (a == h->prot)
        return 1;
    h->prot = a;
    for (n = (infonode *) h->list.head; n->index.node.next != NULL;
         n = (infonode *) n->index.node.next)
        if (!__mp_memprotect(&h->alloc.heap.memory, n->index.block,
             n->index.size, a))
            return 0;
    if (!__mp_protectaddrs(&h->addr, a))
        return 0;
    return __mp_protectalloc(&h->alloc, a);
}


/* Check the validity of all memory blocks that have been filled with
 * a predefined pattern.
 */

MP_GLOBAL void __mp_checkinfo(infohead *h)
{
    allocnode *n;
    infonode *m;
    void *b, *p;
    size_t l, s;

    for (n = (allocnode *) h->alloc.list.head; n->lnode.next != NULL;
         n = (allocnode *) n->lnode.next)
    {
        if ((m = (infonode *) n->info) == NULL)
            /* Check that all free blocks are filled with the free byte, but
             * only if all allocations are not pages since they will be read
             * and write protected in that case.
             */
            if (!(h->alloc.flags & FLG_PAGEALLOC) &&
                (p = __mp_memcheck(n->block, h->alloc.fbyte, n->size)))
            {
                __mp_printsummary(h);
                __mp_diag("\n");
                __mp_error(AT_MAX, "free memory corruption at " MP_POINTER, p);
                if ((l = (char *) n->block + n->size - (char *) p) > 256)
                    __mp_printmemory(p, 256);
                else
                    __mp_printmemory(p, l);
                h->fini = 1;
                __mp_abort();
            }
            else
                continue;
        if (m->data.freed && !(h->alloc.flags & FLG_PAGEALLOC) &&
            !(h->alloc.flags & FLG_PRESERVE))
            /* Check that all freed blocks are filled with the free byte, but
             * only if all allocations are not pages and the original contents
             * were not preserved.
             */
            if (p = __mp_memcheck(n->block, h->alloc.fbyte, n->size))
            {
                __mp_printsummary(h);
                __mp_diag("\n");
                __mp_error(AT_MAX, "freed allocation " MP_POINTER " has memory "
                           "corruption at " MP_POINTER, n->block, p);
                if ((l = (char *) n->block + n->size - (char *) p) > 256)
                    __mp_printmemory(p, 256);
                else
                    __mp_printmemory(p, l);
                __mp_diag("\n");
                __mp_printalloc(&h->syms, n);
                h->fini = 1;
                __mp_abort();
            }
        if (h->alloc.flags & FLG_OFLOWWATCH)
            /* If we have watch areas on every overflow buffer then we don't
             * need to perform the following checks.
             */
            continue;
        if (h->alloc.flags & FLG_PAGEALLOC)
        {
            /* Check that all allocated and freed blocks have overflow buffers
             * filled with the overflow byte, but only if all allocations are
             * pages as this check examines the overflow buffers within the page
             * boundaries.
             */
            b = (void *) __mp_rounddown((unsigned long) n->block,
                                        h->alloc.heap.memory.page);
            s = (char *) n->block - (char *) b;
            l = __mp_roundup(n->size + s, h->alloc.heap.memory.page);
            if ((p = __mp_memcheck(b, h->alloc.obyte, s)) ||
                (p = __mp_memcheck((char *) n->block + n->size, h->alloc.obyte,
                  l - n->size - s)))
            {
                __mp_printsummary(h);
                __mp_diag("\n");
                if (m->data.freed)
                    __mp_error(AT_MAX, "freed allocation " MP_POINTER " has a "
                               "corrupted overflow buffer at " MP_POINTER,
                               n->block, p);
                else
                    __mp_error(AT_MAX, "allocation " MP_POINTER " has a "
                               "corrupted overflow buffer at " MP_POINTER,
                               n->block, p);
                if (p < n->block)
                    __mp_printmemory(b, s);
                else
                    __mp_printmemory((char *) n->block + n->size,
                                     l - n->size - s);
                __mp_diag("\n");
                __mp_printalloc(&h->syms, n);
                h->fini = 1;
                __mp_abort();
            }
        }
        else if ((l = h->alloc.oflow) > 0)
            /* Check that all allocated and freed blocks have overflow buffers
             * filled with the overflow byte, but only if all allocations are
             * not pages and the overflow buffer size is greater than zero.
             */
            if ((p = __mp_memcheck((char *) n->block - l, h->alloc.obyte, l)) ||
                (p = __mp_memcheck((char *) n->block + n->size, h->alloc.obyte,
                  l)))
            {
                __mp_printsummary(h);
                __mp_diag("\n");
                if (m->data.freed)
                    __mp_error(AT_MAX, "freed allocation " MP_POINTER " has a "
                               "corrupted overflow buffer at " MP_POINTER,
                               n->block, p);
                else
                    __mp_error(AT_MAX, "allocation " MP_POINTER " has a "
                               "corrupted overflow buffer at " MP_POINTER,
                               n->block, p);
                if (p < n->block)
                    __mp_printmemory((char *) n->block - l, l);
                else
                    __mp_printmemory((char *) n->block + n->size, l);
                __mp_diag("\n");
                __mp_printalloc(&h->syms, n);
                h->fini = 1;
                __mp_abort();
            }
    }
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
