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
 * tools.  Options with long names are also supported in a way that is similar
 * to the GNU style of command line option handling.
 */


#include "getopt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>


#if MP_IDENT_SUPPORT
#ident "$Id: getopt.c,v 1.9 2000-11-17 18:05:28 graeme Exp $"
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


/* Convert a string representation of a number to an integer,
 * reporting any errors that occur during the conversion.
 */

MP_GLOBAL int __mp_getnum(char *p, char *s, long *n, int u)
{
    char *t;
    int e;

    e = errno;
    errno = 0;
    if ((u == 1) && (*s == '-'))
    {
        fprintf(stderr, "%s: Illegal positive number `%s'\n", p, s);
        t = s;
    }
    else if ((u == 0) && (*s == '-') && (s[1] == '0') && ((s[2] == 'b') ||
             (s[2] == 'B')))
        /* This is a negative binary number.
         */
        *n = -strtol(s + 3, &t, 2);
    else if ((*s == '0') && ((s[1] == 'b') || (s[1] == 'B')))
    {
        /* This is a positive binary number.
         */
        if (u == 0)
            *n = strtol(s + 2, &t, 2);
        else
            *n = strtoul(s + 2, &t, 2);
    }
    /* Otherwise let the conversion function work out the number base
     * from the prefix.
     */
    else if (u == 0)
        *n = strtol(s, &t, 0);
    else
        *n = strtoul(s, &t, 0);
    if (errno == ERANGE)
        fprintf(stderr, "%s: %s number overflow in `%s'\n", p, ((u == 0) &&
                 (*n == LONG_MIN)) ? "Negative" : "Positive", s);
    errno = e;
    return (*t == '\0');
}


/* Search for an option in the long options table.
 */

static option *findopt(char *s, option *l, char **a)
{
    char *t;
    size_t n;

    if (t = strchr(s, '='))
        n = t - s;
    else
        n = strlen(s);
    while (l->name != NULL)
    {
        if ((strncmp(s, l->name, n) == 0) && (l->name[n] == '\0'))
        {
            if (t != NULL)
                n++;
            *a = s + n;
            return l;
        }
        l++;
    }
    return NULL;
}


/* Read an option from a supplied command line argument array.
 */

MP_GLOBAL int __mp_getopt(unsigned long n, char **a, char *s, option *l)
{
    static char *t;
    option *m;
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
        /* Parse a long option and possibly its argument.
         */
        if ((*t == '-') && (l != NULL))
        {
            t++;
            /* Check that the option appears in the long options table.
             */
            if ((m = findopt(t, l, &t)) == NULL)
            {
                fprintf(stderr, "%s: Illegal option `--%s'\n", a[0], t);
                __mp_optindex++;
                t = NULL;
                return '?';
            }
            /* Check to see if the option takes an argument.
             */
            if (m->arg)
                if (*t == '\0')
                {
                    /* The rest of this argument is empty, so we proceed to the
                     * next argument.
                     */
                    if ((++__mp_optindex >= n) ||
                        (strcmp(a[__mp_optindex], "--") == 0))
                    {
                        fprintf(stderr, "%s: Option `--%s' requires an "
                                "argument\n", a[0], m->name);
                        t = NULL;
                        return '?';
                    }
                    __mp_optarg = a[__mp_optindex];
                }
                else
                    __mp_optarg = t;
            else if (*t != '\0')
                fprintf(stderr, "%s: Ignoring argument `%s' for option "
                        "`--%s'\n", a[0], t, m->name);
            __mp_optindex++;
            t = NULL;
            return m->value;
        }
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
            if ((++__mp_optindex >= n) || (strcmp(a[__mp_optindex], "--") == 0))
            {
                fprintf(stderr, "%s: Option `-%c' requires an argument\n", a[0],
                        r);
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


/* Build a string of short options letters from the long options table.
 */

MP_GLOBAL char *__mp_shortopts(char *s, option *l)
{
    char *t;

    t = s;
    while (l->name != NULL)
    {
        if ((l->value >= SHORTOPT_MIN) && (l->value <= SHORTOPT_MAX))
        {
            *t++ = l->value;
            if (l->arg)
                *t++ = ':';
        }
        l++;
    }
    *t = '\0';
    return s;
}


/* Display a quick-reference option summary.
 */

MP_GLOBAL void __mp_showopts(option *l)
{
    fputs("Options:\n", stdout);
    while (l->name != NULL)
    {
        if ((l->value >= SHORTOPT_MIN) && (l->value <= SHORTOPT_MAX))
            fprintf(stdout, "  -%c", l->value);
        else
            fputs("    ", stdout);
        fprintf(stdout, "  --%s", l->name);
        if (l->arg)
            fprintf(stdout, "=<%s>", l->arg);
        fputc('\n', stdout);
        fputs(l->desc, stdout);
        l++;
    }
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
