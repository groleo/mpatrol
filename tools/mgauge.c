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
 * Memory allocation gauge.  Implements mgaugestart(), mgaugeend(),
 * mgaugeon() and mgaugeoff() using mpatrol to produce a simple gauge
 * showing the total allocated memory.
 */


/*
 * $Id: mgauge.c,v 1.2 2001-02-18 18:50:44 graeme Exp $
 */


#include "mgauge.h"
#include <stdio.h>
#include <ctype.h>


#define COLUMN_WIDTH  80  /* the column width in which to display the gauge */

#define DEFAULT_CHAR  '#' /* the default allocation character to use */
#define DELIMIT_CHAR  '|' /* the character to use as a gauge delimiter */
#define OVERFLOW_CHAR '+' /* the character to use when an overflow occurs */


typedef void (*epilogue_handler)(MP_CONST void *, MP_CONST void *);


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* The previous mpatrol epilogue handler.
 */

static epilogue_handler old_epilogue;


/* The file where the gauge is to be displayed.
 */

static FILE *gauge_file;


/* The character to use when displaying allocated memory.
 */

static unsigned char gauge_char;


/* The total number of bytes that can be shown in the gauge.
 */

static unsigned long gauge_size;


/* The flag indicating if the gauge is currently paused.
 */

static int gauge_pause;


/* The number of allocation events that must occur before the gauge is updated.
 */

static unsigned long update_count, update_frequency;


/* Update the memory allocation gauge.
 */

static
void
updategauge(void)
{
    static char b[COLUMN_WIDTH - 3];
    __mp_heapinfo h;
    unsigned long i, n;

    if (!gauge_pause && __mp_stats(&h))
    {
        n = (unsigned long) (((double) (COLUMN_WIDTH - 4) * (double) h.atotal) /
                             (double) gauge_size);
        if (n > COLUMN_WIDTH - 4)
            n = COLUMN_WIDTH - 4;
        for (i = 0; i < n; i++)
            b[i] = gauge_char;
        while (i < COLUMN_WIDTH - 4)
            b[i++] = ' ';
        b[i] = '\0';
        fprintf(gauge_file, "\r%c %s%c%c", DELIMIT_CHAR, b,
                (h.atotal > gauge_size) ? OVERFLOW_CHAR : ' ', DELIMIT_CHAR);
    }
}


/* Update the memory allocation gauge and possibly also call the old epilogue
 * function if one was installed.
 */

static
void
epilogue(MP_CONST void *p, MP_CONST void *a)
{
    if ((update_count++ % update_frequency) == 0)
        updategauge();
    if (old_epilogue != NULL)
        old_epilogue(p, a);
}


/* Open the gauge file.
 */

int
mgaugestart(MP_CONST char *f, unsigned char c, unsigned long s, unsigned long u)
{
    if (gauge_file != NULL)
        return 0;
    if ((f == NULL) || (*f == '\0') || (gauge_file = fopen(f, "w")))
    {
        if ((f == NULL) || (*f == '\0'))
            gauge_file = stderr;
        else
            setvbuf(gauge_file, NULL, _IONBF, 0);
        if ((c == '\0') || (c == DELIMIT_CHAR) || (c == OVERFLOW_CHAR) ||
            isspace(c) || !isprint(c))
            gauge_char = DEFAULT_CHAR;
        else
            gauge_char = c;
        if (s == 0)
            gauge_size = 1;
        else
            gauge_size = s;
        gauge_pause = 0;
        update_count = 1;
        if (u == 0)
            update_frequency = 1;
        else
            update_frequency = u;
        fprintf(gauge_file, "0 %*lu\n", COLUMN_WIDTH - 2, gauge_size);
        updategauge();
        old_epilogue = __mp_epilogue(epilogue);
        return 1;
    }
    return 0;
}


/* Close the gauge file.
 */

void
mgaugeend(void)
{
    if (gauge_file == NULL)
        return;
    __mp_epilogue(old_epilogue);
    updategauge();
    fputc('\n', gauge_file);
    if (gauge_file != stderr)
        fclose(gauge_file);
    gauge_file = NULL;
    gauge_char = 0;
    gauge_size = 0;
    gauge_pause = 0;
    update_count = update_frequency = 0;
}


/* Enable the gauge.
 */

void
mgaugeon(void)
{
    if (gauge_file == NULL)
        return;
    if (gauge_pause)
    {
        gauge_pause = 0;
        updategauge();
    }
}


/* Disable the gauge.
 */

void
mgaugeoff(void)
{
    if (gauge_file == NULL)
        return;
    if (!gauge_pause)
    {
        updategauge();
        gauge_pause = 1;
    }
}


#ifdef __cplusplus
}
#endif /* __cplusplus */