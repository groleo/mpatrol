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
 * A tool designed to read a profiling output file produced by the mpatrol
 * library and display the profiling information that was obtained.
 */


#include "tree.h"
#include "getopt.h"
#include "version.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#if MP_IDENT_SUPPORT
#ident "$Id: mprof.c,v 1.19 2000-10-03 16:17:00 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#define VERSION "1.1" /* the current version of this program */


/* Structure containing statistics about the counts and totals of all of the
 * small, medium, large and extra large allocations and deallocations for a
 * particular call site.
 */

typedef struct profiledata
{
    size_t acount[4]; /* total numbers of allocations */
    size_t dcount[4]; /* total numbers of deallocations */
    size_t atotal[4]; /* total numbers of allocated bytes */
    size_t dtotal[4]; /* total numbers of deallocated bytes */
}
profiledata;


/* Structure containing profiling details for a function in a call stack.
 */

typedef struct profilenode
{
    treenode node;        /* tree node */
    treenode tnode;       /* temporary tree node */
    unsigned long parent; /* parent node */
    void *addr;           /* return address */
    unsigned long symbol; /* associated symbol */
    unsigned long name;   /* associated symbol name */
    unsigned long data;   /* profiling data */
    profiledata tdata;    /* temporary profiling data */
    unsigned char flags;  /* temporary flags */
}
profilenode;


/* The total number of allocations and deallocations.
 */

static size_t acount, dcount;


/* The total bytes of allocations and deallocations.
 */

static size_t atotal, dtotal;


/* The allocation and deallocation bins.
 */

static size_t *acounts, *dcounts;


/* The total bytes of large allocations and deallocations.
 */

static size_t atotals, dtotals;


/* The number of allocation bins.
 */

static size_t binsize;


/* The allocations and deallocations for all call sites.
 */

static profiledata *data;


/* The number of profiledata structures for all call sites.
 */

static size_t datasize;


/* The profiling details for all call sites.
 */

static profilenode *nodes;


/* The number of profilenode structures for all call sites.
 */

static size_t nodesize;


/* The array containing the symbol addresses.
 */

static void **addrs;


/* The string table containing the symbol names.
 */

static char *symbols;


/* The small, medium and large allocation boundaries.
 */

static size_t sbound, mbound, lbound;


/* The tree containing profiling details for all call sites.
 */

static treeroot proftree;


/* The tree containing temporary profiling details for all call sites.
 */

static treeroot temptree;


/* The profiling output file produced by mpatrol.
 */

static FILE *proffile;


/* The filename used to invoke this tool.
 */

static char *progname;


/* Indicates if different allocation points within single functions should
 * be noted when displaying the profiling tables.
 */

static int useaddresses;


/* Indicates if the emphasis will be on allocation counts rather than
 * allocated bytes when displaying the profiling tables.
 */

static int showcounts;


/* Indicates the maximum stack depth to use when comparing function call
 * stacks for the memory leak table.
 */

static unsigned long maxstack;


/* The table describing all recognised options.
 */

static option options_table[] =
{
    {"addresses", 'a', NULL,
     "\tSpecifies that different call sites from within the same function\n"
     "\tare to be differentiated and that the names of all functions should\n"
     "\tbe displayed with their call site offset in bytes.\n"},
    {"counts", 'c', NULL,
     "\tSpecifies that certain tables should be sorted by the number of\n"
     "\tallocations or deallocations rather than the total number of bytes\n"
     "\tallocated or deallocated.\n"},
    {"stack-depth", 'n', "depth",
     "\tSpecifies the maximum stack depth to display and also use when\n"
     "\tcalculating if one call site has the same call stack as another call\n"
     "\tsite.\n"},
    {"version", 'V', NULL,
     "\tDisplays the version number of this program.\n"},
    NULL
};


/* Clear the statistics for a set of profiling data.
 */

static void cleardata(profiledata *a)
{
    size_t i;

    for (i = 0; i < 4; i++)
    {
        a->acount[i] = 0;
        a->dcount[i] = 0;
        a->atotal[i] = 0;
        a->dtotal[i] = 0;
    }
}


/* Sum the statistics from two sets of profiling data.
 */

static void sumdata(profiledata *a, profiledata *b)
{
    size_t i;

    for (i = 0; i < 4; i++)
    {
        a->acount[i] += b->acount[i];
        a->dcount[i] += b->dcount[i];
        a->atotal[i] += b->atotal[i];
        a->dtotal[i] += b->dtotal[i];
    }
}


/* Compare two function call stacks.
 */

static int comparestack(profilenode *n, profilenode *p)
{
    size_t i;

    for (i = 1; (maxstack == 0) || (i < maxstack); i++)
    {
        if ((n->parent == 0) || (p->parent == 0))
            return ((n->parent == 0) && (p->parent == 0));
        n = &nodes[n->parent - 1];
        p = &nodes[p->parent - 1];
        if ((n->addr != p->addr) && (useaddresses || (n->symbol != p->symbol)))
            return 0;
    }
    return 1;
}


/* Byte-swap a block of memory.
 */

static void byteswap(void *b, size_t n)
{
    char *s, *t;
    char c;

    s = (char *) b;
    t = (char *) b + n - 1;
    while (s < t)
    {
        c = *s;
        *s++ = *t;
        *t-- = c;
    }
}


/* Read an entry from the profiling output file.
 */

static void getentry(void *d, size_t l, size_t n, int b)
{
    size_t i;

    if (fread(d, l, n, proffile) != n)
    {
        fprintf(stderr, "%s: Error reading file\n", progname);
        exit(EXIT_FAILURE);
    }
    /* Byte-swap all of the elements if necessary.
     */
    if (b != 0)
        for (i = 0; i < n; i++)
        {
            byteswap(d, l);
            d = (char *) d + l;
        }
}


/* Read all of the data from the profiling output file.
 */

static void readfile(void)
{
    char s[4];
    profiledata *d;
    profilenode *p;
    size_t i;
    unsigned long n;
    int b;

    /* When reading the profiling output file, we assume that if it begins and
     * ends with the magic sequence of characters then it is a valid profiling
     * output file from the mpatrol library.  There are probably an infinite
     * number of checks we could do to ensure that the rest of the data in the
     * file is valid, but that would be overcomplicated and probably slow this
     * program down.  However, if the file is only partially written then the
     * getentry() function will catch the error before we do something silly.
     */
    getentry(s, sizeof(char), 4, 0);
    if (memcmp(s, MP_PROFMAGIC, 4) != 0)
    {
        fprintf(stderr, "%s: Invalid file format\n", progname);
        exit(EXIT_FAILURE);
    }
    /* The following test allows us to read profiling output files that were
     * produced on a different processor architecture.  If the next word in the
     * file does not contain the value 1 then we have to byte-swap any further
     * data that we read from the file.  Note that this test only works if the
     * word size is the same on both machines.
     */
    getentry(&i, sizeof(size_t), 1, 0);
    b = (i != 1);
    getentry(&sbound, sizeof(size_t), 1, b);
    getentry(&mbound, sizeof(size_t), 1, b);
    getentry(&lbound, sizeof(size_t), 1, b);
    /* Read the allocation and deallocation bins.
     */
    getentry(&binsize, sizeof(size_t), 1, b);
    if (binsize > 0)
    {
        if (((acounts = (size_t *) malloc(binsize * sizeof(size_t))) == NULL) ||
            ((dcounts = (size_t *) malloc(binsize * sizeof(size_t))) == NULL))
        {
            fprintf(stderr, "%s: Out of memory\n", progname);
            exit(EXIT_FAILURE);
        }
        getentry(acounts, sizeof(size_t), binsize, b);
        getentry(&atotals, sizeof(size_t), 1, b);
        getentry(dcounts, sizeof(size_t), binsize, b);
        getentry(&dtotals, sizeof(size_t), 1, b);
        for (i = 0; i < binsize; i++)
        {
            acount += acounts[i];
            dcount += dcounts[i];
            if (i == binsize - 1)
            {
                atotal += atotals;
                dtotal += dtotals;
            }
            else
            {
                atotal += acounts[i] * (i + 1);
                dtotal += dcounts[i] * (i + 1);
            }
        }
    }
    /* Read the profiling data structures.
     */
    getentry(&datasize, sizeof(size_t), 1, b);
    if (datasize > 0)
    {
        if ((data = (profiledata *) malloc(datasize * sizeof(profiledata))) ==
            NULL)
        {
            fprintf(stderr, "%s: Out of memory\n", progname);
            exit(EXIT_FAILURE);
        }
        for (i = 0; i < datasize; i++)
        {
            getentry(&n, sizeof(unsigned long), 1, b);
            d = &data[n - 1];
            getentry(d->acount, sizeof(size_t), 4, b);
            getentry(d->atotal, sizeof(size_t), 4, b);
            getentry(d->dcount, sizeof(size_t), 4, b);
            getentry(d->dtotal, sizeof(size_t), 4, b);
        }
    }
    /* Read the statistics for every call site.
     */
    getentry(&nodesize, sizeof(size_t), 1, b);
    if (nodesize > 0)
    {
        if ((nodes = (profilenode *) malloc(nodesize * sizeof(profilenode))) ==
            NULL)
        {
            fprintf(stderr, "%s: Out of memory\n", progname);
            exit(EXIT_FAILURE);
        }
        for (i = 0; i < nodesize; i++)
        {
            getentry(&n, sizeof(unsigned long), 1, b);
            p = &nodes[n - 1];
            getentry(&p->parent, sizeof(unsigned long), 1, b);
            getentry(&p->addr, sizeof(void *), 1, b);
            getentry(&p->symbol, sizeof(unsigned long), 1, b);
            getentry(&p->name, sizeof(unsigned long), 1, b);
            getentry(&p->data, sizeof(unsigned long), 1, b);
            __mp_treeinsert(&proftree, &p->node, (unsigned long) p->addr);
            cleardata(&p->tdata);
            p->flags = 0;
        }
    }
    /* Read the table containing the symbol addresses.
     */
    getentry(&i, sizeof(size_t), 1, b);
    if (i > 0)
    {
        if ((addrs = (void **) malloc(i * sizeof(void *))) == NULL)
        {
            fprintf(stderr, "%s: Out of memory\n", progname);
            exit(EXIT_FAILURE);
        }
        getentry(addrs, sizeof(void *), i, b);
    }
    /* Read the string table containing the symbol names.
     */
    getentry(&i, sizeof(size_t), 1, b);
    if (i > 0)
    {
        if ((symbols = (char *) malloc(i * sizeof(char))) == NULL)
        {
            fprintf(stderr, "%s: Out of memory\n", progname);
            exit(EXIT_FAILURE);
        }
        getentry(symbols, sizeof(char), i, 0);
    }
    getentry(s, sizeof(char), 4, 0);
    if (memcmp(s, MP_PROFMAGIC, 4) != 0)
    {
        fprintf(stderr, "%s: Invalid file format\n", progname);
        exit(EXIT_FAILURE);
    }
}


/* Display a character a specified number of times.
 */

static void printchar(char c, size_t n)
{
    size_t i;

    for (i = 0; i < n; i++)
        fputc(c, stdout);
}


/* Display a set of profiling data.
 */

static void printdata(size_t *d, size_t t)
{
    size_t i;
    double n;

    for (i = 0; i < 4; i++)
        if ((d[i] == 0) || (t == 0))
            fputs("   ", stdout);
        else
        {
            n = ((double) d[i] / (double) t) * 100.0;
            if (n <= 0.5)
                fputs("  .", stdout);
            else if (n >= 99.5)
                fputs(" %%", stdout);
            else
                fprintf(stdout, " %2.0f", n);
        }
}


/* Display the symbol associated with a particular call site.
 */

static void printsymbol(profilenode *n)
{
    ptrdiff_t o;

    if (n->name == 0)
        fprintf(stdout, MP_POINTER, n->addr);
    else
    {
        fputs(symbols + n->name, stdout);
        if (useaddresses && (n->symbol != 0) &&
            ((o = (char *) n->addr - (char *) addrs[n->symbol - 1]) != 0))
            fprintf(stdout, "%+ld", o);
    }
}


/* Display the allocation bin table.
 */

static void bintable(void)
{
    size_t i;
    unsigned long a, b, c, d;
    double e, f, g, h;
    int p;

    p = 0;
    printchar(' ', 32);
    fputs("ALLOCATION BINS\n\n", stdout);
    printchar(' ', 29);
    fprintf(stdout, "(number of bins: %lu)\n\n", binsize);
    printchar(' ', 21);
    fputs("allocated", stdout);
    printchar(' ', 26);
    fputs("unfreed\n", stdout);
    printchar(' ', 10);
    printchar('-', 32);
    fputs("  ", stdout);
    printchar('-', 32);
    fputs("\n    size   count       %     bytes       %   "
          "count       %     bytes       %\n\n", stdout);
    for (i = 0; i < binsize; i++)
        if (acounts[i] != 0)
        {
            a = acounts[i];
            b = a - dcounts[i];
            if (i == binsize - 1)
            {
                c = atotals;
                d = c - dtotals;
            }
            else
            {
                c = a * (i + 1);
                d = b * (i + 1);
            }
            e = ((double) a / (double) acount) * 100.0;
            if (acount != dcount)
                f = ((double) b / (double) (acount - dcount)) * 100.0;
            else
                f = 0.0;
            g = ((double) c / (double) atotal) * 100.0;
            if (atotal != dtotal)
                h = ((double) d / (double) (atotal - dtotal)) * 100.0;
            else
                h = 0.0;
            fprintf(stdout, " %s %4lu  %6lu  %6.2f  %8lu  %6.2f  "
                    "%6lu  %6.2f  %8lu  %6.2f\n",
                    (i == binsize - 1) ? ">=" : "  ",
                    i + 1, a, e, c, g, b, f, d, h);
            p = 1;
        }
    if (p == 1)
        fputc('\n', stdout);
    fprintf(stdout, "   total  %6lu          %8lu          "
            "%6lu          %8lu\n", acount, atotal,
            acount - dcount, atotal - dtotal);
}


/* Display the direct allocation table.
 */

static void directtable(void)
{
    profiledata *d;
    profilenode *n, *p;
    treenode *t;
    profiledata m;
    size_t i;
    unsigned long a, b, c;
    double e, f;

    cleardata(&m);
    printchar(' ', 31);
    fputs("DIRECT ALLOCATIONS\n\n", stdout);
    printchar(' ', 20);
    fprintf(stdout, "(0 < s <= %lu < m <= %lu < l <= %lu < x)\n\n",
            sbound, mbound, lbound);
    if (showcounts)
    {
        printchar(' ', 9);
        fputs("allocated", stdout);
        printchar(' ', 21);
        fputs("unfreed\n", stdout);
        printchar('-', 27);
        fputs("  ", stdout);
        printchar('-', 27);
        fputs("\n count       %   s  m  l  x   "
              "count       %   s  m  l  x     bytes  function\n\n", stdout);
    }
    else
    {
        printchar(' ', 10);
        fputs("allocated", stdout);
        printchar(' ', 23);
        fputs("unfreed\n", stdout);
        printchar('-', 29);
        fputs("  ", stdout);
        printchar('-', 29);
        fputs("\n   bytes       %   s  m  l  x     "
              "bytes       %   s  m  l  x   count  function\n\n", stdout);
    }
    for (n = (profilenode *) __mp_minimum(proftree.root); n != NULL; n = p)
    {
        p = (profilenode *) __mp_successor(&n->node);
        if (n->data != 0)
        {
            d = &n->tdata;
            sumdata(d, &data[n->data - 1]);
            while ((p != NULL) && ((p->addr == n->addr) || (!useaddresses &&
                     (p->symbol != 0) && (p->symbol == n->symbol))))
            {
                if (p->data != 0)
                    sumdata(d, &data[p->data - 1]);
                p = (profilenode *) __mp_successor(&p->node);
            }
            a = 0;
            for (i = 0; i < 4; i++)
                if (showcounts)
                    a += d->acount[i];
                else
                    a += d->atotal[i];
            __mp_treeinsert(&temptree, &n->tnode, a);
            sumdata(&m, d);
        }
    }
    for (t = __mp_maximum(temptree.root); t != NULL; t = __mp_predecessor(t))
    {
        n = (profilenode *) ((char *) t - offsetof(profilenode, tnode));
        d = &n->tdata;
        a = t->key;
        b = c = 0;
        for (i = 0; i < 4; i++)
        {
            if (showcounts)
            {
                b += d->dcount[i];
                c += d->atotal[i];
            }
            else
            {
                b += d->dtotal[i];
                c += d->acount[i];
            }
            d->dcount[i] = d->acount[i] - d->dcount[i];
            d->dtotal[i] = d->atotal[i] - d->dtotal[i];
        }
        b = a - b;
        if (showcounts)
        {
            e = ((double) a / (double) acount) * 100.0;
            if (acount != dcount)
                f = ((double) b / (double) (acount - dcount)) * 100.0;
            else
                f = 0.0;
            fprintf(stdout, "%6lu  %6.2f ", a, e);
            printdata(d->acount, acount);
            fprintf(stdout, "  %6lu  %6.2f ", b, f);
            printdata(d->dcount, acount - dcount);
            fprintf(stdout, "  %8lu  ", c);
        }
        else
        {
            e = ((double) a / (double) atotal) * 100.0;
            if (atotal != dtotal)
                f = ((double) b / (double) (atotal - dtotal)) * 100.0;
            else
                f = 0.0;
            fprintf(stdout, "%8lu  %6.2f ", a, e);
            printdata(d->atotal, atotal);
            fprintf(stdout, "  %8lu  %6.2f ", b, f);
            printdata(d->dtotal, atotal - dtotal);
            fprintf(stdout, "  %6lu  ", c);
        }
        printsymbol(n);
        fputc('\n', stdout);
        cleardata(d);
    }
    for (i = 0; i < 4; i++)
    {
        m.dcount[i] = m.acount[i] - m.dcount[i];
        m.dtotal[i] = m.atotal[i] - m.dtotal[i];
    }
    if (temptree.size != 0)
        fputc('\n', stdout);
    if (showcounts)
    {
        fprintf(stdout, "%6lu         ", acount);
        printdata(m.acount, acount);
        fprintf(stdout, "  %6lu         ", acount - dcount);
        printdata(m.dcount, acount - dcount);
        fprintf(stdout, "  %8lu  total\n", atotal);
    }
    else
    {
        fprintf(stdout, "%8lu         ", atotal);
        printdata(m.atotal, atotal);
        fprintf(stdout, "  %8lu         ", atotal - dtotal);
        printdata(m.dtotal, atotal - dtotal);
        fprintf(stdout, "  %6lu  total\n", acount);
    }
    __mp_newtree(&temptree);
}


/* Display the memory leak table.
 */

static void leaktable(void)
{
    profiledata *d;
    profilenode *n, *p;
    treenode *t;
    size_t i;
    unsigned long a, b, j, k;
    double e, f, g;

    printchar(' ', 34);
    fputs("MEMORY LEAKS\n\n", stdout);
    printchar(' ', 28);
    fprintf(stdout, "(maximum stack depth: %lu)\n\n", maxstack);
    printchar(' ', 16);
    fputs("unfreed", stdout);
    printchar(' ', 22);
    fputs("allocated\n", stdout);
    printchar('-', 40);
    fputs("  ", stdout);
    printchar('-', 16);
    if (showcounts)
        fputs("\n     %   count       %     bytes       %   "
              "count     bytes  function\n\n", stdout);
    else
        fputs("\n     %     bytes       %   count       %     "
              "bytes   count  function\n\n", stdout);
    for (n = (profilenode *) __mp_minimum(proftree.root); n != NULL; n = p)
    {
        p = (profilenode *) __mp_successor(&n->node);
        if ((n->data != 0) && !n->flags)
        {
            d = &n->tdata;
            sumdata(d, &data[n->data - 1]);
            while ((p != NULL) && ((p->addr == n->addr) || (!useaddresses &&
                     (p->symbol != 0) && (p->symbol == n->symbol))))
            {
                if ((p->data != 0) && !p->flags && comparestack(n, p))
                {
                    sumdata(d, &data[p->data - 1]);
                    p->flags = 1;
                }
                p = (profilenode *) __mp_successor(&p->node);
            }
            p = (profilenode *) __mp_successor(&n->node);
            a = 0;
            for (i = 0; i < 4; i++)
                if (showcounts)
                    a += d->acount[i] - d->dcount[i];
                else
                    a += d->atotal[i] - d->dtotal[i];
            if (a > 0)
                __mp_treeinsert(&temptree, &n->tnode, a);
        }
    }
    for (n = (profilenode *) __mp_minimum(proftree.root); n != NULL;
         n = (profilenode *) __mp_successor(&n->node))
        n->flags = 0;
    for (t = __mp_maximum(temptree.root); t != NULL; t = __mp_predecessor(t))
    {
        n = (profilenode *) ((char *) t - offsetof(profilenode, tnode));
        d = &n->tdata;
        a = t->key;
        b = j = k = 0;
        for (i = 0; i < 4; i++)
            if (showcounts)
            {
                b += d->dtotal[i];
                j += d->acount[i];
                k += d->atotal[i];
            }
            else
            {
                b += d->dcount[i];
                j += d->atotal[i];
                k += d->acount[i];
            }
        b = k - b;
        e = ((double) a / (double) j) * 100.0;
        f = ((double) b / (double) k) * 100.0;
        if (showcounts)
        {
            g = ((double) a / (double) (acount - dcount)) * 100.0;
            fprintf(stdout, "%6.2f  %6lu  %6.2f  %8lu  %6.2f  %6lu  %8lu  ",
                    g, a, e, b, f, j, k);
        }
        else
        {
            g = ((double) a / (double) (atotal - dtotal)) * 100.0;
            fprintf(stdout, "%6.2f  %8lu  %6.2f  %6lu  %6.2f  %8lu  %6lu  ",
                    g, a, e, b, f, j, k);
        }
        printsymbol(n);
        fputc('\n', stdout);
        p = n;
        for (i = 1; (maxstack == 0) || (i < maxstack); i++)
        {
            if (p->parent == 0)
                break;
            p = &nodes[p->parent - 1];
            printchar(' ', 60);
            printsymbol(p);
            fputc('\n', stdout);
        }
        cleardata(d);
    }
    if (acount != 0)
        e = ((double) (acount - dcount) / (double) acount) * 100.0;
    else
        e = 0.0;
    if (atotal != 0)
        f = ((double) (atotal - dtotal) / (double) atotal) * 100.0;
    else
        f = 0.0;
    if (temptree.size != 0)
        fputc('\n', stdout);
    if (showcounts)
        fprintf(stdout, "        %6lu  %6.2f  %8lu  %6.2f  %6lu  %8lu  total\n",
                acount - dcount, e, atotal - dtotal, f, acount, atotal);
    else
        fprintf(stdout, "        %8lu  %6.2f  %6lu  %6.2f  %8lu  %6lu  total\n",
                atotal - dtotal, f, acount - dcount, e, atotal, acount);
    __mp_newtree(&temptree);
}


/* Read the profiling output file and display all specified information.
 */

int main(int argc, char **argv)
{
    char b[256];
    char *f;
    int c, e, v;

    e = v = 0;
    maxstack = 1;
    progname = argv[0];
    while ((c = __mp_getopt(argc, argv, __mp_shortopts(b, options_table),
             options_table)) != EOF)
        switch (c)
        {
          case 'a':
            useaddresses = 1;
            break;
          case 'c':
            showcounts = 1;
            break;
          case 'n':
            if (!__mp_getnum(progname, __mp_optarg, (long *) &maxstack, 1))
                e = 1;
            break;
          case 'V':
            v = 1;
            break;
          default:
            e = 1;
            break;
        }
    argc -= __mp_optindex;
    argv += __mp_optindex;
    if (v == 1)
    {
        fprintf(stderr, "%s %s\n%s\n\n", progname, VERSION, __mp_copyright);
        fputs("This is free software, and you are welcome to redistribute it "
              "under certain\n", stderr);
        fputs("conditions; see the GNU Library General Public License for "
              "details.\n\n", stderr);
        fputs("For the latest mpatrol release and documentation,\n", stderr);
        fprintf(stderr, "visit %s.\n\n", __mp_homepage);
    }
    if ((argc > 1) || (e == 1))
    {
        fprintf(stderr, "Usage: %s [options] [file]\n\n", progname);
        __mp_showopts(options_table);
        exit(EXIT_FAILURE);
    }
    if (argc == 1)
        f = argv[0];
    else
        f = MP_PROFFILE;
    acount = dcount = 0;
    atotal = dtotal = 0;
    acounts = dcounts = NULL;
    atotals = dtotals = 0;
    binsize = 0;
    data = NULL;
    datasize = 0;
    nodes = NULL;
    nodesize = 0;
    addrs = NULL;
    symbols = NULL;
    sbound = mbound = lbound = 0;
    __mp_newtree(&proftree);
    __mp_newtree(&temptree);
    if (strcmp(f, "-") == 0)
        proffile = stdin;
    else if ((proffile = fopen(f, "rb")) == NULL)
    {
        fprintf(stderr, "%s: Cannot open file `%s'\n", progname, f);
        exit(EXIT_FAILURE);
    }
    readfile();
    fclose(proffile);
    bintable();
    fputs("\n\n", stdout);
    directtable();
    fputs("\n\n", stdout);
    leaktable();
    if (acounts != NULL)
        free(acounts);
    if (dcounts != NULL)
        free(dcounts);
    if (data != NULL)
        free(data);
    if (nodes != NULL)
        free(nodes);
    if (addrs != NULL)
        free(addrs);
    if (symbols != NULL)
        free(symbols);
    return EXIT_SUCCESS;
}
