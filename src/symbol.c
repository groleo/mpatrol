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
 * in order to obtain symbols from shared objects.  A very useful book on the
 * various different object file and library formats that exist is Linkers &
 * Loaders, First Edition by John R. Levine (Morgan Kaufmann Publishers, 2000,
 * ISBN 1-558-60496-0).
 */


#include "symbol.h"
#include "diag.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#if FORMAT == FORMAT_COFF || FORMAT == FORMAT_XCOFF || \
    FORMAT == FORMAT_ELF32 || FORMAT == FORMAT_ELF64 || FORMAT == FORMAT_BFD
#include <fcntl.h>
#include <unistd.h>
#if FORMAT == FORMAT_COFF || FORMAT == FORMAT_XCOFF
#include <a.out.h>
#if SYSTEM == SYSTEM_AIX
#ifndef ISCOFF
#define ISCOFF(m) (((m) == U800TOCMAGIC) || ((m) == U802TOCMAGIC))
#endif /* ISCOFF */
#elif SYSTEM == SYSTEM_LYNXOS
#include <coff.h>
#ifndef ISCOFF
#define ISCOFF(m) ((m) == COFFMAGICNUM)
#endif /* ISCOFF */
#ifndef ISFCN
#define ISFCN(t) (((t) & 0x30) == (DT_FCN << 4))
#endif /* ISFCN */
#ifndef n_name
#define n_name _n._n_name
#endif /* n_name */
#endif /* SYSTEM */
#elif FORMAT == FORMAT_ELF32 || FORMAT == FORMAT_ELF64
#include <libelf.h>
#elif FORMAT == FORMAT_BFD
#include <bfd.h>
#endif /* FORMAT */
#endif /* FORMAT */
#if TARGET == TARGET_UNIX
#if SYSTEM == SYSTEM_AIX
/* The shared libraries that an AIX executable has loaded can be obtained via
 * the loadquery() function.
 */
#include <sys/ldr.h>
#elif SYSTEM == SYSTEM_DGUX || SYSTEM == SYSTEM_DYNIX || \
      SYSTEM == SYSTEM_LINUX || SYSTEM == SYSTEM_SINIX || \
      SYSTEM == SYSTEM_SOLARIS || SYSTEM == SYSTEM_UNIXWARE
/* Despite the fact that Linux is now ELF-based, libelf seems to be missing from
 * many recent distributions and so we must use the GNU BFD library to read the
 * symbols from the object files and libraries.  However, we still need the ELF
 * definitions for reading the internal structures of the dynamic linker.
 */
#include <elf.h>
#elif SYSTEM == SYSTEM_HPUX
/* The HP/UX dynamic linker support routines are available for use even by
 * statically linked programs.
 */
#include <dl.h>
#elif SYSTEM == SYSTEM_IRIX
/* IRIX doesn't have a conventional SVR4 dynamic linker and so does not have the
 * same interface for accessing information about any required shared objects at
 * run-time.
 */
#include <obj.h>
#include <obj_list.h>
#endif /* SYSTEM */
#elif TARGET == TARGET_WINDOWS
/* We use the imagehlp library on Windows platforms to obtain information about
 * the symbols loaded from third-party and system DLLs.  We can also use it to
 * obtain information about any symbols contained in the executable file and
 * program database if COFF or CodeView debugging information is being used.
 */
#include <windows.h>
#include <imagehlp.h>
#endif /* TARGET */


#if MP_IDENT_SUPPORT
#ident "$Id: symbol.c,v 1.29 2000-06-26 22:57:22 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#if TARGET == TARGET_UNIX
#if SYSTEM == SYSTEM_DGUX || SYSTEM == SYSTEM_DYNIX || \
    SYSTEM == SYSTEM_LINUX || SYSTEM == SYSTEM_SINIX || \
    SYSTEM == SYSTEM_SOLARIS || SYSTEM == SYSTEM_UNIXWARE
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
#endif /* TARGET */


#if TARGET == TARGET_UNIX
#if SYSTEM == SYSTEM_DGUX || SYSTEM == SYSTEM_DYNIX || \
    SYSTEM == SYSTEM_LINUX || SYSTEM == SYSTEM_SINIX || \
    SYSTEM == SYSTEM_SOLARIS || SYSTEM == SYSTEM_UNIXWARE
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
#elif SYSTEM == SYSTEM_IRIX
/* This structure represents an N32 ABI shared object as opposed to an O32 ABI
 * shared object, and is defined on IRIX 6.0 and above platforms as
 * Elf32_Obj_Info.  In order for us to compile on earlier IRIX platforms we
 * define this structure here, but under a different name so as to avoid a
 * naming clash on IRIX 6.0 and later.
 */

typedef struct objectinfo
{
    Elf32_Word magic;  /* N32 ABI shared object */
    Elf32_Word size;   /* size of this structure */
    Elf32_Addr next;   /* next shared object */
    Elf32_Addr prev;   /* previous shared object */
    Elf32_Addr ehdr;   /* address of object file header */
    Elf32_Addr ohdr;   /* original address of object file header */
    Elf32_Addr name;   /* filename of shared object */
    Elf32_Word length; /* length of filename of shared object */
}
objectinfo;
#endif /* SYSTEM */
#elif TARGET == TARGET_WINDOWS
/* This structure is used to pass information to the callback function
 * called by SymEnumerateSymbols().
 */

typedef struct syminfo
{
    symhead *syms; /* pointer to symbol table */
    char *file;    /* filename of module */
}
syminfo;


/* This structure is used to pass information to the callback function
 * called by SymEnumerateModules().
 */

typedef struct modinfo
{
    symhead *syms; /* pointer to symbol table */
    size_t index;  /* index of module */
    char libs;     /* only read DLLs */
}
modinfo;
#endif /* TARGET */


#if FORMAT == FORMAT_BFD
/* This structure is used to maintain a list of access library handles for
 * the purposes of mapping return addresses to line numbers.
 */

typedef struct objectfile
{
    struct objectfile *next;  /* pointer to next object file */
    bfd *file;                /* access library handle */
    asymbol **symbols;        /* pointer to symbols */
    size_t base;              /* virtual address of object file */
}
objectfile;


/* This structure is used when searching for source locations corresponding
 * to virtual addresses by looking at debugging information embedded in an
 * object file.
 */

typedef struct sourcepos
{
    bfd_vma addr;       /* virtual address to search for */
    asymbol **symbols;  /* pointer to symbols */
    size_t base;        /* virtual address of object file */
    char *func;         /* function name */
    char *file;         /* file name */
    unsigned int line;  /* line number */
    char found;         /* found flag */
}
sourcepos;
#endif /* FORMAT */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


#if TARGET == TARGET_UNIX
#if SYSTEM == SYSTEM_DGUX || SYSTEM == SYSTEM_DYNIX || \
    SYSTEM == SYSTEM_LINUX || SYSTEM == SYSTEM_SINIX || \
    SYSTEM == SYSTEM_SOLARIS || SYSTEM == SYSTEM_UNIXWARE
/* The declaration of the _DYNAMIC symbol, which allows us direct access to the
 * dynamic linker's internal data structures.  We make it have weak visibility
 * so that it is always defined, even in the statically linked case.  It is
 * declared as a function because some compilers only allow weak function
 * symbols.  Another alternative would be to declare it as a common symbol, but
 * that wouldn't work under C++ and many libc shared objects have it defined as
 * a text symbol.
 */

#pragma weak _DYNAMIC
void _DYNAMIC(void);
#elif SYSTEM == SYSTEM_IRIX
/* The __rld_obj_head symbol is always defined in IRIX and points to the first
 * entry in a list of shared object files that are required by the program.  For
 * statically linked programs this will either be NULL or will only contain the
 * entry for the program itself.
 */
extern struct obj_list *__rld_obj_head;
#endif /* SYSTEM */
#endif /* TARGET */


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
    y->hhead = y->htail = NULL;
    y->lineinfo = 0;
}


/* Close all access library handles.
 */

MP_GLOBAL void __mp_closesymbols(symhead *y)
{
#if FORMAT == FORMAT_BFD
    objectfile *n, *p;
#endif /* FORMAT */

#if FORMAT == FORMAT_BFD
    for (n = (objectfile *) y->hhead; n != NULL; n = p)
    {
        p = n->next;
        bfd_close(n->file);
        free(n->symbols);
        free(n);
    }
#endif /* FORMAT */
    y->hhead = y->htail = NULL;
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
     * some more memory for them.  An extra MP_ALLOCFACTOR pages of memory
     * should suffice.
     */
    if ((n = (symnode *) __mp_getslot(&y->table)) == NULL)
    {
        if ((p = __mp_heapalloc(y->heap, y->heap->memory.page * MP_ALLOCFACTOR,
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


#if FORMAT == FORMAT_COFF || FORMAT == FORMAT_XCOFF
/* Allocate a new symbol node for a given COFF or XCOFF symbol.
 */

static int addsymbol(symhead *y, SYMENT *p, char *f, char *s)
{
    AUXENT *e;
    symnode *n;
    char *r;

    /* We don't bother storing a symbol which has no name or whose name
     * contains a '$', '@' or a '.'.  However, in XCOFF the symbol name is
     * likely to be the name of a CSECT beginning with a '.' and not the
     * original name of the function, so we skip the first character.  We
     * also don't allocate a symbol node for symbols which have a virtual
     * address of zero and we only remember symbols that are declared
     * statically or externally visible.
     */
    if ((s != NULL) &&
#if FORMAT == FORMAT_XCOFF
        (*s++ == '.') && (strcmp(s, "text") != 0) &&
#else /* FORMAT */
        (*s != '\0') &&
#endif /* FORMAT */
        !strpbrk(s, "$@.") && (p->n_value > 0) &&
#if FORMAT == FORMAT_XCOFF
        (ISFCN(p->n_type) || (p->n_sclass == C_EXT)))
#else /* FORMAT */
        ((p->n_sclass == C_STAT) || (p->n_sclass == C_EXT)))
#endif /* FORMAT */
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
        n->data.index = 0;
        n->data.offset = 0;
        /* The linkage information is required for when we look up a symbol.
         */
        n->data.flags = p->n_sclass;
    }
    return 1;
}
#elif FORMAT == FORMAT_ELF32
/* Allocate a new symbol node for a given ELF32 symbol.
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
        n->data.index = 0;
        n->data.offset = 0;
        /* The linkage information is required for when we look up a symbol.
         */
        n->data.flags = ELF32_ST_BIND(p->st_info);
    }
    return 1;
}
#elif FORMAT == FORMAT_ELF64
/* Allocate a new symbol node for a given ELF64 symbol.
 */

static int addsymbol(symhead *y, Elf64_Sym *p, char *f, char *s, size_t b)
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
        (((t = ELF64_ST_TYPE(p->st_info)) == STT_NOTYPE) || (t == STT_FUNC)))
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
        n->data.index = 0;
        n->data.offset = 0;
        /* The linkage information is required for when we look up a symbol.
         */
        n->data.flags = ELF64_ST_BIND(p->st_info);
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
     * contains a '$', '@' or a '.'.  However, in XCOFF the symbol name is
     * likely to be the name of a CSECT beginning with a '.' and not the
     * original name of the function, so we skip the first character.  In
     * addition, the HP/UX $START$ symbol contains dollar characters but we
     * don't want to bother allowing any other symbols containing dollars.
     * We also don't allocate a symbol node for symbols which have a virtual
     * address of zero and we only remember symbols that are declared
     * statically, externally or weakly visible.
     */
    if ((s != NULL) &&
#if (SYSTEM == SYSTEM_AIX || SYSTEM == SYSTEM_LYNXOS) && \
    (ARCH == ARCH_POWER || ARCH == ARCH_POWERPC)
        (*s++ == '.') && (strcmp(s, "text") != 0) &&
#else /* SYSTEM && ARCH */
        (*s != '\0') &&
#endif /* SYSTEM && ARCH */
#if SYSTEM == SYSTEM_HPUX
        ((strcmp(s, "$START$") == 0) || !strpbrk(s, "$@.")) &&
#else /* SYSTEM */
        !strpbrk(s, "$@.") &&
#endif /* SYSTEM */
        (a > 0) && (p->flags & (BSF_LOCAL | BSF_GLOBAL | BSF_WEAK)))
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
        n->data.index = 0;
        n->data.offset = 0;
        /* The linkage information is required for when we look up a symbol.
         */
        n->data.flags = p->flags;
    }
    return 1;
}
#endif /* FORMAT */


#if TARGET == TARGET_WINDOWS
/* The callback function called to allocate a new symbol node for each
 * symbol located in a module by the imagehlp library.
 */

static int __stdcall addsym(char *s, unsigned long a, unsigned long l, void *p)
{
    syminfo *i;
    symhead *y;
    symnode *n;
    char *r;

    i = (syminfo *) p;
    y = i->syms;
    /* We don't bother storing a symbol which has no name or which has a
     * virtual address of zero.  Unfortunately, the imagehlp library does
     * not provide information about the types of symbols so we will just
     * have to add any symbols representing objects here as well.
     */
    if ((s != NULL) && (*s != '\0') && (a > 0))
    {
        if ((n = getsymnode(y)) == NULL)
            return 0;
        if ((r = __mp_addstring(&y->strings, s)) == NULL)
        {
            __mp_freeslot(&y->table, n);
            return 0;
        }
        __mp_treeinsert(&y->dtree, &n->data.node, a);
        n->data.file = i->file;
        n->data.name = r;
        n->data.addr = (void *) a;
        n->data.size = l;
        n->data.index = 0;
        n->data.offset = 0;
        /* Unfortunately, the imagehlp library does not provide information
         * about the visibility of symbols.
         */
        n->data.flags = 0;
    }
    return 1;
}
#endif /* TARGET */


#if FORMAT == FORMAT_COFF || FORMAT == FORMAT_XCOFF
/* Allocate a set of symbol nodes for a COFF or XCOFF executable file.
 */

static int addsymbols(symhead *y, char *e, char *f, size_t b)
{
    char n[SYMNMLEN + 1];
    FILHDR *o;
    SCNHDR *h;
    SYMENT *p;
    char *m, *s;
    size_t i, t;

    /* Check that we have a valid COFF or XCOFF executable file.
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
        if ((h[i].s_flags & STYP_TEXT) ||
            (strncmp(h[i].s_name, ".text", 8) == 0))
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
    for (i = 0; i < o->f_nsyms; i += p->n_numaux + 1,
         p = (SYMENT *) ((char *) p + (p->n_numaux + 1) * SYMESZ))
        /* We only need to bother looking at text symbols.
         */
        if (p->n_scnum == t)
        {
            if (p->n_zeroes)
            {
                /* Symbol name is stored in structure.
                 */
                strncpy(n, p->n_name, SYMNMLEN);
                n[SYMNMLEN] = '\0';
                s = n;
            }
            else
                /* Symbol name is stored in string table.
                 */
                s = m + p->n_offset;
            if (!addsymbol(y, p, f, s))
                return 0;
        }
    return 1;
}
#elif FORMAT == FORMAT_ELF32
/* Allocate a set of symbol nodes for an ELF32 object file.
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
#elif FORMAT == FORMAT_ELF64
/* Allocate a set of symbol nodes for an ELF64 object file.
 */

static int addsymbols(symhead *y, Elf *e, char *a, char *f, size_t b)
{
    Elf_Scn *s;
    Elf64_Shdr *h;
    Elf64_Sym *p;
    Elf_Data *d;
    char *m;
    size_t i, l, n, t;

    /* Look for the symbol table.
     */
    for (s = elf_nextscn(e, NULL), d = NULL; s != NULL; s = elf_nextscn(e, s))
        if ((h = elf64_getshdr(s)) && (h->sh_type == SHT_SYMTAB) &&
            (d = elf_getdata(s, NULL)))
            break;
    if (d == NULL)
        /* If we couldn't find the symbol table then it is likely that the file
         * has been stripped.  However, if the file was dynamically linked then
         * we may be able to obtain some symbols from its dynamic symbol table.
         */
        for (s = elf_nextscn(e, NULL), d = NULL; s != NULL;
             s = elf_nextscn(e, s))
            if ((h = elf64_getshdr(s)) && (h->sh_type == SHT_DYNSYM) &&
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
    if (((s = elf_getscn(e, t)) == NULL) || ((h = elf64_getshdr(s)) == NULL) ||
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
    p = (Elf64_Sym *) d->d_buf + 1;
    l = d->d_size / sizeof(Elf64_Sym);
    /* Cycle through every symbol contained in the object file.
     */
    for (i = 1; i < l; i++, p++)
        /* We don't need to bother looking at undefined, absolute or common
         * symbols, and we only need to store non-data symbols.
         */
        if (((n = p->st_shndx) != SHN_UNDEF) && (n != SHN_ABS) &&
            (n != SHN_COMMON) && (s = elf_getscn(e, n)) &&
            (h = elf64_getshdr(s)) && (h->sh_flags & SHF_EXECINSTR))
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
    /* If we are making use of line number information in the object files then
     * we store the symbol table along with the access library handle; otherwise
     * we can free the symbol table now.
     */
    if (y->lineinfo && (r == 1))
        ((objectfile *) y->htail)->symbols = p;
    else
        free(p);
    return r;
}
#endif /* FORMAT */


#if TARGET == TARGET_WINDOWS
/* The callback function called to allocate a set of symbol nodes for each
 * module located by the imagehlp library.
 */

static int __stdcall addsyms(char *f, unsigned long b, void *p)
{
    modinfo *i;
    symhead *y;
    char *t;
    syminfo s;
    IMAGEHLP_MODULE m;
    int r;

    r = 1;
    i = (modinfo *) p;
    y = i->syms;
    /* The executable file is the first module, so we only want to examine it
     * if we are not looking for external symbols.  The DLLs are the subsequent
     * modules, so we only want to examine them if we are looking for external
     * symbols.
     */
    if ((!i->libs && !i->index) || (i->libs && i->index))
    {
        /* Attempt to determine the full path of the current module.
         */
        m.SizeOfStruct = sizeof(IMAGEHLP_MODULE);
        if (SymGetModuleInfo(GetCurrentProcess(), b, &m))
            f = m.LoadedImageName;
        if ((t = __mp_addstring(&y->strings, f)) == NULL)
            r = 0;
        else
        {
            /* Cycle through every symbol contained in the module.
             */
            s.syms = y;
            s.file = t;
            r = SymEnumerateSymbols(GetCurrentProcess(), b, addsym, &s);
        }
    }
    i->index++;
    return r;
}
#endif /* TARGET */


/* Read a file and add all relevant symbols contained within it to the
 * symbol table.
 */

MP_GLOBAL int __mp_addsymbols(symhead *y, char *s, size_t b)
{
#if FORMAT == FORMAT_COFF || FORMAT == FORMAT_XCOFF || \
    FORMAT == FORMAT_ELF32 || FORMAT == FORMAT_ELF64 || FORMAT == FORMAT_BFD
#if FORMAT == FORMAT_COFF || FORMAT == FORMAT_XCOFF
    char *m;
    off_t o;
#elif FORMAT == FORMAT_ELF32 || FORMAT == FORMAT_ELF64
    Elf *a, *e;
    Elf_Arhdr *h;
#elif FORMAT == FORMAT_BFD
    objectfile *p, *q;
    bfd *h;
#endif /* FORMAT */
    char *t;
    int f;
#elif FORMAT == FORMAT_PE
    modinfo m;
#endif /* FORMAT */
    int r;

    r = 1;
#if TARGET == TARGET_WINDOWS
    /* We always want to initialise the imagehlp library here since we will
     * be using it to obtain the symbols from any loaded DLLs later on and
     * possibly also from the executable file if we are not using any other
     * object file access library.  In any case we can set the demangling
     * option in the imagehlp library and also instruct it to load line number
     * information if the USEDEBUG option is given.
     */
    if (y->lineinfo)
        SymSetOptions(SYMOPT_UNDNAME | SYMOPT_LOAD_LINES);
    else
        SymSetOptions(SYMOPT_UNDNAME);
    SymInitialize(GetCurrentProcess(), NULL, 1);
#endif /* TARGET */
#if FORMAT == FORMAT_COFF || FORMAT == FORMAT_XCOFF
    /* This is a very simple, yet portable, way to read symbols from COFF
     * and XCOFF executable files.
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
#elif FORMAT == FORMAT_ELF32 || FORMAT == FORMAT_ELF64
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
     * probably a better choice to use than the in-built COFF and XCOFF
     * implementations but currently has no support for symbol sizes, so
     * the ELF access library is still worth using for ELF file formats,
     * but the BFD library comes with support for debugging information.
     * So take your pick!
     */
    p = NULL;
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
        else if (y->lineinfo &&
                 ((p = (objectfile *) malloc(sizeof(objectfile))) == NULL))
            r = 0;
        else if ((t = __mp_addstring(&y->strings, s)) == NULL)
            r = 0;
        else
        {
            if (y->lineinfo)
            {
                if (y->hhead == NULL)
                    y->hhead = p;
                else
                {
                    q = (objectfile *) y->htail;
                    q->next = p;
                }
                y->htail = p;
                p->next = NULL;
                p->file = h;
                p->symbols = NULL;
                p->base = b;
            }
            r = addsymbols(y, h, t, b);
            if (y->lineinfo && (r == 0))
                if (y->hhead == p)
                    y->hhead = y->htail = NULL;
                else
                {
                    y->htail = q;
                    q->next = NULL;
                }
        }
        if (!y->lineinfo || (r == 0))
        {
            if (p != NULL)
                free(p);
            bfd_close(h);
        }
    }
#elif FORMAT == FORMAT_PE
    /* We only want to obtain the symbols from the executable file using the
     * imagehlp library if we are not using another object file access library,
     * such as GNU BFD.
     */
    m.syms = y;
    m.index = 0;
    m.libs = 0;
    r = SymEnumerateModules(GetCurrentProcess(), addsyms, &m);
#endif /* FORMAT */
    return r;
}


/* Add any external or additional symbols to the symbol table.
 */

MP_GLOBAL int __mp_addextsymbols(symhead *y)
{
#if TARGET == TARGET_UNIX
#if SYSTEM == SYSTEM_AIX
    static char b[4096];
    struct ld_info *l;
#elif SYSTEM == SYSTEM_DGUX || SYSTEM == SYSTEM_DYNIX || \
      SYSTEM == SYSTEM_LINUX || SYSTEM == SYSTEM_SINIX || \
      SYSTEM == SYSTEM_SOLARIS || SYSTEM == SYSTEM_UNIXWARE
    Elf32_Dyn *d;
    dynamiclink *l;
#elif SYSTEM == SYSTEM_HPUX
    struct shl_descriptor d;
    size_t i;
    unsigned int o;
#elif SYSTEM == SYSTEM_IRIX
    struct obj_list *l;
    struct obj *o;
    objectinfo *i;
    char *s;
    size_t b;
#endif /* SYSTEM */
#elif TARGET == TARGET_WINDOWS
    modinfo m;
#endif /* TARGET */

    /* This function liaises with the dynamic linker when a program is
     * dynamically linked in order to read symbols from any required shared
     * objects.
     */
#if TARGET == TARGET_UNIX
#if SYSTEM == SYSTEM_AIX
    if (loadquery(L_GETINFO, b, sizeof(b)) != -1)
    {
        /* We skip past the first item on the list since it represents the
         * executable file.
         */
        l = (struct ld_info *) b;
        while (l = l->ldinfo_next ? (struct ld_info *)
                                    ((char *) l + l->ldinfo_next) : NULL)
            if ((*l->ldinfo_filename != '\0') &&
                !__mp_addsymbols(y, l->ldinfo_filename,
                                 (unsigned long) l->ldinfo_textorg))
                return 0;
    }
#elif SYSTEM == SYSTEM_DGUX || SYSTEM == SYSTEM_DYNIX || \
      SYSTEM == SYSTEM_LINUX || SYSTEM == SYSTEM_SINIX || \
      SYSTEM == SYSTEM_SOLARIS || SYSTEM == SYSTEM_UNIXWARE
    if ((&_DYNAMIC != NULL) && (d = (Elf32_Dyn *) _DYNAMIC))
    {
        /* Search for the DT_DEBUG tag in the _DYNAMIC symbol.
         */
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
#elif SYSTEM == SYSTEM_HPUX
    /* An index of -1 indicates the dynamic linker and an index of 0 indicates
     * the main executable program.  We are interested in all other object files
     * that the program depends on.
     */
    for (i = 1; shl_get_r(i, &d) != -1; i++)
    {
        /* Determine the offset of the first text symbol in the library.  This
         * is normally 0x1000 but may be something else on later systems.  The
         * handle structure is not documented anywhere, but the fourth word
         * appears to contain the information we need, based on trial and error.
         */
        if (d.handle != NULL)
            o = ((unsigned int *) d.handle)[3];
        else
            o = 0;
        if ((d.filename[0] != '\0') &&
            !__mp_addsymbols(y, d.filename, d.tstart - o))
            return 0;
    }
#elif SYSTEM == SYSTEM_IRIX
    if (l = __rld_obj_head)
        while (l = l->next)
        {
            /* Determine if the shared object we are looking at is an O32 ABI
             * object or an N32 ABI object.
             */
            i = (objectinfo *) l->data;
            if (i->magic == 0xFFFFFFFF)
            {
                s = (char *) i->name;
                b = (long) i->ehdr - (long) i->ohdr;
                if ((s != NULL) && (*s != '\0') && !__mp_addsymbols(y, s, b))
                    return 0;
            }
            else
            {
                o = (struct obj *) i;
                b = (long) o->o_text_start - (long) o->o_base_address;
                if ((o->o_path != NULL) && (*o->o_path != '\0') &&
                    !__mp_addsymbols(y, o->o_path, b))
                    return 0;
            }
        }
#endif /* SYSTEM */
#elif TARGET == TARGET_WINDOWS
    /* The imagehlp library allows us to locate the symbols contained in
     * all of the loaded DLLs without having to actually read the files
     * themselves.
     */
    m.syms = y;
    m.index = 0;
    m.libs = 1;
    if (!SymEnumerateModules(GetCurrentProcess(), addsyms, &m))
        return 0;
#endif /* TARGET */
    return 1;
}


/* Attempt to tidy up the symbol table by correcting any potential errors or
 * conflicts from the symbols that have been read.
 */

MP_GLOBAL void __mp_fixsymbols(symhead *y)
{
    symnode *n, *p;
    void *l, *m;
#if TARGET == TARGET_AMIGA
    unsigned long o;
#endif /* TARGET */

#if TARGET == TARGET_AMIGA
    o = 0;
#endif /* TARGET */
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
#if TARGET == TARGET_AMIGA
        /* On AmigaOS, sections are scatter-loaded into memory and will occupy
         * different addresses each time a program is loaded.  One easy way to
         * determine the run-time address of each function is to find the
         * offset for one function and add it to all functions, but this means
         * that this function must be compiled with symbolic information and
         * assumes that all text symbols are in the one hunk.
         */
        if ((o == 0) && (strcmp(n->data.name, "___mp_fixsymbols") == 0))
            o = (char *) __mp_fixsymbols - (char *) n->data.addr;
#endif /* TARGET */
    }
#if TARGET == TARGET_AMIGA
    if (o != 0)
    {
#if FORMAT == FORMAT_BFD
        if (y->hhead != NULL)
            ((objectfile *) y->hhead)->base = o;
#endif /* FORMAT */
        for (n = (symnode *) __mp_minimum(y->dtree.root); n != NULL;
             n = (symnode *) __mp_successor(&n->index.node))
        {
            n->data.node.key += o;
            n->data.addr = (char *) n->data.addr + o;
        }
    }
#endif /* TARGET */
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
#elif FORMAT == FORMAT_XCOFF
                /* We give precedence to global symbols, then hidden external
                 * symbols, then local symbols.
                 */
                if ((r == NULL) || ((r->data.flags & C_STAT) &&
                     ((n->data.flags & C_HIDEXT) || (n->data.flags & C_EXT))) ||
                    ((r->data.flags & C_HIDEXT) && (n->data.flags & C_EXT)))
                    r = n;
#elif FORMAT == FORMAT_ELF32 || FORMAT == FORMAT_ELF64
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


#if FORMAT == FORMAT_BFD
/* Search a BFD section for a specific virtual memory address and attempt
 * to match it up with source position information.
 */

static void findsource(bfd *h, asection *p, void *d)
{
    sourcepos *s;
    bfd_vma v;
    size_t l;

    s = (sourcepos *) d;
    if (!s->found && !bfd_is_und_section(p) &&
        !bfd_is_abs_section(p) && !bfd_is_com_section(p) &&
        (p->flags & SEC_ALLOC) && (p->flags & SEC_CODE))
    {
        v = bfd_section_vma(h, p) + s->base;
        l = bfd_section_size(h, p);
        if ((s->addr >= v) && (s->addr < v + l))
        {
            s->file = s->func = NULL;
            s->line = 0;
            s->found = bfd_find_nearest_line(h, p, s->symbols, s->addr - v,
                                             (const char **) &s->file,
                                             (const char **) &s->func,
                                             &s->line);
        }
    }
}
#endif /* FORMAT */


/* Attempt to find the source correspondence for a machine instruction located
 * at a particular address.
 */

MP_GLOBAL int __mp_findsource(symhead *y, void *p, char **s, char **t,
                              unsigned long *u)
{
#if FORMAT == FORMAT_BFD
    objectfile *n;
    sourcepos m;
#endif /* FORMAT */
#if TARGET == TARGET_WINDOWS
    static char b[1024];
    IMAGEHLP_SYMBOL *i;
    IMAGEHLP_LINE l;
    unsigned long d;
#endif /* TARGET */

#if FORMAT == FORMAT_BFD
    m.addr = (bfd_vma) p;
    m.found = 0;
    for (n = (objectfile *) y->hhead; n != NULL; n = n->next)
    {
        m.symbols = n->symbols;
        m.base = n->base;
        bfd_map_over_sections(n->file, findsource, &m);
        if (m.found)
        {
            *s = m.func;
            *t = m.file;
            *u = m.line;
            return 1;
        }
    }
#endif /* FORMAT */
    *s = *t = NULL;
    *u = 0;
#if TARGET == TARGET_WINDOWS
    if (y->lineinfo)
    {
        i = (IMAGEHLP_SYMBOL *) b;
        i->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
        i->MaxNameLength = sizeof(b) - sizeof(IMAGEHLP_SYMBOL);
        if (SymGetSymFromAddr(GetCurrentProcess(), (unsigned long) p, &d, i))
            *s = i->Name;
        l.SizeOfStruct = sizeof(IMAGEHLP_LINE);
        if (SymGetLineFromAddr(GetCurrentProcess(), (unsigned long) p, &d, &l))
        {
            *t = l.FileName;
            *u = l.LineNumber;
        }
        if ((*s != NULL) || (*t != NULL))
            return 1;
    }
#endif /* TARGET */
    return 0;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
