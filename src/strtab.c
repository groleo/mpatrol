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
 * String tables.  All memory allocations are performed by the heap
 * manager and all strnodes are stored at the beginning of their
 * respective blocks of memory.  The hash function comes from P. J.
 * Weinberger's C compiler and was published in Compilers: Principles,
 * Techniques and Tools, First Edition by Aho, Sethi and Ullman
 * (Addison-Wesley, 1986, ISBN 0-201-10194-7).
 */


#include "strtab.h"
#include "utils.h"
#include <string.h>


#if MP_IDENT_SUPPORT
#ident "$Id: strtab.c,v 1.4 2000-03-13 21:09:43 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* Initialise the fields of a strtab so that the string table becomes empty.
 */

MP_GLOBAL void __mp_newstrtab(strtab *t, heaphead *h)
{
    struct { char x; strnode y; } z;
    long n;

    t->heap = h;
    __mp_newtree(&t->tree);
    t->size = 0;
    /* Determine the minimum alignment for a strnode on this system
     * and force the alignment to be a power of two.
     */
    n = (char *) &z.y - &z.x;
    t->align = __mp_poweroftwo(n);
}


/* Forget all data currently in the string table.
 */

MP_GLOBAL void __mp_deletestrtab(strtab *t)
{
    /* We don't need to explicitly free any memory as this is dealt with
     * at a lower level by the heap manager.
     */
    t->heap = NULL;
    __mp_newtree(&t->tree);
    t->size = 0;
}


/* Calculate the hash bucket a string should be placed in.
 */

static unsigned long hash(char *s)
{
    unsigned long g, h;

    for (h = 0; *s != '\0'; s++)
    {
        h = (h << 4) + *s;
        if (g = h & 0xF0000000)
        {
            h ^= g >> 24;
            h ^= g;
        }
    }
    return h % MP_HASHTABSIZE;
}


/* Add a new string to the string table.
 */

MP_GLOBAL char *__mp_addstring(strtab *t, char *s)
{
    strnode *n;
    heapnode *p;
    char *r;
    size_t l, m;

    l = strlen(s) + 1;
    /* If we have no suitable space left then we must allocate some more
     * memory for the string table in order for it to be able to hold the
     * new string.  The size of the new string is rounded up to a multiple
     * of the system page size.
     */
    if ((n = (strnode *) __mp_searchhigher(t->tree.root, l)) == NULL)
    {
        m = __mp_roundup(sizeof(strnode) + l, t->heap->memory.page);
        if ((p = __mp_heapalloc(t->heap, m, t->align)) == NULL)
            return NULL;
        n = (strnode *) p->block;
        n->block = p->block;
        n->next = (char *) p->block + sizeof(strnode);
        n->avail = p->size - sizeof(strnode);
        n->size = p->size;
        t->size += p->size;
    }
    else
        __mp_treeremove(&t->tree, &n->node);
    r = n->next;
    __mp_memcopy(r, s, l);
    n->next += l;
    n->avail -= l;
    /* We have already removed the strnode from the allocation tree since
     * we have altered the number of available characters within it.  We now
     * insert the strnode back into the tree, reflecting its new status.
     */
    __mp_treeinsert(&t->tree, &n->node, n->avail);
    return r;
}


/* Protect the memory blocks used by the string table with the
 * supplied access permission.
 */

MP_GLOBAL int __mp_protectstrtab(strtab *t, memaccess a)
{
    strnode *n;

    for (n = (strnode *) __mp_minimum(t->tree.root); n != NULL;
         n = (strnode *) __mp_successor(&n->node))
        if (!__mp_memprotect(&t->heap->memory, n->block, n->size, a))
            return 0;
    return 1;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
