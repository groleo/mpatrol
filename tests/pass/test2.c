/*
 * mpatrol
 * A library for controlling and tracing dynamic memory allocations.
 * Copyright (C) 1997-1999 Graeme S. Roy <graeme@epc.co.uk>
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
 * Demonstrates and tests the facility for specifying user-defined prologue
 * and epilogue functions.
 */


#include "mpatrol.h"
#include <stdio.h>


void prologue(void *p, size_t l)
{
    if (p == (void *) -1)
        fprintf(stderr, "allocating %lu bytes\n", l);
    else if (l == (size_t) -1)
        fprintf(stderr, "freeing allocation 0x%08lX\n", p);
    else if (l == (size_t) -2)
        fprintf(stderr, "duplicating string `%s'\n", p);
    else
        fprintf(stderr, "reallocating allocation 0x%08lX to %lu bytes\n", p, l);
}


void epilogue(void *p)
{
    if (p != (void *) -1)
        fprintf(stderr, "allocation returns 0x%08lX\n", p);
}


int main(void)
{
    void *p, *q;

    __mp_prologue(prologue);
    __mp_epilogue(epilogue);
    if (p = malloc(16))
        if (q = realloc(p, 32))
            free(q);
        else
            free(p);
    if (p = (char *) strdup("test"))
        free(p);
    return EXIT_SUCCESS;
}
