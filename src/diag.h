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


MP_EXPORT char *__mp_functionnames[AT_MAX];


MP_EXPORT char *__mp_logfile(meminfo *, char *);
MP_EXPORT char *__mp_proffile(meminfo *, char *);
MP_EXPORT int __mp_openlogfile(char *);
MP_EXPORT int __mp_closelogfile(void);
MP_EXPORT void __mp_diag(char *, ...);
MP_EXPORT void __mp_warn(alloctype, char *, ...);
MP_EXPORT void __mp_error(alloctype, char *, ...);
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
