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


#include "signals.h"
#include "diag.h"
#include "inter.h"
#include <stdlib.h>
#include <signal.h>
#if MP_SIGINFO_SUPPORT
#include <siginfo.h>
#endif /* MP_SIGINFO_SUPPORT */


#if MP_IDENT_SUPPORT
#ident "$Id: signals.c,v 1.3 2000-01-09 20:08:55 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


#if TARGET == TARGET_UNIX || TARGET == TARGET_WINDOWS
/* Handles any signals that result from illegal memory accesses.
 */

#if MP_SIGINFO_SUPPORT
static void signalhandler(int s, siginfo_t *n, void *p)
#else /* MP_SIGINFO_SUPPORT */
static void signalhandler(int s)
#endif /* MP_SIGINFO_SUPPORT */
{
    infohead *h;
#if MP_SIGINFO_SUPPORT
    allocnode *t;
    void *a;
#endif /* MP_SIGINFO_SUPPORT */
    stackinfo i;

    h = __mp_memhead();
    __mp_printsummary(h);
#if MP_SIGINFO_SUPPORT && MP_WATCH_SUPPORT
    /* If we received a TRAP signal then we should only say that it is an
     * illegal memory access if it is a watch point error.  Otherwise we
     * just do the default and abort the process.
     */
    if ((s == SIGTRAP) && (n != NULL) && (n->si_code != TRAP_RWATCH) &&
        (n->si_code != TRAP_WWATCH) && (n->si_code != TRAP_XWATCH))
    {
        h->fini = 1;
        __mp_abort();
    }
#endif /* MP_SIGINFO_SUPPORT && MP_WATCH_SUPPORT */
    __mp_diag("\n");
#if MP_SIGINFO_SUPPORT
    if ((n != NULL) && (n->si_code > 0))
    {
        /* With the siginfo_t structure we can determine which address
         * caused the illegal memory access.  This is not always guaranteed
         * to be the exact byte location, but it is at least guaranteed that
         * it will be on the same page which will suffice for our purposes.
         */
        a = (void *) n->si_addr;
        __mp_error(AT_MAX, "illegal memory access at address " MP_POINTER, a);
        if (t = __mp_findnode(&h->alloc, a))
            if (t->info != NULL)
                __mp_printalloc(&h->syms, t);
            else
            {
                __mp_diag("    within free block " MP_POINTER " (", t->block);
                __mp_printsize(t->size);
                __mp_diag(")\n");
            }
        else
            __mp_diag("    " MP_POINTER " not in heap\n", a);
    }
    else
#endif /* MP_SIGINFO_SUPPORT */
        __mp_error(AT_MAX, "illegal memory access");
    /* Obtain the call stack so that we can tell where the illegal memory
     * access came from.  This relies on the assumption that the stack area
     * for the signal handler is the same as the stack area for the main
     * process.
     */
    __mp_newframe(&i);
    if (__mp_getframe(&i) && __mp_getframe(&i) && __mp_getframe(&i))
    {
        __mp_diag("\n    call stack\n");
        __mp_printstack(&h->syms, &i);
    }
    h->fini = 1;
    __mp_abort();
}
#endif /* TARGET */


/* Initialise a sighead structure.
 */

MP_GLOBAL void __mp_initsignals(sighead *s)
{
#if MP_SIGINFO_SUPPORT
    struct sigaction i;
#endif /* MP_SIGINFO_SUPPORT */

#if MP_SIGINFO_SUPPORT
    i.sa_flags = SA_SIGINFO;
    (void *) i.sa_handler = (void *) signalhandler;
    sigfillset(&i.sa_mask);
    sigaction(SIGSEGV, &i, NULL);
#if MP_WATCH_SUPPORT
    sigaction(SIGTRAP, &i, NULL);
#endif /* MP_WATCH_SUPPORT */
#elif TARGET == TARGET_UNIX || TARGET == TARGET_WINDOWS
    signal(SIGSEGV, signalhandler);
#endif /* MP_SIGINFO_SUPPORT && TARGET */
    s->sigint = s->sigterm = NULL;
    s->saved = 0;
}


/* Save the current signal handlers and set them to ignore.
 */

MP_GLOBAL void __mp_savesignals(sighead *s)
{
    s->sigint = signal(SIGINT, SIG_IGN);
    s->sigterm = signal(SIGTERM, SIG_IGN);
    s->saved = 1;
}


/* Restore the previous signal handlers.
 */

MP_GLOBAL void __mp_restoresignals(sighead *s)
{
    if (s->saved)
    {
        signal(SIGINT, s->sigint);
        signal(SIGTERM, s->sigterm);
        s->saved = 0;
    }
}


/* Send the current process an ABORT signal.
 */

MP_GLOBAL void __mp_abort(void)
{
#if TARGET == TARGET_UNIX || TARGET == TARGET_WINDOWS
    /* Send the current process an ABORT signal for use in a debugger.
     * Used on systems where this is supported.
     */
    abort();
#elif TARGET == TARGET_AMIGA || TARGET == TARGET_NETWARE
    /* Terminate the current process with a failure exit code.
     * Used on systems where memory will not be automatically returned
     * back to the system on process termination.
     */
    exit(EXIT_FAILURE);
#endif /* TARGET */
}


/* Provide a function which can be used as a breakpoint target in a debugger.
 */

MP_GLOBAL void __mp_trap(void)
{
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
