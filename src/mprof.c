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
#ident "$Id: mprof.c,v 1.5 2000-04-25 19:24:30 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#define VERSION "1.0" /* the current version of this program */


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
    unsigned long parent; /* parent node */
    void *addr;           /* return address */
    unsigned long symbol; /* associated symbol */
    unsigned long data;   /* profiling data */
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


/* The string table containing the symbol names.
 */

static char *symbols;


/* The small, medium and large allocation boundaries.
 */

static size_t sbound, mbound, lbound;


/* The tree containing information about each call site.
 */

static treeroot sitetree;


/* The profiling output file produced by mpatrol.
 */

static FILE *proffile;


/* The filename used to invoke this tool.
 */

static char *progname;


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


/* Display a set of profiling data.
 */

static void printdata(size_t *d, size_t t)
{
    size_t i;
    double n;

    for (i = 0; i < 4; i++)
        if ((t == 0) || (d[i] == 0))
            fputs("   ", stdout);
        else
        {
            n = ((double) d[i] / (double) t) * 100.0;
            if (n >= 99.5)
                fputs(" %%", stdout);
            else
                fprintf(stdout, " %2.0f", n);
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


/* Read an entry from the profiling output file.
 */

static void getentry(void *d, size_t l, size_t n)
{
    if (fread(d, l, n, proffile) != n)
    {
        fprintf(stderr, "%s: Error reading file\n", progname);
        exit(EXIT_FAILURE);
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

    /* When reading the profiling output file, we assume that if it begins and
     * ends with the magic sequence of characters then it is a valid profiling
     * output file from the mpatrol library.  There are probably an infinite
     * number of checks we could do to ensure that the rest of the data in the
     * file is valid, but that would be overcomplicated and probably slow this
     * program down.  However, if the file is only partially written then the
     * getentry() function will catch the error before we do something silly.
     */
    getentry(s, sizeof(char), 4);
    if (memcmp(s, MP_PROFMAGIC, 4) != 0)
    {
        fprintf(stderr, "%s: Invalid file format\n", progname);
        exit(EXIT_FAILURE);
    }
    getentry(&sbound, sizeof(size_t), 1);
    getentry(&mbound, sizeof(size_t), 1);
    getentry(&lbound, sizeof(size_t), 1);
    /* Read the allocation and deallocation bins.
     */
    getentry(&binsize, sizeof(size_t), 1);
    if (binsize > 0)
    {
        if (((acounts = (size_t *) malloc(binsize * sizeof(size_t))) == NULL) ||
            ((dcounts = (size_t *) malloc(binsize * sizeof(size_t))) == NULL))
        {
            fprintf(stderr, "%s: Out of memory\n", progname);
            exit(EXIT_FAILURE);
        }
        getentry(acounts, sizeof(size_t), binsize);
        getentry(&atotals, sizeof(size_t), 1);
        getentry(dcounts, sizeof(size_t), binsize);
        getentry(&dtotals, sizeof(size_t), 1);
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
    getentry(&datasize, sizeof(size_t), 1);
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
            getentry(&n, sizeof(unsigned long), 1);
            d = &data[n - 1];
            getentry(d->acount, sizeof(size_t), 4);
            getentry(d->atotal, sizeof(size_t), 4);
            getentry(d->dcount, sizeof(size_t), 4);
            getentry(d->dtotal, sizeof(size_t), 4);
        }
    }
    /* Read the statistics for every call site.
     */
    getentry(&nodesize, sizeof(size_t), 1);
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
            getentry(&n, sizeof(unsigned long), 1);
            p = &nodes[n - 1];
            getentry(&p->parent, sizeof(unsigned long), 1);
            getentry(&p->addr, sizeof(void *), 1);
            getentry(&p->symbol, sizeof(unsigned long), 1);
            getentry(&p->data, sizeof(unsigned long), 1);
            __mp_treeinsert(&sitetree, &p->node, (unsigned long) p->addr);
        }
    }
    /* Read the string table containing the symbol names.
     */
    getentry(&i, sizeof(size_t), 1);
    if (i > 1)
    {
        if ((symbols = (char *) malloc(i * sizeof(char))) == NULL)
        {
            fprintf(stderr, "%s: Out of memory\n", progname);
            exit(EXIT_FAILURE);
        }
        getentry(symbols, sizeof(char), i);
    }
    getentry(s, sizeof(char), 4);
    if (memcmp(s, MP_PROFMAGIC, 4) != 0)
    {
        fprintf(stderr, "%s: Invalid file format\n", progname);
        exit(EXIT_FAILURE);
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
    profilenode *n, *p;
    profiledata d;
    size_t i;
    unsigned long a, b, c;

    printchar(' ', 31);
    fputs("DIRECT ALLOCATIONS\n\n", stdout);
    printchar(' ', 14);
    fputs("allocated", stdout);
    printchar(' ', 15);
    fputs("unfreed\n", stdout);
    printchar(' ', 8);
    printchar('-', 21);
    fputs("  ", stdout);
    printchar('-', 21);
    fputs("\n     %     bytes   s  m  l  x     "
          "bytes   s  m  l  x   calls  function\n\n", stdout);
    for (n = (profilenode *) __mp_minimum(sitetree.root); n != NULL; n = p)
    {
        p = (profilenode *) __mp_successor(&n->node);
        if (n->data != 0)
        {
            d = data[n->data - 1];
            while ((p->addr == n->addr) ||
                   ((p->symbol != 0) && (p->symbol == n->symbol)))
            {
                n = p;
                p = (profilenode *) __mp_successor(&n->node);
                if (n->data != 0)
                    sumdata(&d, &data[n->data - 1]);
            }
            a = b = c = 0;
            for (i = 0; i < 4; i++)
            {
                a += d.atotal[i];
                b += d.dtotal[i];
                c += d.acount[i];
            }
            fprintf(stdout, "%6.2f  %8lu ",
                    ((double) a / (double) atotal) * 100.0, a);
            printdata(d.atotal, atotal);
            fprintf(stdout, "  %8lu ", b);
            printdata(d.dtotal, dtotal);
            fprintf(stdout, "  %6lu  ", c);
            if (n->symbol != 0)
                fprintf(stdout, "%s\n", symbols + n->symbol);
            else
                fprintf(stdout, MP_POINTER "\n", n->addr);
        }
    }
}


/* Read the profiling output file and display all specified information.
 */

int main(int argc, char **argv)
{
    char *f;
    int c, e, v;

    e = v = 0;
    progname = argv[0];
    while ((c = __mp_getopt(argc, argv, "V")) != EOF)
        switch (c)
        {
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
        fprintf(stderr, "Usage: %s [-V] [file]\n", progname);
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
    symbols = NULL;
    sbound = mbound = lbound = 0;
    __mp_newtree(&sitetree);
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
    if (acounts != NULL)
        free(acounts);
    if (dcounts != NULL)
        free(dcounts);
    if (data != NULL)
        free(data);
    if (nodes != NULL)
        free(nodes);
    if (symbols != NULL)
        free(symbols);
    return EXIT_SUCCESS;
}
