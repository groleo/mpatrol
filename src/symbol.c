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
 * Symbol tables.  The implementation of reading symbols from executable files
 * may also interface with access libraries which support other file formats.
 * This module may also have to liaise with the dynamic linker (or equivalent)
 * in order to obtain symbols from shared objects.
 */


#include "symbol.h"
#include "diag.h"
#include "utils.h"
#if FORMAT == FORMAT_COFF || FORMAT == FORMAT_ELF32 || FORMAT == FORMAT_BFD
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#if FORMAT == FORMAT_COFF
#include <a.out.h>
#elif FORMAT == FORMAT_ELF32
#include <libelf.h>
#elif FORMAT == FORMAT_BFD
#include <bfd.h>
#endif /* FORMAT */
#endif /* FORMAT */
#if SYSTEM == SYSTEM_DGUX || SYSTEM == SYSTEM_DYNIX || \
    SYSTEM == SYSTEM_LINUX || SYSTEM == SYSTEM_SOLARIS
/* Despite the fact that Linux is now ELF-based, libelf seems to be missing from
 * many recent distributions and so we must use the GNU BFD library to read the
 * symbols from the object files and libraries.  However, we still need the ELF
 * definitions for reading the internal structures of the dynamic linker.
 */
#include <elf.h>
#endif /* SYSTEM */


#if MP_IDENT_SUPPORT
#ident "$Id: symbol.c,v 1.6 2000-02-10 20:24:08 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#if SYSTEM == SYSTEM_DGUX || SYSTEM == SYSTEM_DYNIX || \
    SYSTEM == SYSTEM_LINUX || SYSTEM == SYSTEM_SOLARIS
/* These definitions are not always defined in ELF header files on all
 * systems so we define them here as they are documented in most
 * System V ABI documents.
 */

#ifndef DT_NULL
#define DT_NULL  0
#define DT_DEBUG 21

typedef struct Elf32_Dyn
{
    Elf32_Sword d_tag;    /* tag indicating type of data stored */
    union
    {
        Elf32_Word d_val; /* data is a value */
        Elf32_Addr d_ptr; /* data is a pointer */
    }
    d_un;
}
Elf32_Dyn;
#endif /* DT_NULL */
#endif /* SYSTEM */


#if SYSTEM == SYSTEM_DGUX || SYSTEM == SYSTEM_DYNIX || \
    SYSTEM == SYSTEM_LINUX || SYSTEM == SYSTEM_SOLARIS
/* This is a structure that is internal to the dynamic linker on ELF systems,
 * and so it is not always guaranteed to be the same.  We try to rely on this
 * definition here for portability's sake as it is not publicly declared in
 * all ELF header files.
 */

typedef struct dynamiclink
{
    size_t base;              /* virtual address of shared object */
    char *name;               /* filename of shared object */
    Elf32_Dyn *dyn;           /* dynamic linking information */
    struct dynamiclink *next; /* pointer to next shared object */
}
dynamiclink;
#endif /* SYSTEM */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


#if SYSTEM == SYSTEM_DGUX || SYSTEM == SYSTEM_DYNIX || \
    SYSTEM == SYSTEM_LINUX || SYSTEM == SYSTEM_SOLARIS
/* The declaration of the _DYNAMIC symbol, which allows us direct access to the
 * dynamic linker's internal data structures.  We make it have weak visibility
 * so that it is always defined, even in the statically linked case.  It is
 * declared as a function because some compilers only allow weak function
 * symbols.  Another alternative would be to declare it as a common symbol, but
 * that wouldn't work under C++ and many libc shared objects have it defined as
 * a text symbol.
 */

#ifdef __GNUC__
void _DYNAMIC(void) __attribute__ ((weak));
#else /* __GNUC__ */
#pragma weak _DYNAMIC
void _DYNAMIC(void);
#endif /* __GNUC__ */
#endif /* SYSTEM */


/* Initialise the fields of a symhead so that the symbol table becomes empty.
 */

MP_GLOBAL void __mp_newsymbols(symhead *y, heaphead *h)
{
    struct { char x; symnode y; } z;
    long n;

    y->heap = h;
    /* Determine the minimum alignment for a symbol node on this system
     * and force the alignment to be a power of two.  This information
     * is used when initialising the slot table.
     */
    n = (char *) &z.y - &z.x;
    __mp_newstrtab(&y->strings, h);
    __mp_newslots(&y->table, sizeof(symnode), __mp_poweroftwo(n));
    __mp_newtree(&y->itree);
    __mp_newtree(&y->dtree);
    y->size = 0;
}


/* Forget all data currently in the symbol table.
 */

MP_GLOBAL void __mp_deletesymbols(symhead *y)
{
    /* We don't need to explicitly free any memory as this is dealt with
     * at a lower level by the heap manager.
     */
    y->heap = NULL;
    __mp_deletestrtab(&y->strings);
    y->table.free = NULL;
    y->table.size = 0;
    __mp_newtree(&y->itree);
    __mp_newtree(&y->dtree);
    y->size = 0;
}


/* Allocate a new symbol node.
 */

static symnode *getsymnode(symhead *y)
{
    symnode *n;
    heapnode *p;

    /* If we have no more symbol node slots left then we must allocate
     * some more memory for them.  An extra page of memory should
     * suffice.
     */
    if ((n = (symnode *) __mp_getslot(&y->table)) == NULL)
    {
        if ((p = __mp_heapalloc(y->heap, y->heap->memory.page,
              y->table.entalign)) == NULL)
            return NULL;
        __mp_initslots(&y->table, p->block, p->size);
        n = (symnode *) __mp_getslot(&y->table);
        __mp_treeinsert(&y->itree, &n->index.node, (unsigned long) p->block);
        n->index.block = p->block;
        n->index.size = p->size;
        y->size += p->size;
        n = (symnode *) __mp_getslot(&y->table);
    }
    return n;
}


#if FORMAT == FORMAT_COFF
/* Allocate a new symbol node for a given COFF symbol.
 */

static int addsymbol(symhead *y, SYMENT *p, char *f, char *s)
{
    AUXENT *e;
    symnode *n;
    char *r;

    /* We don't bother storing a symbol which has no name or whose name
     * contains a '$', '@' or a '.'.  We also don't allocate a symbol node
     * for symbols which have a virtual address of zero and we only remember
     * symbols that are declared statically or externally visible.
     */
    if ((s != NULL) && (*s != '\0') && !strpbrk(s, "$@.") && (p->n_value > 0) &&
        ((p->n_sclass == C_STAT) || (p->n_sclass == C_EXT)))
    {
        if ((n = getsymnode(y)) == NULL)
            return 0;
        if ((r = __mp_addstring(&y->strings, s)) == NULL)
        {
            __mp_freeslot(&y->table, n);
            return 0;
        }
        __mp_treeinsert(&y->dtree, &n->data.node, p->n_value);
        n->data.file = f;
        n->data.name = r;
        n->data.addr = (void *) p->n_value;
        /* Attempt to figure out the size of the symbol.
         */
        if (p->n_numaux > 0)
        {
            e = (AUXENT *) ((char *) p + SYMESZ);
            if (ISFCN(p->n_type))
                n->data.size = e->x_sym.x_misc.x_fsize;
            else
                n->data.size = e->x_sym.x_misc.x_lnsz.x_size;
        }
        else
            n->data.size = 0;
        /* The linkage information is required for when we look up a symbol.
         */
        n->data.flags = p->n_sclass;
    }
    return 1;
}
#elif FORMAT == FORMAT_ELF32
/* Allocate a new symbol node for a given ELF symbol.
 */

static int addsymbol(symhead *y, Elf32_Sym *p, char *f, char *s, size_t b)
{
    symnode *n;
    char *r;
    size_t a;
    unsigned char t;

    a = b + p->st_value;
    /* We don't bother storing a symbol which has no name or whose name
     * contains a '$', '@' or a '.'.  We also don't allocate a symbol node
     * for symbols which have a virtual address of zero or are of object type.
     */
    if ((s != NULL) && (*s != '\0') && !strpbrk(s, "$@.") && (a > 0) &&
        (((t = ELF32_ST_TYPE(p->st_info)) == STT_NOTYPE) || (t == STT_FUNC)))
    {
        if ((n = getsymnode(y)) == NULL)
            return 0;
        if ((r = __mp_addstring(&y->strings, s)) == NULL)
        {
            __mp_freeslot(&y->table, n);
            return 0;
        }
        __mp_treeinsert(&y->dtree, &n->data.node, a);
        n->data.file = f;
        n->data.name = r;
        n->data.addr = (void *) a;
        n->data.size = p->st_size;
        /* The linkage information is required for when we look up a symbol.
         */
        n->data.flags = ELF32_ST_BIND(p->st_info);
    }
    return 1;
}
#elif FORMAT == FORMAT_BFD
/* Allocate a new symbol node for a given BFD symbol.
 */

static int addsymbol(symhead *y, asymbol *p, char *f, char *s, size_t b)
{
    symnode *n;
    char *r;
    size_t a;

    a = b + (size_t) p->value;
    /* We don't bother storing a symbol which has no name or whose name
     * contains a '$', '@' or a '.'.  We also don't allocate a symbol node
     * for symbols which have a virtual address of zero and we only remember
     * symbols that are declared statically, externally or weakly visible.
     */
    if ((s != NULL) && (*s != '\0') && !strpbrk(s, "$@.") && (a > 0) &&
        (p->flags & (BSF_LOCAL | BSF_GLOBAL | BSF_WEAK)))
    {
        if ((n = getsymnode(y)) == NULL)
            return 0;
        if ((r = __mp_addstring(&y->strings, s)) == NULL)
        {
            __mp_freeslot(&y->table, n);
            return 0;
        }
        __mp_treeinsert(&y->dtree, &n->data.node, a);
        n->data.file = f;
        n->data.name = r;
        n->data.addr = (void *) a;
        /* The BFD library doesn't seem to support reading symbol sizes,
         * so we have to rely on the assumption that a function symbol
         * ends at the beginning of the next function symbol.  This will
         * be calculated in __mp_fixsymbols().
         */
        n->data.size = 0;
        /* The linkage information is required for when we look up a symbol.
         */
        n->data.flags = p->flags;
    }
    return 1;
}
#endif /* FORMAT */


#if FORMAT == FORMAT_COFF
/* Allocate a set of symbol nodes for a COFF executable file.
 */

static int addsymbols(symhead *y, char *e, char *f, size_t b)
{
    char n[SYMNMLEN + 1];
    FILHDR *o;
    SCNHDR *h;
    SYMENT *p;
    char *m, *s;
    size_t i, t;

    /* Check that we have a valid COFF executable file.
     */
    if (b < FILHSZ)
    {
        __mp_warn(AT_MAX, "%s: not an object file\n", f);
        return 1;
    }
    o = (FILHDR *) e;
    b -= FILHSZ;
    if (!ISCOFF(o->f_magic) || (o->f_opthdr == 0) || (b < o->f_opthdr))
    {
        __mp_warn(AT_MAX, "%s: not an executable file\n", f);
        return 1;
    }
    b -= o->f_opthdr;
    if ((o->f_nscns == 0) || (b < o->f_nscns * SCNHSZ))
    {
        __mp_warn(AT_MAX, "%s: missing section table\n", f);
        return 1;
    }
    /* Look for the section index of the text section.  This is
     * usually 1, but we should really make sure.
     */
    h = (SCNHDR *) (e + FILHSZ + o->f_opthdr);
    b += FILHSZ + o->f_opthdr - o->f_symptr;
    for (i = t = 0; i < o->f_nscns; i++)
        if ((h[i].s_flags & STYP_TEXT) || (strncmp(h[i].s_name, _TEXT, 8) == 0))
        {
            t = i + 1;
            break;
        }
    if (t == 0)
        t = 1;
    /* Look for the symbol table.
     */
    i = o->f_nsyms * SYMESZ;
    if ((o->f_symptr == 0) || (o->f_nsyms == 0) || (b < i))
    {
        __mp_warn(AT_MAX, "%s: missing symbol table\n", f);
        return 1;
    }
    p = (SYMENT *) (e + o->f_symptr);
    b -= i;
    /* Look for the string table.
     */
    m = (char *) p + i;
    if (b < 4)
        i = 0;
    else
        i = *((size_t *) m);
    if ((i == 0) || (b < i))
    {
        __mp_warn(AT_MAX, "%s: missing string table\n", f);
        return 1;
    }
    /* Cycle through every symbol contained in the executable file.
     */
    for (i = 0; i < o->f_nsyms; i += p[i].n_numaux + 1)
        /* We only need to bother looking at text symbols.
         */
        if (p[i].n_scnum == t)
        {
            if (p[i].n_zeroes)
            {
                /* Symbol name is stored in structure.
                 */
                strncpy(n, p[i].n_name, 8);
                n[8] = '\0';
                s = n;
            }
            else
                /* Symbol name is stored in string table.
                 */
                s = m + p[i].n_offset;
            if (!addsymbol(y, p + i, f, s))
                return 0;
        }
    return 1;
}
#elif FORMAT == FORMAT_ELF32
/* Allocate a set of symbol nodes for an ELF object file.
 */

static int addsymbols(symhead *y, Elf *e, char *a, char *f, size_t b)
{
    Elf_Scn *s;
    Elf32_Shdr *h;
    Elf32_Sym *p;
    Elf_Data *d;
    char *m;
    size_t i, l, n, t;

    /* Look for the symbol table.
     */
    for (s = elf_nextscn(e, NULL), d = NULL; s != NULL; s = elf_nextscn(e, s))
        if ((h = elf32_getshdr(s)) && (h->sh_type == SHT_SYMTAB) &&
            (d = elf_getdata(s, NULL)))
            break;
    if (d == NULL)
        /* If we couldn't find the symbol table then it is likely that the file
         * has been stripped.  However, if the file was dynamically linked then
         * we may be able to obtain some symbols from its dynamic symbol table.
         */
        for (s = elf_nextscn(e, NULL), d = NULL; s != NULL;
             s = elf_nextscn(e, s))
            if ((h = elf32_getshdr(s)) && (h->sh_type == SHT_DYNSYM) &&
                (d = elf_getdata(s, NULL)))
                break;
    if ((d == NULL) || (d->d_buf == NULL) || (d->d_size == 0))
    {
        m = "missing symbol table";
        if (a != NULL)
            __mp_warn(AT_MAX, "%s [%s]: %s\n", f, a, m);
        else
            __mp_warn(AT_MAX, "%s: %s\n", f, m);
        return 1;
    }
    t = h->sh_link;
    /* Look for the string table.
     */
    if (((s = elf_getscn(e, t)) == NULL) || ((h = elf32_getshdr(s)) == NULL) ||
        (h->sh_type != SHT_STRTAB))
    {
        m = "missing string table";
        if (a != NULL)
            __mp_warn(AT_MAX, "%s [%s]: %s\n", f, a, m);
        else
            __mp_warn(AT_MAX, "%s: %s\n", f, m);
        return 1;
    }
    if (a != NULL)
        f = a;
    p = (Elf32_Sym *) d->d_buf + 1;
    l = d->d_size / sizeof(Elf32_Sym);
    /* Cycle through every symbol contained in the object file.
     */
    for (i = 1; i < l; i++, p++)
        /* We don't need to bother looking at undefined, absolute or common
         * symbols, and we only need to store non-data symbols.
         */
        if (((n = p->st_shndx) != SHN_UNDEF) && (n != SHN_ABS) &&
            (n != SHN_COMMON) && (s = elf_getscn(e, n)) &&
            (h = elf32_getshdr(s)) && (h->sh_flags & SHF_EXECINSTR))
            if (!addsymbol(y, p, f, elf_strptr(e, t, p->st_name), b))
                return 0;
    return 1;
}
#elif FORMAT == FORMAT_BFD
/* Allocate a set of symbol nodes for a BFD object file.
 */

static int addsymbols(symhead *y, bfd *h, char *f, size_t b)
{
    asymbol **p;
    long i, n;
    int d, r;

    /* Look for the symbol table.
     */
    d = 0;
    if ((n = bfd_get_symtab_upper_bound(h)) < 0)
    {
        __mp_error(AT_MAX, "%s: %s\n", f, bfd_errmsg(bfd_get_error()));
        return 0;
    }
    if (n == 0)
    {
        /* If we couldn't find the symbol table then it is likely that the file
         * has been stripped.  However, if the file was dynamically linked then
         * we may be able to obtain some symbols from its dynamic symbol table.
         */
        if ((n = bfd_get_dynamic_symtab_upper_bound(h)) < 0)
        {
            __mp_error(AT_MAX, "%s: %s\n", f, bfd_errmsg(bfd_get_error()));
            return 0;
        }
        if (n == 0)
        {
            __mp_warn(AT_MAX, "%s: missing symbol table\n", f);
            return 1;
        }
        d = 1;
    }
    /* It's actually safe to call malloc() here since the library checks
     * for recursive behaviour.
     */
    if ((p = (asymbol **) malloc(n)) == NULL)
    {
        __mp_error(AT_MAX, "%s: no memory for symbols\n", f);
        return 0;
    }
    r = 1;
    if (((d == 0) && ((n = bfd_canonicalize_symtab(h, p)) < 0)) ||
        ((d == 1) && ((n = bfd_canonicalize_dynamic_symtab(h, p)) < 0)))
    {
        __mp_error(AT_MAX, "%s: %s\n", f, bfd_errmsg(bfd_get_error()));
        r = 0;
    }
    else
        /* Cycle through every symbol contained in the object file.
         */
        for (i = 0; i < n; i++)
            /* We don't need to bother looking at undefined, absolute or common
             * symbols, and we only need to store non-data symbols.
             */
            if (!bfd_is_und_section(p[i]->section) &&
                !bfd_is_abs_section(p[i]->section) &&
                !bfd_is_com_section(p[i]->section) &&
                (p[i]->section->flags & SEC_CODE))
                if (!addsymbol(y, p[i], f, (char *) p[i]->name,
                    (size_t) p[i]->section->vma + b))
                {
                    r = 0;
                    break;
                }
    free(p);
    return r;
}
#endif /* FORMAT */


/* Read a file and add all relevant symbols contained within it to the
 * symbol table.
 */

MP_GLOBAL int __mp_addsymbols(symhead *y, char *s, size_t b)
{
#if FORMAT == FORMAT_COFF || FORMAT == FORMAT_ELF32 || FORMAT == FORMAT_BFD
#if FORMAT == FORMAT_COFF
    char *m;
    off_t o;
#elif FORMAT == FORMAT_ELF32
    Elf *a, *e;
    Elf_Arhdr *h;
#elif FORMAT == FORMAT_BFD
    bfd *h;
#endif /* FORMAT */
    char *t;
    int f;
#endif /* FORMAT */
    int r;

    r = 1;
#if FORMAT == FORMAT_COFF
    /* This is a very simple, yet portable, way to read symbols from COFF
     * executable files.  It's unlikely to work with anything but the most
     * basic COFF file format.
     */
    if ((f = open(s, O_RDONLY)) == -1)
    {
        __mp_error(AT_MAX, "%s: cannot open file\n", s);
        r = 0;
    }
    else
    {
        /* Find out the size of the file by doing a seek to the end of the file
         * and then a seek back to the start of the file.  Unfortunately, on
         * some systems off_t is a structure, so the code below may not work
         * for them.
         */
        if (((o = lseek(f, 0, SEEK_END)) == (off_t) -1) ||
            (lseek(f, 0, SEEK_SET) == (off_t) -1))
        {
            __mp_error(AT_MAX, "%s: cannot seek file\n", s);
            r = 0;
        }
        else if ((m = (char *) malloc((size_t) o)) == NULL)
        {
            /* It's actually safe to call malloc() here since the
             * library checks for recursive behaviour.
             */
            __mp_error(AT_MAX, "%s: no memory for symbols\n", s);
            r = 0;
        }
        else
        {
            if (read(f, m, (size_t) o) != (size_t) o)
            {
                __mp_error(AT_MAX, "%s: cannot read file\n", s);
                r = 0;
            }
            else if ((t = __mp_addstring(&y->strings, s)) == NULL)
                r = 0;
            else
                r = addsymbols(y, m, t, (size_t) o);
            free(m);
        }
        close(f);
    }
#elif FORMAT == FORMAT_ELF32
    /* We use the libelf ELF access library in order to simplify the reading
     * of symbols.  At the moment, this solution is better than using the
     * GNU BFD library as it currently has no support for symbol sizes or
     * liaising with the dynamic linker.
     */
    if (elf_version(EV_CURRENT) == EV_NONE)
    {
        __mp_error(AT_MAX, "%s: wrong version of libelf\n", s);
        r = 0;
    }
    else if ((f = open(s, O_RDONLY)) == -1)
    {
        __mp_error(AT_MAX, "%s: cannot open file\n", s);
        r = 0;
    }
    else
    {
        if ((e = elf_begin(f, ELF_C_READ, NULL)) == NULL)
        {
            __mp_error(AT_MAX, "%s: %s\n", s, elf_errmsg(-1));
            r = 0;
        }
        else if ((t = __mp_addstring(&y->strings, s)) == NULL)
            r = 0;
        else
        {
            if (elf_kind(e) == ELF_K_AR)
                /* Normally we wouldn't ever need to read symbols from an
                 * archive library, but this is just provided for
                 * completeness.
                 */
                while ((r == 1) && (a = elf_begin(f, ELF_C_READ, e)))
                {
                    if ((h = elf_getarhdr(a)) == NULL)
                    {
                        __mp_error(AT_MAX, "%s: %s\n", s, elf_errmsg(-1));
                        r = 0;
                    }
                    else if (*h->ar_name != '/')
                        r = addsymbols(y, a, t, h->ar_name, b);
                    if (r == 1)
                        elf_next(a);
                    elf_end(a);
                }
            else
                r = addsymbols(y, e, NULL, t, b);
            elf_end(e);
        }
        close(f);
    }
#elif FORMAT == FORMAT_BFD
    /* Using the GNU BFD library allows us to read weird and wonderful
     * file formats that would otherwise be hard to support.  This is
     * probably a better choice to use than the in-built COFF implementation
     * but currently has no support for symbol sizes, so the ELF access
     * library is still worth using for ELF file formats.
     */
    bfd_init();
    if ((h = bfd_openr(s, NULL)) == NULL)
    {
        __mp_error(AT_MAX, "%s: %s\n", s, bfd_errmsg(bfd_get_error()));
        r = 0;
    }
    else
    {
        if (!bfd_check_format(h, bfd_object))
        {
            __mp_error(AT_MAX, "%s: %s\n", s, bfd_errmsg(bfd_get_error()));
            r = 0;
        }
        else if ((t = __mp_addstring(&y->strings, s)) == NULL)
            r = 0;
        else
            r = addsymbols(y, h, t, b);
        bfd_close(h);
    }
#endif /* FORMAT */
    return r;
}


/* Add any external or additional symbols to the symbol table.
 */

MP_GLOBAL int __mp_addextsymbols(symhead *y)
{
#if SYSTEM == SYSTEM_DGUX || SYSTEM == SYSTEM_DYNIX || \
    SYSTEM == SYSTEM_LINUX || SYSTEM == SYSTEM_SOLARIS
    Elf32_Dyn *d;
    dynamiclink *l;
#endif /* SYSTEM */

#if SYSTEM == SYSTEM_DGUX || SYSTEM == SYSTEM_DYNIX || \
    SYSTEM == SYSTEM_LINUX || SYSTEM == SYSTEM_SOLARIS
    /* This function liaises with the dynamic linker when a program is
     * dynamically linked in order to read symbols from any required shared
     * objects.
     */
    if ((&_DYNAMIC != NULL) && (d = (Elf32_Dyn *) _DYNAMIC))
    {
        for (l = NULL; d->d_tag != DT_NULL; d++)
            if (d->d_tag == DT_DEBUG)
            {
                if (!d->d_un.d_ptr)
                    l = NULL;
                else if (l = (dynamiclink *) *((unsigned int *) d->d_un.d_ptr +
                          1))
                    l = l->next;
                break;
            }
        while (l != NULL)
        {
            if ((l->base != 0) && (l->name != NULL) && (*l->name != '\0') &&
                !__mp_addsymbols(y, l->name, l->base))
                return 0;
            l = l->next;
        }
    }
#endif /* SYSTEM */
    return 1;
}


/* Attempt to tidy up the symbol table by correcting any potential errors or
 * conflicts from the symbols that have been read.
 */

MP_GLOBAL void __mp_fixsymbols(symhead *y)
{
    symnode *n, *p;
    void *l, *m;

    l = NULL;
    for (n = (symnode *) __mp_minimum(y->dtree.root); n != NULL; n = p)
    {
        /* If a symbol has a zero size and it is closely followed by another
         * symbol then the likelihood is that the symbol really has a non-zero
         * size so we fix that here.  This commonly occurs with system startup
         * files.
         */
        p = (symnode *) __mp_successor(&n->data.node);
        if ((n->data.size == 0) && (n->data.addr >= l))
            if ((p == NULL) || (n->data.file != p->data.file))
                n->data.size = 256;
            else
                n->data.size = (char *) p->data.addr - (char *) n->data.addr;
        if ((m = (char *) n->data.addr + n->data.size) > l)
            l = m;
    }
}


/* Protect the memory blocks used by the symbol table with the supplied access
 * permission.
 */

MP_GLOBAL int __mp_protectsymbols(symhead *y, memaccess a)
{
    symnode *n;

    for (n = (symnode *) __mp_minimum(y->itree.root); n != NULL;
         n = (symnode *) __mp_successor(&n->index.node))
        if (!__mp_memprotect(&y->heap->memory, n->index.block, n->index.size,
             a))
            return 0;
    return __mp_protectstrtab(&y->strings, a);
}


/* Attempt to find the symbol located at a particular address.
 */

MP_GLOBAL symnode *__mp_findsymbol(symhead *y, void *p)
{
    symnode *m, *n, *r;

    /* This function does not deal completely correctly with nested symbols
     * but that occurrence does not happen frequently so the current
     * implementation should suffice.
     */
    r = NULL;
    if (n = (symnode *) __mp_searchlower(y->dtree.root, (unsigned long) p))
    {
        while ((m = (symnode *) __mp_predecessor(&n->data.node)) &&
               (m->data.addr == n->data.addr))
            n = m;
        for (m = n; (n != NULL) && (n->data.addr == m->data.addr);
             n = (symnode *) __mp_successor(&n->data.node))
            if ((char *) n->data.addr + n->data.size > (char *) p)
            {
#if FORMAT == FORMAT_COFF
                /* We give precedence to global symbols, then local symbols.
                 */
                if ((r == NULL) || ((r->data.flags == C_STAT) &&
                     (n->data.flags == C_EXT)))
                    r = n;
#elif FORMAT == FORMAT_ELF32
                /* We give precedence to global symbols, then weak symbols,
                 * then local symbols.
                 */
                if ((r == NULL) || ((r->data.flags == STB_LOCAL) &&
                     ((n->data.flags == STB_WEAK) ||
                      (n->data.flags == STB_GLOBAL))) ||
                    ((r->data.flags == STB_WEAK) &&
                     (n->data.flags == STB_GLOBAL)))
                    r = n;
#elif FORMAT == FORMAT_BFD
                /* We give precedence to global symbols, then weak symbols,
                 * then local symbols.
                 */
                if ((r == NULL) || ((r->data.flags & BSF_LOCAL) &&
                     ((n->data.flags & BSF_WEAK) ||
                      (n->data.flags & BSF_GLOBAL))) ||
                    ((r->data.flags & BSF_WEAK) &&
                     (n->data.flags & BSF_GLOBAL)))
                    r = n;
#else /* FORMAT */
                r = n;
                break;
#endif /* FORMAT */
            }
    }
    return r;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
