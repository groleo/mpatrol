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
 * Option parsing.  Implements a routine, similar to getopt() provided on most
 * UNIX systems, which is used to parse command line options in the mpatrol
 * tools.
 */


#include "getopt.h"
#include <stdio.h>
#include <string.h>


#if MP_IDENT_SUPPORT
#ident "$Id: getopt.c,v 1.1 2000-04-05 17:49:19 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* The index of the current option in the command line argument array.
 */

MP_GLOBAL unsigned long __mp_optindex;


/* The argument for the current option.
 */

MP_GLOBAL char *__mp_optarg;


/* Read an option from a supplied command line argument array.
 */

MP_GLOBAL int __mp_getopt(unsigned long n, char **a, char *s)
{
    static char *t;
    char *p;
    int r;

    __mp_optarg = NULL;
    /* If the index of the current option is zero or if there are no more
     * options in this argument then we proceed to the next argument.
     */
    if ((__mp_optindex == 0) || ((t != NULL) && (*t == '\0')))
    {
        __mp_optindex++;
        t = NULL;
    }
    /* If there is not a current option then attempt to locate it, otherwise
     * return EOF if there are no more options.
     */
    if (t == NULL)
    {
        if (__mp_optindex >= n)
            return EOF;
        t = a[__mp_optindex];
        /* Stop parsing options if either the argument is not an option, if it
         * is a single dash (representing stdin) or if it is a double dash
         * representing the end of options.
         */
        if ((*t != '-') || (t[1] == '\0') || ((t[1] == '-') && (t[2] == '\0')))
        {
            if ((*t == '-') && (t[1] == '-') && (t[2] == '\0'))
                __mp_optindex++;
            t = NULL;
            return EOF;
        }
        t++;
    }
    /* Check that the option appears in the string of recognised options.
     */
    if ((*t == ':') || ((p = strchr(s, *t)) == NULL))
    {
        fprintf(stderr, "%s: Illegal option `-%c'\n", a[0], *t++);
        return '?';
    }
    r = *t++;
    /* Check to see if the option takes an argument.
     */
    if (p[1] == ':')
    {
        if (*t == '\0')
        {
            /* The rest of this argument is empty, so we proceed to the next
             * argument.
             */
            if (++__mp_optindex >= n)
            {
                fprintf(stderr, "%s: Option `-%c' requires an argument\n", a[0],
                        *t);
                t = NULL;
                return '?';
            }
            __mp_optarg = a[__mp_optindex];
        }
        else
            __mp_optarg = t;
        __mp_optindex++;
        t = NULL;
    }
    return r;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
