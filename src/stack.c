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
 * Call stacks.  The method for traversing a function call stack is
 * dependent on both the operating system and processor architecture.
 * The most correct way of doing this would be to perform code-reading
 * in order to ascertain the return address for a function.
 */


#include "stack.h"
#if TARGET == TARGET_UNIX && ARCH == ARCH_SPARC && !MP_BUILTINSTACK_SUPPORT
#include <ucontext.h>
#ifndef R_SP
#define R_SP REG_SP
#endif /* R_SP */
#endif /* TARGET && ARCH && MP_BUILTINSTACK_SUPPORT */


#if MP_IDENT_SUPPORT
#ident "$Id: stack.c,v 1.3 2000-01-09 20:35:21 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#if MP_BUILTINSTACK_SUPPORT

/* This method of call stack traversal uses two special builtin functions in
 * gcc called __builtin_frame_address() and __builtin_return_address().  Both
 * of these functions take the number of stack frames to traverse as a parameter
 * but this must currently be a constant, hence the reason for all of the
 * following complicated macros, and for the fact that there must currently
 * be a maximum number of stack frames to traverse that is determined at compile
 * time.  However, it may be the case that this method is slightly better than
 * manually traversing the call stack.  Perhaps in the future gcc might allow
 * these functions to accept non-constant parameters...
 */

#define frameaddress(v, n) (v[n] = __builtin_frame_address(n))
#define frameaddress1(v) frameaddress(v, 0)
#define frameaddress2(v) frameaddress1(v) && frameaddress(v, 1)
#define frameaddress3(v) frameaddress2(v) && frameaddress(v, 2)
#define frameaddress4(v) frameaddress3(v) && frameaddress(v, 3)
#define frameaddress5(v) frameaddress4(v) && frameaddress(v, 4)
#define frameaddress6(v) frameaddress5(v) && frameaddress(v, 5)
#define frameaddress7(v) frameaddress6(v) && frameaddress(v, 6)
#define frameaddress8(v) frameaddress7(v) && frameaddress(v, 7)
#define frameaddressn(v, w, n) if (frameaddress ## n(v)) \
                                   w = __builtin_frame_address(n)
#define frameaddresses(v, w, n) frameaddressn(v, w, n)

#define returnaddress(v, n) (v[n] = __builtin_return_address(n))
#define returnaddress1(v) returnaddress(v, 0)
#define returnaddress2(v) returnaddress1(v) && returnaddress(v, 1)
#define returnaddress3(v) returnaddress2(v) && returnaddress(v, 2)
#define returnaddress4(v) returnaddress3(v) && returnaddress(v, 3)
#define returnaddress5(v) returnaddress4(v) && returnaddress(v, 4)
#define returnaddress6(v) returnaddress5(v) && returnaddress(v, 5)
#define returnaddress7(v) returnaddress6(v) && returnaddress(v, 6)
#define returnaddress8(v) returnaddress7(v) && returnaddress(v, 7)
#define returnaddressn(v, w, n) if (returnaddress ## n(v)) \
                                    w = __builtin_return_address(n)
#define returnaddresses(v, w, n) returnaddressn(v, w, n)

#if MP_MAXSTACK > 8
#error not enough frameaddress() and returnaddress() macros
#endif /* MP_MAXSTACK */
#endif /* MP_BUILTINSTACK_SUPPORT */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* Initialise the fields of a stackinfo structure.
 */

MP_GLOBAL void __mp_newframe(stackinfo *s)
{
    s->frame = s->addr = NULL;
#if MP_BUILTINSTACK_SUPPORT
    for (s->index = 0; s->index < MP_MAXSTACK; s->index++)
        s->frames[s->index] = s->addrs[s->index] = NULL;
    s->index = 0;
#else /* MP_BUILTINSTACK_SUPPORT */
    s->next = NULL;
#endif /* MP_BUILTINSTACK_SUPPORT */
}


#if !MP_BUILTINSTACK_SUPPORT
#if (TARGET == TARGET_UNIX && (ARCH == ARCH_IX86 || ARCH == ARCH_M68K || \
      ARCH == ARCH_M88K || ARCH == ARCH_SPARC)) || \
    ((TARGET == TARGET_WINDOWS || TARGET == NETWARE) && ARCH == ARCH_IX86)
/* Obtain the return address for the specified stack frame handle.
 */

static unsigned int *getaddr(unsigned int *p)
{
    unsigned int *a;

    /* This function relies heavily on the stack frame format of supported
     * OS / processor combinations.  A better way to determine the return
     * address would be to perform code reading.
     */
#if ARCH == ARCH_IX86 || ARCH == ARCH_M68K || ARCH == ARCH_M88K
    a = (unsigned int *) *(p + 1);
#elif ARCH == ARCH_SPARC
    if (a = (unsigned int *) *((unsigned int *) *p + 15))
        a += 2;
#endif /* ARCH */
    return a;
}
#endif /* TARGET && ARCH */
#endif /* MP_BUILTINSTACK_SUPPORT */


#if TARGET == TARGET_UNIX && ARCH == ARCH_SPARC && !MP_BUILTINSTACK_SUPPORT
/* Return a handle for the frame pointer at the current point in execution.
 */

static unsigned int *getframe(void)
{
    ucontext_t c;

    if (getcontext(&c) == -1)
        return NULL;
    return (unsigned int *) c.uc_mcontext.gregs[R_SP] + 14;
}
#endif /* TARGET && ARCH && MP_BUILTINSTACK_SUPPORT */


/* Return a handle for the stack frame at the current point in execution
 * or the next stack frame in the call stack.
 */

MP_GLOBAL int __mp_getframe(stackinfo *p)
{
#if MP_BUILTINSTACK_SUPPORT
    void *f;
#else /* MP_BUILTINSTACK_SUPPORT */
#if (TARGET == TARGET_UNIX && (ARCH == ARCH_IX86 || ARCH == ARCH_M68K || \
      ARCH == ARCH_M88K || ARCH == ARCH_SPARC)) || \
    ((TARGET == TARGET_WINDOWS || TARGET == NETWARE) && ARCH == ARCH_IX86)
    unsigned int *f;
#endif /* TARGET && ARCH */
#endif /* MP_BUILTINSTACK_SUPPORT */

#if MP_BUILTINSTACK_SUPPORT
    if (p->index == 0)
    {
        /* We use the macros defined above to fill in the arrays of frame
         * pointers and return addresses.  These macros rely on the fact that
         * if there are no more frames in the call stack then the builtin
         * functions will return NULL.  If this is not the case then the
         * library will crash.
         */
        frameaddresses(p->frames, f, MP_MAXSTACK);
        returnaddresses(p->addrs, f, MP_MAXSTACK);
    }
    if ((p->index++ < MP_MAXSTACK) && (p->frame = p->frames[p->index - 1]))
    {
        p->addr = p->addrs[p->index - 1];
        return 1;
    }
    p->frame = NULL;
    p->addr = NULL;
    p->index = MP_MAXSTACK;
#else /* MP_BUILTINSTACK_SUPPORT */
#if (TARGET == TARGET_UNIX && (ARCH == ARCH_IX86 || ARCH == ARCH_M68K || \
      ARCH == ARCH_M88K || ARCH == ARCH_SPARC)) || \
    ((TARGET == TARGET_WINDOWS || TARGET == NETWARE) && ARCH == ARCH_IX86)
    /* This function is not complete in any way for the OS / processor
     * combinations it supports, as it is intended to be as portable as possible
     * without writing in assembler.  In particular, optimised code is likely
     * to cause major problems for stack traversal on some platforms.
     */
    if (p->frame == NULL)
#if ARCH == ARCH_IX86 || ARCH == ARCH_M68K
        f = (unsigned int *) &p - 2;
#elif ARCH == ARCH_M88K
        f = (unsigned int *) &p - 4;
#elif ARCH == ARCH_SPARC
        f = getframe();
#endif /* ARCH */
    else
        f = (unsigned int *) p->next;
    if (p->frame = f)
    {
        p->addr = getaddr(f);
        /* We cache the next frame pointer in the call stack since on some
         * systems it may be overwritten by another call.
         */
#if ARCH == ARCH_IX86 || ARCH == ARCH_M68K || ARCH == ARCH_M88K
        p->next = (void *) *f;
#elif ARCH == ARCH_SPARC
        if (p->addr == NULL)
            p->next = NULL;
        else
            p->next = (void *) ((unsigned int *) *f + 14);
#endif /* ARCH */
        return 1;
    }
#endif /* TARGET && ARCH */
#endif /* MP_BUILTINSTACK_SUPPORT */
    return 0;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
