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
 * Processor-dependent assembler routines.  This module contains any routines
 * required to support a particular processor that could not be written in C
 * or C++.  Despite the filename extension, this is not a C source file, but
 * is named this way so that it can be preprocessed by the C compiler before
 * being assembled.
 */


#include "config.h"


#if MP_THREADS_SUPPORT && MP_INIT_SUPPORT
/* Provide support for initialising the mpatrol mutexes and data structures
 * before main() is called.  This is preferred over any other such solutions
 * on the following systems since it means that we can link with any compiler
 * rather than relying on linking with the same compiler that built the mutex
 * module.
 */


#if ARCH == ARCH_IX86
/* Define the __mp_initsection variable.
 */

	.section .data,"aw"
	.align	2
__mp_initsection:
	.long	1
	.globl	__mp_initsection
	.type	__mp_initsection,@object
	.size	__mp_initsection,4


/* Place calls to initialise the mpatrol mutexes and data structures into
 * the .init section.
 */

	.section .init,"ax"
	call	__mp_initmutexes
	call	__mp_init
#elif ARCH == ARCH_M68K
/* Define the __mp_initsection variable.
 */

	.section .data,"aw",@progbits
	.align	2
__mp_initsection:
	.long	1
	.globl	__mp_initsection
	.type	__mp_initsection,@object
	.size	__mp_initsection,4


/* Place calls to initialise the mpatrol mutexes and data structures into
 * the .init section.
 */

	.section .init,"ax",@progbits
	jbsr	__mp_initmutexes
	jbsr	__mp_init
#elif ARCH == ARCH_M88K
/* Define the __mp_initsection variable.
 */

	section	.data,"aw",#progbits
	align	4
___mp_initsection:
	word	1
	global	___mp_initsection
	type	___mp_initsection,#object
	size	___mp_initsection,4


/* Place calls to initialise the mpatrol mutexes and data structures into
 * the .init section.
 */

	section	.init,"ax",#progbits
	bsr	___mp_initmutexes
	bsr	___mp_init
#elif ARCH == ARCH_SPARC
/* Define the __mp_initsection variable.
 */

	.section ".data",#alloc,#write
	.align	4
__mp_initsection:
	.word	1
	.global	__mp_initsection
	.type	__mp_initsection,#object
	.size	__mp_initsection,4


/* Place calls to initialise the mpatrol mutexes and data structures into
 * the .init section.
 */

	.section ".init",#alloc,#execinstr
	call	__mp_initmutexes
	nop
	call	__mp_init
	nop
#endif /* ARCH */
#endif /* MP_THREADS_SUPPORT && MP_INIT_SUPPORT */


#if !MP_BUILTINSTACK_SUPPORT
#if MP_LIBRARYSTACK_SUPPORT && SYSTEM == SYSTEM_HPUX
/* A similar function to this also resides in the system traceback library,
 * libcl, but it must belong to the executable module rather than reside in
 * a shared library, hence its inclusion here.
 */


/* Fill in the frameinfo structure for the current function.
 */

	.CODE
__mp_frameinfo
	.PROC
	.CALLINFO
	.ENTRY
	stw	sp,4(arg0)
	mfsp	sr4,r20
	stw	r20,8(arg0)
	stw	rp,12(arg0)
	stw	dp,16(arg0)
	stw	r3,36(arg0)
	stw	r4,44(arg0)
	stw	r0,0(arg0)
	stw	r0,20(arg0)
	stw	r19,40(arg0)
	bv	r0(rp)
	.EXIT
	stw	r0,24(arg0)
	.PROCEND
	.EXPORT	__mp_frameinfo,CODE,PRIV_LEV=3
	.END
#elif !MP_LIBRARYSTACK_SUPPORT && ARCH == ARCH_MIPS
/* Not all MIPS platforms have the getcontext() function, and even those that
 * do may not enter the correct return address into the ucontext_t structure.
 * So, unfortunately we have to provide assembler routines to obtain the
 * correct information.
 */


/* Obtain the stack pointer for the current function.
 */

	.text
	.globl	__mp_stackpointer
	.ent	__mp_stackpointer
__mp_stackpointer:
	.frame	$29,0,$31
	move	$2,$29
	j	$31
	.end	__mp_stackpointer


/* Obtain the return address for the current function.
 */

	.text
	.globl	__mp_returnaddress
	.ent	__mp_returnaddress
__mp_returnaddress:
	.frame	$29,0,$31
	move	$2,$31
	j	$31
	.end	__mp_returnaddress
#endif /* MP_LIBRARYSTACK_SUPPORT && SYSTEM && ARCH */
#endif /* MP_BUILTINSTACK_SUPPORT */
