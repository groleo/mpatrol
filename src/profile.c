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
 * Memory allocation profiling.  The call graphs for every memory allocation
 * and deallocation are recorded here along with their memory usage statistics
 * and are written to a file for later processing by a profiling tool.
 */


#include "profile.h"
#include "info.h"
#include "diag.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>


#if MP_IDENT_SUPPORT
#ident "$Id: profile.c,v 1.14 2000-04-20 23:45:46 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* Initialise the fields of a profhead so that the mpatrol library
 * is ready to profile memory allocations.
 */

MP_GLOBAL void __mp_newprofile(profhead *p, heaphead *h)
{
    struct { char x; profnode y; } z;
    size_t i;
    long n;

    p->heap = h;
    /* Determine the minimum alignment for a profnode on this system
     * and force the alignment to be a power of two.  This information
     * is used when initialising the slot table.
     */
    n = (char *) &z.y - &z.x;
    __mp_newslots(&p->table, sizeof(profnode), __mp_poweroftwo(n));
    __mp_newlist(&p->list);
    __mp_newtree(&p->tree);
    p->size = 0;
    for (i = 0; i < MP_BIN_SIZE; i++)
        p->acounts[i] = p->dcounts[i] = 0;
    p->atotals = p->dtotals = 0;
    p->sbound = MP_SMALLBOUND;
    p->mbound = MP_MEDIUMBOUND;
    p->lbound = MP_LARGEBOUND;
    p->autosave = p->autocount = 0;
    p->file = MP_PROFFILE;
    p->profiling = 0;
}


/* Forget all existing profiling information.
 */

MP_GLOBAL void __mp_deleteprofile(profhead *p)
{
    size_t i;

    /* We don't need to explicitly free any memory as this is dealt with
     * at a lower level by the heap manager.
     */
    p->heap = NULL;
    p->table.free = NULL;
    p->table.size = 0;
    __mp_newlist(&p->list);
    __mp_newtree(&p->tree);
    p->size = 0;
    for (i = 0; i < MP_BIN_SIZE; i++)
        p->acounts[i] = p->dcounts[i] = 0;
    p->atotals = p->dtotals = 0;
    p->autocount = 0;
    p->file = NULL;
    p->profiling = 0;
}


/* Allocate a new profiling node.
 */

static profnode *getprofnode(profhead *p)
{
    profnode *n;
    heapnode *h;

    /* If we have no more profnode slots left then we must allocate
     * some more memory for them.  An extra MP_ALLOCFACTOR pages of memory
     * should suffice.
     */
    if ((n = (profnode *) __mp_getslot(&p->table)) == NULL)
    {
        if ((h = __mp_heapalloc(p->heap, p->heap->memory.page * MP_ALLOCFACTOR,
              p->table.entalign)) == NULL)
            return NULL;
        __mp_initslots(&p->table, h->block, h->size);
        n = (profnode *) __mp_getslot(&p->table);
        __mp_addtail(&p->list, &n->index.node);
        n->index.block = h->block;
        n->index.size = h->size;
        p->size += h->size;
        n = (profnode *) __mp_getslot(&p->table);
    }
    return n;
}


/* Locate or create a call site associated with a specified return address.
 */

static profnode *getcallsite(profhead *p, addrnode *a)
{
    profnode *n;
    size_t i;

    if ((n = (profnode *) __mp_search(p->tree.root,
          (unsigned long) a->data.addr)) == NULL)
    {
        if ((n = getprofnode(p)) == NULL)
            return NULL;
        __mp_treeinsert(&p->tree, &n->data.node, (unsigned long) a->data.addr);
        for (i = 0; i < 4; i++)
        {
            n->data.data.acount[i] = n->data.data.dcount[i] = 0;
            n->data.data.atotal[i] = n->data.data.dtotal[i] = 0;
        }
    }
    return n;
}


/* Record a memory allocation for profiling.
 */

MP_GLOBAL int __mp_profilealloc(profhead *p, size_t l, void *d)
{
    profnode *n;
    infonode *m;
    size_t i;

    /* Try to associate the allocation with a previous call site, or create
     * a new call site if no such site exists.  This information is not
     * recorded if the return address could not be determined.
     */
    m = (infonode *) d;
    if ((m->data.stack != NULL) && (m->data.stack->data.addr != NULL))
    {
        if ((n = getcallsite(p, m->data.stack)) == NULL)
            return 0;
        if (l <= p->sbound)
            i = 0;
        else if (l <= p->mbound)
            i = 1;
        else if (l <= p->lbound)
            i = 2;
        else
            i = 3;
        n->data.data.acount[i]++;
        n->data.data.atotal[i] += l;
    }
    /* Note the size of the allocation in one of the allocation bins.
     * The highest allocation bin stores a count of all the allocations
     * that are larger than the largest bin.
     */
    if (l < MP_BIN_SIZE)
        p->acounts[l - 1]++;
    else
    {
        p->acounts[MP_BIN_SIZE - 1]++;
        p->atotals += l;
    }
    /* If the autosave feature is enabled then we may need to write out
     * all of the current profiling information to the output file before
     * we can return.
     */
    p->autocount++;
    if ((p->autosave != 0) && (p->autocount % p->autosave == 0))
        __mp_writeprofile(p);
    return 1;
}


/* Record a memory deallocation for profiling.
 */

MP_GLOBAL int __mp_profilefree(profhead *p, size_t l, void *d)
{
    profnode *n;
    infonode *m;
    size_t i;

    /* Try to associate the deallocation with a previous call site, or create
     * a new call site if no such site exists.  This information is not
     * recorded if the return address could not be determined.
     */
    m = (infonode *) d;
    if ((m->data.stack != NULL) && (m->data.stack->data.addr != NULL))
    {
        if ((n = getcallsite(p, m->data.stack)) == NULL)
            return 0;
        if (l <= p->sbound)
            i = 0;
        else if (l <= p->mbound)
            i = 1;
        else if (l <= p->lbound)
            i = 2;
        else
            i = 3;
        n->data.data.dcount[i]++;
        n->data.data.dtotal[i] += l;
    }
    /* Note the size of the deallocation in one of the deallocation bins.
     * The highest deallocation bin stores a count of all the deallocations
     * that are larger than the largest bin.
     */
    if (l < MP_BIN_SIZE)
        p->dcounts[l - 1]++;
    else
    {
        p->dcounts[MP_BIN_SIZE - 1]++;
        p->dtotals += l;
    }
    /* If the autosave feature is enabled then we may need to write out
     * all of the current profiling information to the output file before
     * we can return.
     */
    p->autocount++;
    if ((p->autosave != 0) && (p->autocount % p->autosave == 0))
        __mp_writeprofile(p);
    return 1;
}


/* Write the profiling information to the output file.
 */

MP_GLOBAL int __mp_writeprofile(profhead *p)
{
    char s[4];
    profnode *n;
    FILE *f;
    size_t i;

    p->autocount = 0;
    /* The profiling file name can also be named as stderr and stdout which
     * will go to the standard error and standard output streams respectively.
     */
    if (p->file == NULL)
        return 0;
    else if (strcmp(p->file, "stderr") == 0)
        f = stderr;
    else if (strcmp(p->file, "stdout") == 0)
        f = stdout;
    else if ((f = fopen(p->file, "wb")) == NULL)
    {
        __mp_error(AT_MAX, "%s: cannot open file\n", p->file);
        p->file = NULL;
        return 0;
    }
    /* Technically, we should check the return values from each of the calls
     * to fwrite().  However, that would increase the complexity of this
     * function and would make the code extremely hard to follow.  Instead,
     * we just assume that each write to the output file succeeds and hope
     * that if an error does occur then it will not be too drastic if we
     * continue writing the rest of the file.
     */
    __mp_memcopy(s, MP_PROFMAGIC, 4);
    fwrite(s, sizeof(char), 4, f);
    /* Write out the allocation and deallocation bins.
     */
    i = MP_BIN_SIZE;
    fwrite(&i, sizeof(size_t), 1, f);
    fwrite(p->acounts, sizeof(size_t), MP_BIN_SIZE, f);
    fwrite(&p->atotals, sizeof(size_t), 1, f);
    fwrite(p->dcounts, sizeof(size_t), MP_BIN_SIZE, f);
    fwrite(&p->dtotals, sizeof(size_t), 1, f);
    /* Write out the statistics from every call site.
     */
    fwrite(&p->sbound, sizeof(size_t), 1, f);
    fwrite(&p->mbound, sizeof(size_t), 1, f);
    fwrite(&p->lbound, sizeof(size_t), 1, f);
    fwrite(&p->tree.size, sizeof(size_t), 1, f);
    for (n = (profnode *) __mp_minimum(p->tree.root); n != NULL;
         n = (profnode *) __mp_successor(&n->data.node))
    {
        fwrite(&n->data.node.key, sizeof(unsigned long), 1, f);
        fwrite(n->data.data.acount, sizeof(size_t), 4, f);
        fwrite(n->data.data.atotal, sizeof(size_t), 4, f);
        fwrite(n->data.data.dcount, sizeof(size_t), 4, f);
        fwrite(n->data.data.dtotal, sizeof(size_t), 4, f);
    }
    fwrite(s, sizeof(char), 4, f);
    if ((f != stderr) && (f != stdout) && fclose(f))
        return 0;
    return 1;
}


/* Protect the memory blocks used by the profiling table with the supplied
 * access permission.
 */

MP_GLOBAL int __mp_protectprofile(profhead *p, memaccess a)
{
    profnode *n;

    for (n = (profnode *) p->list.head; n->index.node.next != NULL;
         n = (profnode *) n->index.node.next)
        if (!__mp_memprotect(&p->heap->memory, n->index.block, n->index.size,
             a))
            return 0;
    return 1;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
