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
 * Dbmalloc-compatible interface.  Implements Dbmalloc functions using
 * mpatrol.  Dbmalloc is copyright (C) 1990-1992 Conor P. Cahill.
 */


#include "config.h"
#include "dbmalloc.h"
#include <stdio.h>
#if TARGET == TARGET_WINDOWS
#include <io.h>
#else /* TARGET */
#include <unistd.h>
#endif /* TARGET */


#if MP_IDENT_SUPPORT
#ident "$Id: dbmalloc.c,v 1.1 2001-02-26 23:40:22 graeme Exp $"
#else /* MP_IDENT_SUPPORT */
static MP_CONST MP_VOLATILE char *heapdiff_id = "$Id: dbmalloc.c,v 1.1 2001-02-26 23:40:22 graeme Exp $";
#endif /* MP_IDENT_SUPPORT */


/* The structure used to pass information to the callback function from
 * __mp_iterate() when __mpt_dbmallocdump() and __mpt_dbmalloclist() are
 * called.
 */

typedef struct listinfo
{
    int file;            /* file descriptor */
    unsigned long event; /* upper event bound */
    int header : 1;      /* header output flag */
    int dump : 1;        /* dump output flag */
}
listinfo;


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* Specifies that more details should be shown when __mpt_dbmallocdump() is
 * called.
 */

static int malloc_detail;


/* Display the header for __mpt_dbmallocdump() and __mpt_dbmalloclist().
 */

static
void
printheader(int f)
{
    char b[70 + (sizeof(void *) * 2)];
    size_t i;

    for (i = 0; i < 22 + sizeof(void *); i++)
        b[i] = '*';
    strcpy(b + i, " Dump of Malloc Chain ");
    i += strlen(b + i);
    while (i < 68 + (sizeof(void *) * 2))
        b[i++] = '*';
    b[i++] = '\n';
    b[i] = '\0';
    if (f > 0)
        write(f, b, i);
    else
        __mp_printf(b);
    for (i = 0; i < sizeof(void *) - 4; i++)
        b[i] = ' ';
    strcpy(b + i, "POINTER");
    i += strlen(b + i);
    while (i < 4 + (sizeof(void *) * 2))
        b[i++] = ' ';
    strcpy(b + i, "FILE  WHERE         LINE      ALLOC        DATA     "
           "HEX DUMP\n");
    i += strlen(b + i);
    if (f > 0)
        write(f, b, i);
    else
        __mp_printf(b);
    for (i = 0; i < sizeof(void *) - 4; i++)
        b[i] = ' ';
    strcpy(b + i, "TO DATA");
    i += strlen(b + i);
    while (i < 5 + (sizeof(void *) * 2))
        b[i++] = ' ';
    strcpy(b + i, "ALLOCATED         NUMBER     FUNCT       LENGTH  "
           "OF BYTES 1-7\n");
    i += strlen(b + i);
    if (f > 0)
        write(f, b, i);
    else
        __mp_printf(b);
    for (i = 0; i < sizeof(void *) * 2; i++)
        b[i] = '-';
    b[i++] = ' ';
    while (i < 21 + (sizeof(void *) * 2))
        b[i++] = '-';
    b[i++] = ' ';
    while (i < 29 + (sizeof(void *) * 2))
        b[i++] = '-';
    b[i++] = ' ';
    while (i < 44 + (sizeof(void *) * 2))
        b[i++] = '-';
    b[i++] = ' ';
    while (i < 52 + (sizeof(void *) * 2))
        b[i++] = '-';
    b[i++] = ' ';
    while (i < 67 + (sizeof(void *) * 2))
        b[i++] = '-';
    b[i++] = '\n';
    b[i] = '\0';
    if (f > 0)
        write(f, b, i);
    else
        __mp_printf(b);
}


/* The callback function that is called by __mp_iterate() for every heap
 * allocation that has changed since a specified heap event.
 */

static
int
callback(MP_CONST void *p, void *t)
{
    char b[69 + (sizeof(void *) * 2)];
    char m[64];
    listinfo *i;
    __mp_allocstack *a;
    __mp_allocinfo d;
    __mp_symbolinfo s;
    size_t j, n;

    if (!__mp_info(p, &d))
        return 0;
    i = (listinfo *) t;
    if ((d.event <= i->event) && (!d.freed || (i->dump && malloc_detail)) &&
        (!d.marked || i->dump))
    {
        if (!i->header)
        {
            printheader(i->file);
            i->header = 1;
        }
        sprintf(b, "%0*lX ", sizeof(void *) * 2, d.block);
        n = strlen(b);
        if (d.file != NULL)
        {
            sprintf(m, "%7lu", d.line);
            sprintf(b + n, "%-20.20s %7.7s ", d.file, m);
        }
        else
            sprintf(b + n, "%-28s ", "unknown");
        n += strlen(b + n);
        sprintf(m, "%s(%lu)", __mp_function(d.type), d.alloc);
        sprintf(b + n, "%-14.14s ", m);
        n += strlen(b + n);
        sprintf(m, "%7lu", d.size);
        sprintf(b + n, "%7.7s ", m);
        n += strlen(b + n);
        for (j = 0; (j < 7) && (j < d.size); j++)
            sprintf(m + (j << 1), "%02X", ((unsigned char *) d.block)[j]);
        m[j << 1] = '\0';
        sprintf(b + n, "%s\n", m);
        n += strlen(b + n);
        if (i->file > 0)
            write(i->file, b, n);
        else
            __mp_printf(b);
        for (a = d.stack; a != NULL; a = a->next)
        {
            for (n = 0; n <= sizeof(void *) * 2; n++)
                b[n] = ' ';
            strcpy(b + n, "-> ");
            n += 3;
            if (__mp_syminfo(a->addr, &s))
                if (i->file > 0)
                {
                    write(i->file, b, n);
                    write(i->file, s.name, strlen(s.name));
                    if (s.file != NULL)
                    {
                        write(i->file, " in ", 4);
                        write(i->file, s.file, strlen(s.file));
                        sprintf(b, "(%lu)\n", s.line);
                        write(i->file, b, strlen(b));
                    }
                    else
                        write(i->file, "\n", 1);
                }
                else if (s.file != NULL)
                    __mp_printf("%s%s in %s(%lu)\n", b, s.name, s.file, s.line);
                else
                    __mp_printf("%s%s\n", b, s.name);
            else
            {
                sprintf(b + n, "%0*lX\n", sizeof(void *) * 2, a->addr);
                n += strlen(b + n);
                if (i->file > 0)
                    write(i->file, b, n);
                else
                    __mp_printf(b);
            }
        }
        return 1;
    }
    return 0;
}


/* Set a malloc library option.
 */

int
__mpt_dbmallocoption(int c, union dbmalloptarg *v)
{
    int r;

    r = 0;
    switch (c)
    {
      case MALLOC_REUSE:
        r = __mp_setoption(MP_OPT_NOFREE, v->i);
        break;
      case MALLOC_DETAIL:
        malloc_detail = v->i;
        break;
      default:
        r = 1;
        break;
    }
    return r;
}


/* Verify that the malloc chain is still intact and the heap has not been
 * corrupted.
 */

int
__mpt_dbmallocchaincheck(int f, MP_CONST char *s, MP_CONST char *t,
                         unsigned long u)
{
    __mp_checkheap(s, t, u);
    return 0;
}


/* Display all of the heap allocations and their associated data.
 */

void
__mpt_dbmallocdump(int f)
{
    listinfo i;

    i.file = f;
    i.event = __mp_snapshot();
    i.header = 0;
    i.dump = 1;
    __mp_iterate(callback, &i, 0);
    if (f > 0)
        write(f, "\n", 1);
    else if (i.header)
        __mp_printf("\n");
}


/* Display some of the heap allocations and their associated data.
 */

void
__mpt_dbmalloclist(int f, unsigned long l, unsigned long u)
{
    listinfo i;

    i.file = f;
    if (l <= u)
        i.event = u;
    else
    {
        i.event = l;
        l = u;
        u = i.event;
    }
    i.header = 0;
    i.dump = 0;
    __mp_iterate(callback, &i, l);
    if (f > 0)
        write(f, "\n", 1);
    else if (i.header)
        __mp_printf("\n");
}


/* Return the number of bytes of heap memory currently in use and optionally
 * return the current malloc history id.
 */

unsigned long
__mpt_dbmallocinuse(unsigned long *h)
{
    __mp_heapinfo i;
    unsigned long t;

    if (__mp_stats(&i))
        t = i.atotal;
    else
        t = 0;
    if (h != NULL)
        *h = __mp_snapshot();
    return t;
}


/* Return the size in bytes of the memory allocation that contains a specified
 * address.
 */

size_t
__mpt_dbmallocsize(MP_CONST void *p)
{
    __mp_allocinfo i;
    size_t t;

    if (__mp_info(p, &i) && !i.freed)
        t = i.size;
    else
        t = (size_t) -1;
    return t;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
