#ifndef MP_DIAG_H
#define MP_DIAG_H


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
 * Diagnostics.  All mpatrol library diagnostics have to pass through this
 * interface, which shields the rest of the library from the details of where
 * the diagnostics actually go.
 */


#include "config.h"
#include "info.h"


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* The different types of warnings and errors reported by the mpatrol library.
 */

typedef enum errortype
{
    ET_ALLOVF, /* allocation %1 has a corrupted overflow buffer at %2 */
    ET_ALLZER, /* attempt to create an allocation of size 0 */
    ET_BADALN, /* alignment %1 is not a power of two */
    ET_FRDCOR, /* freed allocation %1 has memory corruption at %2 */
    ET_FRDOPN, /* attempt to perform operation on freed memory */
    ET_FRDOVF, /* freed allocation %1 has a corrupted overflow buffer at %2 */
    ET_FRECOR, /* free memory corruption at %1 */
    ET_FRENUL, /* attempt to free a NULL pointer */
    ET_FREOPN, /* attempt to perform operation on free memory */
    ET_ILLMEM, /* illegal memory access at address %1 */
    ET_INCOMP, /* %1 was allocated with %2 */
    ET_MAXALN, /* alignment %1 is greater than the system page size */
    ET_MISMAT, /* %1 does not match allocation of %2 */
    ET_NOTALL, /* %1 has not been allocated */
    ET_NULOPN, /* attempt to perform operation on a NULL pointer */
    ET_OUTMEM, /* out of memory */
    ET_PRVFRD, /* %1 was freed with %2 */
    ET_RNGOVF, /* range [%1,%2] overflows [%3,%4] */
    ET_RNGOVL, /* range [%1,%2] overlaps [%3,%4] */
    ET_RSZNUL, /* attempt to resize a NULL pointer */
    ET_RSZZER, /* attempt to resize an allocation to size 0 */
    ET_STROVF, /* string %1 overflows [%2,%3] */
    ET_ZERALN, /* alignment 0 is invalid */
    ET_MAX
}
errortype;


MP_EXPORT char *__mp_functionnames[AT_MAX];


MP_EXPORT char *__mp_logfile(meminfo *, char *);
MP_EXPORT char *__mp_proffile(meminfo *, char *);
MP_EXPORT int __mp_openlogfile(char *);
MP_EXPORT int __mp_closelogfile(void);
MP_EXPORT void __mp_diag(char *, ...);
MP_EXPORT void __mp_warn(errortype, alloctype, char *, unsigned long, char *,
                         ...);
MP_EXPORT void __mp_error(errortype, alloctype, char *, unsigned long, char *,
                          ...);
MP_EXPORT void __mp_printmemory(void *, size_t);
MP_EXPORT void __mp_printsize(size_t);
MP_EXPORT void __mp_printtype(infonode *);
MP_EXPORT void __mp_printloc(infonode *);
MP_EXPORT void __mp_printsymbol(symhead *, void *);
MP_EXPORT void __mp_printsymbols(symhead *);
MP_EXPORT void __mp_printaddrs(symhead *, addrnode *);
MP_EXPORT void __mp_printstack(symhead *, stackinfo *);
MP_EXPORT void __mp_printalloc(symhead *, allocnode *);
MP_EXPORT void __mp_logalloc(infohead *, size_t, size_t, alloctype, char *,
                             char *, unsigned long, stackinfo *);
MP_EXPORT void __mp_logrealloc(infohead *, void *, size_t, size_t, alloctype,
                               char *, char *, unsigned long, stackinfo *);
MP_EXPORT void __mp_logfree(infohead *, void *, alloctype, char *, char *,
                            unsigned long, stackinfo *);
MP_EXPORT void __mp_logmemset(infohead *, void *, size_t, unsigned char,
                              alloctype, char *, char *, unsigned long,
                              stackinfo *);
MP_EXPORT void __mp_logmemcopy(infohead *, void *, void *, size_t,
                               unsigned char, alloctype, char *, char *,
                               unsigned long, stackinfo *);
MP_EXPORT void __mp_logmemlocate(infohead *, void *, size_t, void *, size_t,
                                 alloctype, char *, char *, unsigned long,
                                 stackinfo *);
MP_EXPORT void __mp_logmemcompare(infohead *, void *, void *, size_t, alloctype,
                                  char *, char *, unsigned long, stackinfo *);
MP_EXPORT void __mp_printallocs(infohead *, int);
MP_EXPORT void __mp_printfreed(infohead *);
MP_EXPORT void __mp_printmap(infohead *);
MP_EXPORT void __mp_printversion(void);
MP_EXPORT void __mp_printsummary(infohead *);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* MP_DIAG_H */
