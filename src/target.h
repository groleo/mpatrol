#ifndef MP_TARGET_H
#define MP_TARGET_H


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
 * Target definitions.  These are automatically deduced from the
 * preprocessor macros defined by the compiler on the host system,
 * but they can also be overridden.
 */


#define TARGET_UNIX    1 /* UNIX or UNIX-like */
#define TARGET_AMIGA   2 /* Commodore AmigaOS */
#define TARGET_WINDOWS 3 /* Microsoft Windows 95/98/NT */
#define TARGET_NETWARE 4 /* Novell Netware */


#ifndef TARGET
#if defined(unix) || defined(_unix) || defined(__unix) || defined(__unix__) || \
    defined(__Lynx) || defined(__Lynx__)
#define TARGET TARGET_UNIX
#elif defined(AMIGA) || defined(_AMIGA) || defined(__AMIGA) || \
      defined(__AMIGA__)
#define TARGET TARGET_AMIGA
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32) || \
      defined(__WIN32__)
#define TARGET TARGET_WINDOWS
#elif defined(NETWARE) || defined(_NETWARE) || defined(__NETWARE) || \
      defined(__NETWARE__)
#define TARGET TARGET_NETWARE
#else /* TARGET */
#error unrecognised operating system
#endif /* TARGET */
#endif /* TARGET */


#define SYSTEM_ANY     1 /* no specific system */
#define SYSTEM_DGUX    2 /* DG/UX */
#define SYSTEM_DYNIX   3 /* DYNIX/ptx */
#define SYSTEM_HPUX    4 /* HP/UX */
#define SYSTEM_LINUX   5 /* Linux */
#define SYSTEM_LYNXOS  6 /* LynxOS */
#define SYSTEM_SOLARIS 7 /* Solaris */


#ifndef SYSTEM
#if TARGET == TARGET_UNIX
#if defined(DGUX) || defined(_DGUX) || defined(__DGUX) || defined(__DGUX__)
#define SYSTEM SYSTEM_DGUX
#elif defined(sequent) || defined(_sequent) || defined(__sequent) || \
      defined(__sequent__) || defined(SEQUENT) || defined(_SEQUENT_)
#define SYSTEM SYSTEM_DYNIX
#elif defined(hpux) || defined(_hpux) || defined(__hpux) || defined(__hpux__)
#define SYSTEM SYSTEM_HPUX
#elif defined(linux) || defined(_linux) || defined(__linux) || \
      defined(__linux__)
#define SYSTEM SYSTEM_LINUX
#elif defined(__Lynx) || defined(__Lynx__)
#define SYSTEM SYSTEM_LYNXOS
#elif (defined(sun) || defined(_sun) || defined(__sun) || defined(__sun__)) && \
      (defined(svr4) || defined(_svr4) || defined(__svr4) || \
       defined(__svr4__) || defined(SVR4) || defined(_SVR4) || \
       defined(__SVR4) || defined(__SVR4__))
#define SYSTEM SYSTEM_SOLARIS
#else /* SYSTEM */
#define SYSTEM SYSTEM_ANY
#endif /* SYSTEM */
#else /* TARGET */
#define SYSTEM SYSTEM_ANY
#endif /* TARGET */
#endif /* SYSTEM */


#define ARCH_IX86   1 /* Intel 80x86 */
#define ARCH_M68K   2 /* Motorola 680x0 */
#define ARCH_M88K   3 /* Motorola 88xx0 */
#define ARCH_MIPS   4 /* MIPS */
#define ARCH_PARISC 5 /* HP PA/RISC */
#define ARCH_PPC    6 /* PowerPC */
#define ARCH_SPARC  7 /* SPARC */


#ifndef ARCH
#if defined(i386) || defined(_i386) || defined(__i386) || defined(__i386__) || \
    defined(I386) || defined(_I386) || defined(__I386) || defined(__I386__) || \
    defined(ix86) || defined(_ix86) || defined(__ix86) || defined(__ix86__) || \
    defined(x86) || defined(_x86) || defined(__x86) || defined(__x86__) || \
    defined(_M_IX86)
#define ARCH ARCH_IX86
#elif defined(m68k) || defined(_m68k) || defined(__m68k) || \
      defined(__m68k__) || defined(mc68000) || defined(_mc68000) || \
      defined(__mc68000) || defined(__mc68000__) || defined(M68000) || \
      defined(_M68000) || defined(__M68000) || defined(__M68000__)
#define ARCH ARCH_M68K
#elif defined(m88k) || defined(_m88k) || defined(__m88k) || \
      defined(__m88k__) || defined(m88000) || defined(_m88000) || \
      defined(__m88000) || defined(__m88000__)
#define ARCH ARCH_M88K
#elif defined(mips) || defined(_mips) || defined(__mips) || \
      defined(__mips__) || defined(_M_MRX000)
#define ARCH ARCH_MIPS
#elif defined(hppa) || defined(_hppa) || defined(__hppa) || defined(__hppa__)
#define ARCH ARCH_PARISC
#elif defined(ppc) || defined(_ppc) || defined(__ppc) || defined(__ppc__) || \
      defined(powerpc) || defined(_powerpc) || defined(__powerpc) || \
      defined(__powerpc__) || defined(POWERPC) || defined(_POWERPC) || \
      defined(__POWERPC) || defined(__POWERPC__) || defined(_M_PPC)
#define ARCH ARCH_PPC
#elif defined(sparc) || defined(_sparc) || defined(__sparc) || \
      defined(__sparc__)
#define ARCH ARCH_SPARC
#else /* ARCH */
#error unrecognised processor architecture
#endif /* ARCH */
#endif /* ARCH */


#define FORMAT_NONE  1 /* no symbol support */
#define FORMAT_COFF  2 /* COFF */
#define FORMAT_ELF32 3 /* ELF32 */
#define FORMAT_BFD   4 /* GNU BFD */


#ifndef FORMAT
#if TARGET == TARGET_UNIX
#if SYSTEM == SYSTEM_HPUX || SYSTEM == SYSTEM_LINUX || SYSTEM == SYSTEM_LYNXOS
#define FORMAT FORMAT_BFD
#else /* SYSTEM */
#define FORMAT FORMAT_ELF32
#endif /* SYSTEM */
#else /* FORMAT */
#define FORMAT FORMAT_NONE
#endif /* FORMAT */
#endif /* FORMAT */


#endif /* MP_TARGET_H */
