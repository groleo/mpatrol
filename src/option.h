#ifndef MP_OPTION_H
#define MP_OPTION_H


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
 * Option handling.  This module deals with the parsing of the option
 * string read from the configuration environment variable.
 */


#include "config.h"
#include "info.h"


/* Options that can be set at run-time.
 */

#define OPT_HELP          0
#define OPT_SETFLAGS      -1
#define OPT_UNSETFLAGS    -2
#define OPT_ALLOCSTOP     -3
#define OPT_REALLOCSTOP   -4
#define OPT_FREESTOP      -5
#define OPT_ALLOCBYTE     -6
#define OPT_FREEBYTE      -7
#define OPT_OFLOWBYTE     -8
#define OPT_OFLOWSIZE     -9
#define OPT_DEFALIGN      -10
#define OPT_LIMIT         -11
#define OPT_FAILFREQ      -12
#define OPT_FAILSEED      -13
#define OPT_UNFREEDABORT  -14
#define OPT_LOGFILE       -15
#define OPT_PROFFILE      -16
#define OPT_PROGFILE      -17
#define OPT_AUTOSAVE      -18
#define OPT_CHECK         -19
#define OPT_NOFREE        -20
#define OPT_SMALLBOUND    -21
#define OPT_MEDIUMBOUND   -22
#define OPT_LARGEBOUND    -23


/* Flags that can be set at run-time.
 */

#define OPT_SHOWALL       (OPT_SHOWMAP | OPT_SHOWSYMBOLS | OPT_SHOWFREED | \
                           OPT_SHOWUNFREED)
#define OPT_SHOWMAP       0x00000001
#define OPT_SHOWSYMBOLS   0x00000002
#define OPT_SHOWFREED     0x00000004
#define OPT_SHOWUNFREED   0x00000008
#define OPT_LOGALL        (OPT_LOGALLOCS | OPT_LOGREALLOCS | OPT_LOGFREES | \
                           OPT_LOGMEMORY)
#define OPT_LOGALLOCS     0x00000010
#define OPT_LOGREALLOCS   0x00000020
#define OPT_LOGFREES      0x00000040
#define OPT_LOGMEMORY     0x00000080
#define OPT_CHECKALL      (OPT_CHECKALLOCS | OPT_CHECKREALLOCS | \
                           OPT_CHECKFREES)
#define OPT_CHECKALLOCS   0x00000100
#define OPT_CHECKREALLOCS 0x00000200
#define OPT_CHECKFREES    0x00000400
#define OPT_PROF          0x00000800
#define OPT_SAFESIGNALS   0x00001000
#define OPT_NOPROTECT     0x00002000
#define OPT_PRESERVE      0x00004000
#define OPT_OFLOWWATCH    0x00008000
#define OPT_PAGEALLOC     0x00010000
#define OPT_ALLOCUPPER    0x00020000
#define OPT_USEMMAP       0x00040000
#define OPT_USEDEBUG      0x00080000
#define OPT_ALLOWOFLOW    0x00100000


/* The different types of errors when parsing options.
 */

typedef enum optionerr
{
    OE_UNRECOGNISED, /* unrecognised option */
    OE_RECOGNISED,   /* recognised option */
    OE_NOARGUMENT,   /* missing argument */
    OE_BADNUMBER,    /* bad numeric argument */
    OE_BADRANGE,     /* bad numeric range */
    OE_BIGNUMBER,    /* numeric argument too large */
    OE_LOWERORUPPER, /* LOWER or UPPER required */
    OE_IGNARGUMENT   /* ignoring argument */
}
optionerr;


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


MP_EXPORT void __mp_parseoptions(infohead *);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* MP_OPTION_H */
