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


#include "getopt.h"
#include "version.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#if MP_IDENT_SUPPORT
#ident "$Id: mprof.c,v 1.1 2000-04-24 13:15:07 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#define VERSION "1.0" /* the current version of this program */


/* The profiling output file produced by mpatrol.
 */

static FILE *proffile;


/* The filename used to invoke this tool.
 */

static char *progname;


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
    if (strcmp(f, "-") == 0)
        proffile = stdin;
    else if ((proffile = fopen(f, "rb")) == NULL)
    {
        fprintf(stderr, "%s: Cannot open file `%s'\n", progname, f);
        exit(EXIT_FAILURE);
    }
    fclose(proffile);
    return EXIT_SUCCESS;
}
