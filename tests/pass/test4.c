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
 * Demonstrates and tests the facility for obtaining information about
 * the allocation a specific address belongs to.
 */


#include "mpatrol.h"
#include <stdio.h>


void display(void *p)
{
    __mp_allocstack *s;
    __mp_allocinfo d;

    if (!__mp_info(p, &d))
    {
        if (sizeof(void *) == 8)
            fprintf(stderr, "nothing known about address 0x%016lX\n", p);
        else
            fprintf(stderr, "nothing known about address 0x%08lX\n", p);
        return;
    }
    if (sizeof(void *) == 8)
        fprintf(stderr, "block:   0x%016lX\n", d.block);
    else
        fprintf(stderr, "block:   0x%08lX\n", d.block);
    fprintf(stderr, "size:    %lu\n", d.size);
    fprintf(stderr, "type:    %lu\n", d.type);
    fprintf(stderr, "alloc:   %lu\n", d.alloc);
    fprintf(stderr, "realloc: %lu\n", d.realloc);
    fprintf(stderr, "func:    %s\n", d.func ? d.func : "NULL");
    fprintf(stderr, "file:    %s\n", d.file ? d.file : "NULL");
    fprintf(stderr, "line:    %lu\n", d.line);
    for (s = d.stack; s != NULL; s = s->next)
    {
        if (sizeof(void *) == 8)
            fprintf(stderr, "\t0x%016lX: ", s->addr);
        else
            fprintf(stderr, "\t0x%08lX: ", s->addr);
        fprintf(stderr, "%s\n", s->name ? s->name : "NULL");
    }
    fprintf(stderr, "freed:   %d\n", d.freed);
}


void func2(void)
{
    void *p;

    if (p = malloc(16))
    {
        display(p);
        free(p);
    }
    display(p);
}


void func1(void)
{
    func2();
}


int main(void)
{
    func1();
    return EXIT_SUCCESS;
}
