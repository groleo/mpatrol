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
 * A tool designed to read a log file produced by the mpatrol library
 * and report any unfreed memory allocations.  This should be used if
 * the mpatrol library could not finish writing the log file due to
 * abnormal program termination, but note that some of the unfreed
 * allocations might have been freed if the program had terminated
 * successfully.  Also note that no attempt is made to account for
 * resizing of memory allocations and so the total amount of memory
 * used by the resulting unfreed allocations may not be entirely accurate.
 */


#include "tree.h"
#include "getopt.h"
#include "version.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#if MP_IDENT_SUPPORT
#ident "$Id: mleak.c,v 1.3 2000-09-25 21:47:14 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#define VERSION "1.2" /* the current version of this program */


/* Structure containing the allocation details and log file offset for a
 * single memory allocation.
 */

typedef struct allocation
{
    treenode node;        /* tree node */
    unsigned long addr;   /* allocation address */
    unsigned long size;   /* allocation size */
    unsigned long offset; /* log file offset */
}
allocation;


/* The tree containing information about each memory allocation.
 */

static treeroot alloctree;


/* The total number of bytes currently allocated.
 */

static unsigned long alloctotal;


/* The log file produced by mpatrol.
 */

static FILE *logfile;


/* The current offset in the log file.
 */

static long fileoffset;


/* The filename used to invoke this tool.
 */

static char *progname;


/* The table describing all recognised options.
 */

static option options_table[] =
{
    {"version", 'V', NULL,
     "\tDisplays the version number of this program.\n"},
    NULL
};


/* Create a new memory allocation and record its log file offset.
 */

static void newalloc(unsigned long i, unsigned long a, unsigned long l,
                     unsigned long o)
{
    allocation *n;

    if ((n = (allocation *) malloc(sizeof(allocation))) == NULL)
    {
        fprintf(stderr, "%s: Out of memory\n", progname);
        exit(EXIT_FAILURE);
    }
    __mp_treeinsert(&alloctree, &n->node, i);
    n->addr = a;
    n->size = l;
    n->offset = o;
    alloctotal += l;
}


/* Free an existing memory allocation.
 */

static void freealloc(unsigned long i)
{
    allocation *n;

    if (n = (allocation *) __mp_search(alloctree.root, i))
    {
        __mp_treeremove(&alloctree, &n->node);
        alloctotal -= n->size;
        free(n);
    }
}


/* Read an input line from the log file.
 */

static char *getline(void)
{
    static char s[MP_BUFFER_SIZE + 1];
    unsigned long i;
    int c;

    i = 0;
    /* Record the file offset so that we can go back to this position during
     * the second pass of the log file.
     */
    if ((fileoffset = ftell(logfile)) == -1)
    {
        fprintf(stderr, "%s: Cannot determine file position\n", progname);
        exit(EXIT_FAILURE);
    }
    while (((c = fgetc(logfile)) != EOF) && (c != '\n'))
    {
        if (i == MP_BUFFER_SIZE)
        {
            fprintf(stderr, "%s: Buffer overflow\n", progname);
            exit(EXIT_FAILURE);
        }
        s[i++] = c;
    }
    if (c == EOF)
        return NULL;
    s[i] = '\0';
    return s;
}


/* Log the allocations and deallocations from the log file.
 */

static void readfile(void)
{
    char *s, *t;
    unsigned long a, l, n, o;

    while (s = getline())
        if (strncmp(s, "ALLOC: ", 7) == 0)
        {
            /* Parse relevant details from the memory allocation and
             * add the allocation to the allocation tree.
             */
            o = fileoffset;
            if ((s = strchr(s + 7, '(')) && (t = strchr(s + 1, ',')))
            {
                /* Get the allocation index.
                 */
                *t = '\0';
                n = strtoul(s + 1, NULL, 0);
                if ((*(s = t + 1) == ' ') && (t = strchr(s + 1, ' ')))
                {
                    /* Get the allocation size.
                     */
                    *t = '\0';
                    l = strtoul(s + 1, NULL, 0);
                    /* Don't record the allocation if the pointer returned is
                     * NULL.
                     */
                    while ((s = getline()) && (strncmp(s, "returns ", 8) != 0));
                    if ((n != 0) && (s != NULL) &&
                        (a = strtoul(s + 8, NULL, 0)))
                        newalloc(n, a, l, o);
                }
            }
        }
        else if (strncmp(s, "FREE: ", 6) == 0)
        {
            /* Parse relevant details from the memory deallocation and
             * remove the allocation from the allocation tree.
             */
            if ((s = strchr(s + 6, '(')) && (t = strchr(s + 1, ')')))
            {
                /* Get the allocation address.
                 */
                *t = '\0';
                if (a = strtoul(s + 1, NULL, 0))
                {
                    while ((s = getline()) && (*s != '\0'));
                    /* Don't record the deallocation if a warning or error
                     * occurred.
                     */
                    if ((s = getline()) && (strncmp(s, "    ", 4) == 0) &&
                        (s = strchr(s + 4, ':')) && (t = strchr(s + 1, ':')))
                    {
                        /* Get the allocation index.
                         */
                        *t = '\0';
                        n = strtoul(s + 1, NULL, 0);
                        freealloc(n);
                    }
                }
            }
        }
        else if (strncmp(s, "system page size: ", 18) == 0)
            /* If this is the beginning of the summary then there will be
             * no more allocations or deallocations after it.
             */
            break;
}


/* Display all remaining memory allocations.
 */

static void printallocs(void)
{
    allocation *n, *p;
    char *r, *s, *t;

    printf("unfreed allocations: %lu (%lu byte%s)\n", alloctree.size,
           alloctotal, (alloctotal == 1) ? "" : "s");
    for (n = (allocation *) __mp_minimum(alloctree.root); n != NULL; n = p)
    {
        p = (allocation *) __mp_successor(&n->node);
        /* Move to the position in the log file that records the original
         * allocation.
         */
        if (fseek(logfile, n->offset, SEEK_SET) == -1)
        {
            fprintf(stderr, "%s: Cannot set file position\n", progname);
            exit(EXIT_FAILURE);
        }
        /* Extract the relevant information from the allocation log so that
         * we can format it in the same way as that displayed for the
         * SHOWUNFREED option.
         */
        if ((s = getline()) && (strncmp(s, "ALLOC: ", 7) == 0) &&
            (t = strchr(s + 7, '(')) && (t > s) && (*(t = t - 1) == ' '))
        {
            *t = '\0';
            r = s + 7;
            if (s = strchr(t + 2, '['))
            {
                printf("    " MP_POINTER " (%lu byte%s) {%s:%lu:0} %s\n",
                       n->addr, n->size, (n->size == 1) ? "" : "s", r,
                       n->node.key, s);
                while ((s = getline()) && (*s != '\0'))
                    puts(s);
                if (alloctree.size > 1)
                    putchar('\n');
            }
        }
        __mp_treeremove(&alloctree, &n->node);
        free(n);
    }
}


/* Read the log file and display all unfreed memory allocations.
 */

int main(int argc, char **argv)
{
    char *f;
    int c, e, v;

    e = v = 0;
    progname = argv[0];
    while ((c = __mp_getopt(argc, argv, "V", options_table)) != EOF)
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
        fprintf(stderr, "Usage: %s [options] [file]\n\n", progname);
        __mp_showopts(options_table);
        exit(EXIT_FAILURE);
    }
    if (argc == 1)
        f = argv[0];
    else
        f = MP_LOGFILE;
    __mp_newtree(&alloctree);
    alloctotal = 0;
    if (strcmp(f, "-") == 0)
        logfile = stdin;
    else if ((logfile = fopen(f, "r")) == NULL)
    {
        fprintf(stderr, "%s: Cannot open file `%s'\n", progname, f);
        exit(EXIT_FAILURE);
    }
    fileoffset = 0;
    readfile();
    printallocs();
    fclose(logfile);
    return EXIT_SUCCESS;
}
