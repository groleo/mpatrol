#ifndef MP_SIGNALS_H
#define MP_SIGNALS_H


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
 * Signal-handling.  The setting of signal-handlers and their actions
 * is controlled from this module.
 */


#include "config.h"


/* A sighead structure contains details about the current status of signal
 * handlers.
 */

typedef struct sighead
{
    void (*sigint)(int);  /* interrupt signal handler */
    void (*sigterm)(int); /* termination signal handler */
}
sighead;


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


MP_EXPORT void __mp_initsignals(sighead *);
MP_EXPORT void __mp_savesignals(sighead *);
MP_EXPORT void __mp_restoresignals(sighead *);
MP_EXPORT void __mp_abort(void);
MP_EXPORT void __mp_trap(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* MP_SIGNALS_H */
