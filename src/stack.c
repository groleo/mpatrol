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
 * in order to ascertain the return address for a function.  However,
 * some operating systems provide support functions for doing this.
 */


#include "stack.h"
#include "memory.h"
#if !MP_BUILTINSTACK_SUPPORT && TARGET == TARGET_UNIX
#if MP_LIBRARYSTACK_SUPPORT
#if SYSTEM == SYSTEM_IRIX
#include <exception.h>
#include <ucontext.h>
#endif /* SYSTEM */
#else /* MP_LIBRARYSTACK_SUPPORT */
#include <setjmp.h>
#if ARCH == ARCH_MIPS || ARCH == ARCH_SPARC
#include <ucontext.h>
#if ARCH == ARCH_SPARC
#ifndef R_SP
#define R_SP REG_SP
#endif /* R_SP */
#endif /* ARCH */
#endif /* ARCH */
#endif /* MP_LIBRARYSTACK_SUPPORT */
#endif /* MP_BUILTINSTACK_SUPPORT && TARGET */


#if MP_IDENT_SUPPORT
#ident "$Id: stack.c,v 1.10 2000-06-07 20:10:25 graeme Exp $"
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
#elif !MP_LIBRARYSTACK_SUPPORT */
#if TARGET == TARGET_UNIX && ARCH == ARCH_MIPS
/* These macros are used by the unwind() function for setting flags when
 * certain instructions are seen.
 */

#define SP_OFFSET   1 /* stack pointer offset has been set */
#define PC_OFFSET   2 /* program counter offset has been set */
#define CONST_LOWER 4 /* lower part of stack pointer offset has been set */
#define CONST_UPPER 8 /* upper part of stack pointer offset has been set */
#endif /* TARGET && ARCH */
#endif /* MP_BUILTINSTACK_SUPPORT && MP_LIBRARYSTACK_SUPPORT */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


#if !MP_BUILTINSTACK_SUPPORT && TARGET == TARGET_UNIX
#if MP_LIBRARYSTACK_SUPPORT
#if SYSTEM == SYSTEM_HPUX
/* The following functions are defined in the HP/UX traceback library (libcl).
 */

frameinfo U_get_current_frame(void);
int U_get_previous_frame(frameinfo *, frameinfo *);
#elif SYSTEM == SYSTEM_IRIX
/* The unwind() function in the IRIX exception-handling library (libexc) calls
 * malloc() and several memory operation functions, so we need to guard against
 * this by preventing recursive calls.
 */

static unsigned char recursive;
#endif /* SYSTEM */
#else /* MP_LIBRARYSTACK_SUPPORT */
static jmp_buf environment;
static void (*bushandler)(int);
static void (*segvhandler)(int);
#endif /* MP_LIBRARYSTACK_SUPPORT */
#endif /* MP_BUILTINSTACK_SUPPORT && TARGET */


/* Initialise the fields of a stackinfo structure.
 */

MP_GLOBAL void __mp_newframe(stackinfo *s)
{
    s->frame = s->addr = NULL;
#if MP_BUILTINSTACK_SUPPORT
    for (s->index = 0; s->index < MP_MAXSTACK; s->index++)
        s->frames[s->index] = s->addrs[s->index] = NULL;
    s->index = 0;
#elif MP_LIBRARYSTACK_SUPPORT
#if SYSTEM == SYSTEM_HPUX
    __mp_memset(&s->next, 0, sizeof(frameinfo));
#elif SYSTEM == SYSTEM_IRIX
    __mp_memset(&s->next, 0, sizeof(struct sigcontext));
#endif /* SYSTEM */
#else /* MP_BUILTINSTACK_SUPPORT && MP_LIBRARYSTACK_SUPPORT */
#if TARGET == TARGET_UNIX && ARCH == ARCH_MIPS
    s->next.sp = s->next.pc = 0;
#else /* TARGET && ARCH */
    s->next = NULL;
#endif /* TARGET && ARCH */
#endif /* MP_BUILTINSTACK_SUPPORT && MP_LIBRARYSTACK_SUPPORT */
}


#if !MP_BUILTINSTACK_SUPPORT && !MP_LIBRARYSTACK_SUPPORT && \
    TARGET == TARGET_UNIX
/* Handles any signals that result from illegal memory accesses whilst
 * traversing the call stack.
 */

static void stackhandler(int s)
{
    longjmp(environment, 1);
}
#endif /* MP_BUILTINSTACK_SUPPORT && MP_LIBRARYSTACK_SUPPORT && TARGET */


#if !MP_BUILTINSTACK_SUPPORT && !MP_LIBRARYSTACK_SUPPORT
#if (TARGET == TARGET_UNIX && (ARCH == ARCH_IX86 || ARCH == ARCH_M68K || \
      ARCH == ARCH_M88K || ARCH == ARCH_POWER || ARCH == ARCH_POWERPC || \
      ARCH == ARCH_SPARC)) || ((TARGET == TARGET_WINDOWS || \
      TARGET == NETWARE) && ARCH == ARCH_IX86)
/* Obtain the return address for the specified stack frame handle.
 */

static unsigned int *getaddr(unsigned int *p)
{
    unsigned int *a;

    /* This function relies heavily on the stack frame format of supported
     * OS / processor combinations.  A better way to determine the return
     * address would be to perform code reading, but on CISC processors this
     * could be a nightmare.
     */
#if ARCH == ARCH_IX86 || ARCH == ARCH_M68K || ARCH == ARCH_M88K
    a = (unsigned int *) *(p + 1);
#elif ARCH == ARCH_POWER || ARCH == ARCH_POWERPC
    a = (unsigned int *) *(p + 2);
#elif ARCH == ARCH_SPARC
    if (a = (unsigned int *) *((unsigned int *) *p + 15))
        a += 2;
#endif /* ARCH */
    return a;
}
#endif /* TARGET && ARCH */
#endif /* MP_BUILTINSTACK_SUPPORT && MP_LIBRARYSTACK_SUPPORT */


#if !MP_BUILTINSTACK_SUPPORT && !MP_LIBRARYSTACK_SUPPORT
#if TARGET == TARGET_UNIX && ARCH == ARCH_MIPS
/* Determine the stack pointer and return address of the previous stack frame.
 */

static int unwind(frameinfo *f)
{
    long p, s;
    unsigned long a, i, q;
    unsigned short l, u;

    s = -1;
    p = 0;
    q = 0xFFFFFFFF;
    l = u = 0;
    a = 0;
    /* Search for the program counter offset in the stack frame.
     */
    while (!((a & SP_OFFSET) && (a & PC_OFFSET)) && (f->pc < q))
    {
        i = *((unsigned long *) f->pc);
        if (i == 0x03E00008)
        {
            /* jr ra */
            q = f->pc + 8;
        }
        else if (i == 0x03A1E821)
        {
            /* addu sp,sp,at */
            s = 0;
            a |= SP_OFFSET;
        }
        else
            switch (i >> 16)
            {
              case 0x27BD:
                /* addiu sp,sp,?? */
                s = i & 0xFFFF;
                a |= SP_OFFSET;
                break;
              case 0x3401:
                /* ori at,zero,?? */
                l = i & 0xFFFF;
                u = 0;
                a |= CONST_LOWER;
                break;
              case 0x3421:
                /* ori at,at,?? */
                l = i & 0xFFFF;
                a |= CONST_LOWER;
                break;
              case 0x3C01:
                /* lui at,?? */
                l = 0;
                u = i & 0xFFFF;
                a |= CONST_UPPER;
                break;
              case 0x8FBF:
                /* lw ra,??(sp) */
                p = i & 0xFFFF;
                a |= PC_OFFSET;
                break;
            }
        f->pc += 4;
    }
    if ((s == 0) && ((a & CONST_LOWER) || (a & CONST_UPPER)))
        s = (u << 16) | l;
    if ((s > 0) && (i = ((unsigned long *) f->sp)[p >> 2]) &&
        (*((unsigned long *) (i - 8)) == 0x0320F809))
    {
        /* jalr ra,t9 */
        f->sp += s;
        f->pc = i;
        return 1;
    }
    return 0;
}
#endif /* TARGET && ARCH */
#endif /* MP_BUILTINSTACK_SUPPORT && MP_LIBRARYSTACK_SUPPORT */


#if !MP_BUILTINSTACK_SUPPORT && !MP_LIBRARYSTACK_SUPPORT && \
    TARGET == TARGET_UNIX
#if ARCH == ARCH_MIPS
/* Determine the stack pointer and return address of the current stack frame.
 */

static int getframe(frameinfo *f)
{
    ucontext_t c;

    if (getcontext(&c) == -1)
        return 0;
    f->sp = c.uc_mcontext.gregs[CTX_SP];
    f->pc = c.uc_mcontext.gregs[CTX_RA];
    return 1;
}
#elif ARCH == ARCH_SPARC
/* Return a handle for the frame pointer at the current point in execution.
 */

static unsigned int *getframe(void)
{
    ucontext_t c;

    if (getcontext(&c) == -1)
        return NULL;
    return (unsigned int *) c.uc_mcontext.gregs[R_SP] + 14;
}
#endif /* ARCH */
#endif /* MP_BUILTINSTACK_SUPPORT && MP_LIBRARYSTACK_SUPPORT && TARGET */


/* Return a handle for the stack frame at the current point in execution
 * or the next stack frame in the call stack.
 */

MP_GLOBAL int __mp_getframe(stackinfo *p)
{
#if MP_BUILTINSTACK_SUPPORT
    void *f;
#elif MP_LIBRARYSTACK_SUPPORT
#if SYSTEM == SYSTEM_HPUX
    frameinfo f;
#endif /* SYSTEM */
#else /* MP_BUILTINSTACK_SUPPORT && MP_LIBRARYSTACK_SUPPORT */
#if (TARGET == TARGET_UNIX && (ARCH == ARCH_IX86 || ARCH == ARCH_M68K || \
      ARCH == ARCH_M88K || ARCH == ARCH_POWER || ARCH == ARCH_POWERPC || \
      ARCH == ARCH_SPARC)) || ((TARGET == TARGET_WINDOWS || \
      TARGET == NETWARE) && ARCH == ARCH_IX86)
    unsigned int *f;
#endif /* TARGET && ARCH */
#endif /* MP_BUILTINSTACK_SUPPORT && MP_LIBRARYSTACK_SUPPORT */
    int r;

    r = 0;
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
        r = 1;
    }
    else
    {
        p->frame = NULL;
        p->addr = NULL;
        p->index = MP_MAXSTACK;
    }
#elif MP_LIBRARYSTACK_SUPPORT
    /* HP/UX and IRIX provide a library for traversing function call stack
     * frames since the stack frame format does not preserve frame pointers.
     * On HP/UX this is done via a special section which can be read by
     * debuggers.
     */
#if SYSTEM == SYSTEM_HPUX
    if (p->frame == NULL)
    {
        p->next = U_get_current_frame();
        if (U_get_previous_frame(&p->next, &f) == 0)
        {
            p->next.size = f.size;
            p->next.sp = f.sp;
            p->next.ps = f.ps;
            p->next.pc = f.pc;
            p->next.dp = f.dp;
        }
        else
            __mp_memset(&p->next, 0, sizeof(frameinfo));
    }
    if (p->next.pc && (U_get_previous_frame(&p->next, &f) == 0))
    {
        p->frame = (void *) p->next.sp;
        p->addr = (void *) (p->next.pc & ~3);
        p->next.size = f.size;
        p->next.sp = f.sp;
        p->next.ps = f.ps;
        p->next.pc = f.pc;
        p->next.dp = f.dp;
        r = 1;
    }
    else
    {
        p->frame = NULL;
        p->addr = NULL;
    }
#elif SYSTEM == SYSTEM_IRIX
    /* On IRIX, the unwind() function calls malloc(), free() and some memory
     * operation functions every time it is invoked.  Despite the fact that we
     * guard against recursion here, it slows down execution to an unbearable
     * pace, so it might be an idea to remove malloc.c from the mpatrol library
     * if you have the option of recompiling all of your sources to include
     * mpatrol.h.
     */
    if (!recursive)
    {
        recursive = 1;
        if (p->frame == NULL)
        {
            exc_setjmp(&p->next);
            unwind(&p->next, NULL);
        }
        if (p->next.sc_pc != 0)
        {
            /* On IRIX, the sigcontext structure stores registers in 64-bit
             * format so we must be careful when converting them to 32-bit
             * quantities.
             */
            p->frame = (void *) p->next.sc_regs[CTX_SP];
            p->addr = (void *) p->next.sc_pc;
            unwind(&p->next, NULL);
            r = 1;
        }
        else
        {
            p->frame = NULL;
            p->addr = NULL;
        }
        recursive = 0;
    }
#endif /* SYSTEM */
#else /* MP_BUILTINSTACK_SUPPORT && MP_LIBRARYSTACK_SUPPORT */
#if (TARGET == TARGET_UNIX && (ARCH == ARCH_IX86 || ARCH == ARCH_M68K || \
      ARCH == ARCH_M88K || ARCH == ARCH_POWER || ARCH == ARCH_POWERPC || \
      ARCH == ARCH_SPARC)) || ((TARGET == TARGET_WINDOWS || \
      TARGET == NETWARE) && ARCH == ARCH_IX86)
    /* This section is not complete in any way for the OS / processor
     * combinations it supports, as it is intended to be as portable as possible
     * without writing in assembler.  In particular, optimised code is likely
     * to cause major problems for stack traversal on some platforms.
     */
#if TARGET == TARGET_UNIX
    bushandler = signal(SIGBUS, stackhandler);
    segvhandler = signal(SIGSEGV, stackhandler);
    if (setjmp(environment))
        __mp_newframe(p);
    else
#endif /* TARGET */
    {
        if (p->frame == NULL)
#if ARCH == ARCH_IX86 || ARCH == ARCH_M68K
            f = (unsigned int *) &p - 2;
#elif ARCH == ARCH_M88K
            f = (unsigned int *) &p - 4;
#elif ARCH == ARCH_POWER || ARCH == ARCH_POWERPC
            f = (unsigned int *) &p - 6;
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
#if ARCH == ARCH_IX86 || ARCH == ARCH_M68K || ARCH == ARCH_M88K || \
    ARCH == ARCH_POWER || ARCH == ARCH_POWERPC
#if SYSTEM == SYSTEM_LYNXOS
            if (!getaddr((unsigned int *) *f))
                p->next = NULL;
            else
#endif /* SYSTEM */
                p->next = (void *) *f;
#elif ARCH == ARCH_SPARC
            if (p->addr == NULL)
                p->next = NULL;
            else
                p->next = (void *) ((unsigned int *) *f + 14);
#endif /* ARCH */
            r = 1;
        }
    }
#if TARGET == TARGET_UNIX
    signal(SIGBUS, bushandler);
    signal(SIGSEGV, segvhandler);
#endif /* TARGET */
#endif /* TARGET && ARCH */
#endif /* MP_BUILTINSTACK_SUPPORT && MP_LIBRARYSTACK_SUPPORT */
    return r;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
