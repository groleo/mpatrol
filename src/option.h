#ifndef MP_OPTION_H
#define MP_OPTION_H


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
 * Option handling.  This module deals with the parsing of the option
 * string read from the configuration environment variable.
 */


#include "config.h"
#include "info.h"


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
