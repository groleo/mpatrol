#ifndef MP_TARGET_H
#define MP_TARGET_H


/*
 * mpatrol
 * A library for controlling and tracing dynamic memory allocations.
 * Copyright (C) 1997-2001 Graeme S. Roy <graeme@epc.co.uk>
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


/* The target operating system.  This determines how the mpatrol library
 * should interact with the underlying system, and how it should work around
 * deficiencies in some operating systems.
 */

#define TARGET_ANY     0 /* no specific operating system */
#define TARGET_UNIX    1 /* UNIX or UNIX-like */
#define TARGET_AMIGA   2 /* Commodore AmigaOS */
#define TARGET_WINDOWS 3 /* Microsoft Windows 95/98/NT */
#define TARGET_NETWARE 4 /* Novell Netware */


#ifndef TARGET
#if defined(unix) || defined(_unix) || defined(__unix) || defined(__unix__) || \
    defined(AIX) || defined(_AIX) || defined(__AIX) || defined(__AIX__) || \
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


/* The UNIX system variant.  This is only needed on UNIX systems due to the
 * wide variety of vendors and their UNIX systems that support different
 * features.
 */

#define SYSTEM_ANY      0  /* no specific system */
#define SYSTEM_AIX      1  /* AIX */
#define SYSTEM_DGUX     2  /* DG/UX */
#define SYSTEM_DRSNX    3  /* DRS/NX */
#define SYSTEM_DYNIX    4  /* DYNIX/ptx */
#define SYSTEM_FREEBSD  5  /* FreeBSD */
#define SYSTEM_HPUX     6  /* HP/UX */
#define SYSTEM_IRIX     7  /* IRIX */
#define SYSTEM_LINUX    8  /* Linux */
#define SYSTEM_LYNXOS   9  /* LynxOS */
#define SYSTEM_NETBSD   10 /* NetBSD */
#define SYSTEM_OPENBSD  11 /* OpenBSD */
#define SYSTEM_SINIX    12 /* SINIX */
#define SYSTEM_SOLARIS  13 /* Solaris */
#define SYSTEM_SUNOS    14 /* SunOS */
#define SYSTEM_UNIXWARE 15 /* UnixWare */


#ifndef SYSTEM
#if TARGET == TARGET_UNIX
#if defined(AIX) || defined(_AIX) || defined(__AIX) || defined(__AIX__)
#define SYSTEM SYSTEM_AIX
#elif defined(DGUX) || defined(_DGUX) || defined(__DGUX) || defined(__DGUX__)
#define SYSTEM SYSTEM_DGUX
#elif defined(sequent) || defined(_sequent) || defined(__sequent) || \
      defined(__sequent__) || defined(SEQUENT) || defined(_SEQUENT_)
#define SYSTEM SYSTEM_DYNIX
#elif defined(__FreeBSD) || defined(__FreeBSD__)
#define SYSTEM SYSTEM_FREEBSD
#elif defined(hpux) || defined(_hpux) || defined(__hpux) || defined(__hpux__)
#define SYSTEM SYSTEM_HPUX
#elif defined(sgi) || defined(_sgi) || defined(__sgi) || defined(__sgi__)
#define SYSTEM SYSTEM_IRIX
#elif defined(linux) || defined(_linux) || defined(__linux) || \
      defined(__linux__)
#define SYSTEM SYSTEM_LINUX
#elif defined(__Lynx) || defined(__Lynx__)
#define SYSTEM SYSTEM_LYNXOS
#elif defined(__NetBSD) || defined(__NetBSD__)
#define SYSTEM SYSTEM_NETBSD
#elif defined(__OpenBSD) || defined(__OpenBSD__)
#define SYSTEM SYSTEM_OPENBSD
#elif defined(sinix) || defined(_sinix) || defined(__sinix) || \
      defined(__sinix__) || defined(SNI) || defined(_SNI) || defined(__SNI) || \
      defined(__SNI__)
#define SYSTEM SYSTEM_SINIX
#elif defined(sun) || defined(_sun) || defined(__sun) || defined(__sun__)
#if defined(svr4) || defined(_svr4) || defined(__svr4) || defined(__svr4__) || \
    defined(SVR4) || defined(_SVR4) || defined(__SVR4) || defined(__SVR4__)
#define SYSTEM SYSTEM_SOLARIS
#else /* svr4 */
#define SYSTEM SYSTEM_SUNOS
#endif /* svr4 */
#else /* SYSTEM */
#define SYSTEM SYSTEM_ANY
#endif /* SYSTEM */
#else /* TARGET */
#define SYSTEM SYSTEM_ANY
#endif /* TARGET */
#endif /* SYSTEM */


/* The processor architecture.  This defines families of compatible processors
 * from chip manufacturers rather than specific processor models.
 */

#define ARCH_ANY     0 /* no specific architecture */
#define ARCH_IX86    1 /* Intel 80x86 */
#define ARCH_M68K    2 /* Motorola 680x0 */
#define ARCH_M88K    3 /* Motorola 88xx0 */
#define ARCH_MIPS    4 /* MIPS */
#define ARCH_PARISC  5 /* HP PA/RISC */
#define ARCH_POWER   6 /* IBM RS/6000 */
#define ARCH_POWERPC 7 /* PowerPC */
#define ARCH_SPARC   8 /* SPARC */


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
#elif defined(POWER) || defined(_POWER) || defined(__POWER) || \
      defined(__POWER__)
#define ARCH ARCH_POWER
#elif defined(ppc) || defined(_ppc) || defined(__ppc) || defined(__ppc__) || \
      defined(powerpc) || defined(_powerpc) || defined(__powerpc) || \
      defined(__powerpc__) || defined(POWERPC) || defined(_POWERPC) || \
      defined(__POWERPC) || defined(__POWERPC__) || defined(_M_PPC)
#define ARCH ARCH_POWERPC
#elif defined(sparc) || defined(_sparc) || defined(__sparc) || \
      defined(__sparc__)
#define ARCH ARCH_SPARC
#else /* ARCH */
#error unrecognised processor architecture
#endif /* ARCH */
#endif /* ARCH */


/* The processor word size.  This is used to determine the size of pointers
 * and long integers on the target processor.
 */

#define ENVIRON_ANY 0 /* no specific word size */
#define ENVIRON_32  1 /* 32-bit */
#define ENVIRON_64  2 /* 64-bit */


#ifndef ENVIRON
#if SYSTEM == SYSTEM_IRIX
#if defined(ABI64) || defined(_ABI64)
#define ENVIRON ENVIRON_64
#else /* ABI64 */
#define ENVIRON ENVIRON_32
#endif /* ABI64 */
#elif SYSTEM == SYSTEM_SOLARIS
#if defined(sparcv9) || defined(_sparcv9) || defined(__sparcv9) || \
    defined(__sparcv9__)
#define ENVIRON ENVIRON_64
#else /* sparcv9 */
#define ENVIRON ENVIRON_32
#endif /* sparcv9 */
#else /* SYSTEM */
#define ENVIRON ENVIRON_32
#endif /* SYSTEM */
#endif /* ENVIRON */


/* The object file format.  This can either specify an explicit file format
 * for executable files and shared libraries, or specify an object file access
 * library.
 */

#define FORMAT_NONE  0 /* no symbol support */
#define FORMAT_AOUT  1 /* a.out */
#define FORMAT_COFF  2 /* COFF */
#define FORMAT_XCOFF 3 /* XCOFF */
#define FORMAT_ELF32 4 /* ELF32 */
#define FORMAT_ELF64 5 /* ELF64 */
#define FORMAT_BFD   6 /* GNU BFD */
#define FORMAT_PE    7 /* Portable Executable */


#ifndef FORMAT
#if TARGET == TARGET_UNIX
#if SYSTEM == SYSTEM_AIX
#define FORMAT FORMAT_XCOFF
#elif SYSTEM == SYSTEM_DGUX || SYSTEM == SYSTEM_DRSNX || \
      SYSTEM == SYSTEM_DYNIX || SYSTEM == SYSTEM_IRIX || \
      SYSTEM == SYSTEM_SINIX || SYSTEM == SYSTEM_SOLARIS || \
      SYSTEM == SYSTEM_UNIXWARE
#if ENVIRON == ENVIRON_64
#define FORMAT FORMAT_ELF64
#else /* ENVIRON */
#define FORMAT FORMAT_ELF32
#endif /* ENVIRON */
#elif SYSTEM == SYSTEM_FREEBSD || SYSTEM == SYSTEM_NETBSD || \
      SYSTEM == SYSTEM_OPENBSD || SYSTEM == SYSTEM_SUNOS
#ifdef __ELF__
#if ENVIRON == ENVIRON_64
#define FORMAT FORMAT_ELF64
#else /* ENVIRON */
#define FORMAT FORMAT_ELF32
#endif /* ENVIRON */
#else /* __ELF__ */
#define FORMAT FORMAT_AOUT
#endif /* __ELF__ */
#elif SYSTEM == SYSTEM_HPUX || SYSTEM == SYSTEM_LINUX || SYSTEM == SYSTEM_LYNXOS
#define FORMAT FORMAT_BFD
#else /* SYSTEM */
#define FORMAT FORMAT_NONE
#endif /* SYSTEM */
#elif TARGET == TARGET_WINDOWS
#define FORMAT FORMAT_PE
#else /* TARGET */
#if TARGET == TARGET_AMIGA && defined(__GNUC__)
#define FORMAT FORMAT_BFD
#else /* TARGET && __GNUC__ */
#define FORMAT FORMAT_NONE
#endif /* TARGET && __GNUC__ */
#endif /* TARGET */
#endif /* FORMAT */


/* The dynamic linker type.  This is used to specify the method used to obtain
 * information about the shared libraries that a program requires when it is
 * running.
 */

#define DYNLINK_NONE    0 /* no dynamic linker support */
#define DYNLINK_AIX     1 /* AIX dynamic linker */
#define DYNLINK_BSD     2 /* BSD dynamic linker */
#define DYNLINK_HPUX    3 /* HP/UX dynamic linker */
#define DYNLINK_IRIX    4 /* IRIX dynamic linker */
#define DYNLINK_SVR4    5 /* SVR4 dynamic linker */
#define DYNLINK_WINDOWS 6 /* Windows dynamic linker */


#ifndef DYNLINK
#if TARGET == TARGET_UNIX
#if SYSTEM == SYSTEM_AIX
#define DYNLINK DYNLINK_AIX
#elif SYSTEM == SYSTEM_DGUX || SYSTEM == SYSTEM_DRSNX || \
      SYSTEM == SYSTEM_DYNIX || SYSTEM == SYSTEM_LINUX || \
      SYSTEM == SYSTEM_SINIX || SYSTEM == SYSTEM_SOLARIS || \
      SYSTEM == SYSTEM_UNIXWARE
#define DYNLINK DYNLINK_SVR4
#elif SYSTEM == SYSTEM_FREEBSD || SYSTEM == SYSTEM_NETBSD || \
      SYSTEM == SYSTEM_OPENBSD || SYSTEM == SYSTEM_SUNOS
#ifdef __ELF__
#define DYNLINK DYNLINK_SVR4
#else /* __ELF__ */
#define DYNLINK DYNLINK_BSD
#endif /* __ELF__ */
#elif SYSTEM == SYSTEM_HPUX
#define DYNLINK DYNLINK_HPUX
#elif SYSTEM == SYSTEM_IRIX
#define DYNLINK DYNLINK_IRIX
#else /* SYSTEM */
#define DYNLINK DYNLINK_NONE
#endif /* SYSTEM */
#elif TARGET == TARGET_WINDOWS
#define DYNLINK DYNLINK_WINDOWS
#else /* TARGET */
#define DYNLINK DYNLINK_NONE
#endif /* TARGET */
#endif /* DYNLINK */


/* Target feature macros.  Some systems require certain preprocessor macros
 * to be defined before non-standard definitions in system header files are
 * included.
 */

#if TARGET == TARGET_UNIX
#define _POSIX_SOURCE 1
#define _POSIX_C_SOURCE 199506L
#if SYSTEM == SYSTEM_AIX
#define _ALL_SOURCE 1
#elif SYSTEM == SYSTEM_DGUX
#define _DGUX_SOURCE 1
#elif SYSTEM == SYSTEM_HPUX
#define _HPUX_SOURCE 1
#elif SYSTEM == SYSTEM_IRIX
#define _SGI_SOURCE 1
#elif SYSTEM == SYSTEM_LINUX
#define _GNU_SOURCE 1
#elif SYSTEM == SYSTEM_SOLARIS
#define __EXTENSIONS__ 1
#endif /* SYSTEM */
#endif /* TARGET */


#endif /* MP_TARGET_H */
