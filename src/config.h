#ifndef MP_CONFIG_H
#define MP_CONFIG_H


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
 * Library configuration.  All configuration constants for the mpatrol
 * library are defined here.
 */


#include "target.h"


/* The keywords used to specify the visibility of all internal support
 * functions and variables within the library.  If the library is composed
 * of separate object modules then functions in one translation unit may
 * need to call functions in another translation unit even though they
 * should not be visible to the user.
 */

#ifndef MP_EXPORT
#define MP_EXPORT extern
#endif /* MP_EXPORT */

#ifndef MP_GLOBAL
#define MP_GLOBAL
#endif /* MP_GLOBAL */


/* Indicates if all of the heap memory used by the library should be
 * deleted when the process exits.  This should not be set on systems that
 * make dynamic memory allocations after exit() or reference freed memory
 * at process termination, and really only needs to be set on systems that
 * do not reclaim memory from a process when it terminates.
 */

#ifndef MP_DELETE
#if TARGET == TARGET_UNIX
#define MP_DELETE 0
#elif TARGET == TARGET_AMIGA || TARGET == TARGET_WINDOWS || \
      TARGET == TARGET_NETWARE
#define MP_DELETE 1
#endif /* TARGET */
#endif /* MP_DELETE */


/* Indicates if the system supports memory protection.  If not, then the
 * NOPROTECT option will always be used to prevent needless function calls
 * and the PAGEALLOC option will have no effect.
 */

#ifndef MP_PROTECT_SUPPORT
#if TARGET == TARGET_UNIX || TARGET == TARGET_WINDOWS
#define MP_PROTECT_SUPPORT 1
#elif TARGET == TARGET_AMIGA || TARGET == TARGET_NETWARE
#define MP_PROTECT_SUPPORT 0
#endif /* TARGET */
#endif /* MP_PROTECT_SUPPORT */


/* Indicates if the system supports watch areas.  If not, then the OFLOWWATCH
 * option will have no effect.
 */

#ifndef MP_WATCH_SUPPORT
#if SYSTEM == SYSTEM_SOLARIS
#define MP_WATCH_SUPPORT 1
#else /* SYSTEM */
#define MP_WATCH_SUPPORT 0
#endif /* SYSTEM */
#endif /* MP_WATCH_SUPPORT */


/* Indicates if the library is being built with thread-safe support.  This is
 * normally set in the makefile.
 */

#ifndef MP_THREADS_SUPPORT
#define MP_THREADS_SUPPORT 0
#endif /* MP_THREADS_SUPPORT */


/* The system-specific preprocessor macro that needs to be defined in order to
 * compile thread-safe code.
 */

#if MP_THREADS_SUPPORT
#if TARGET == TARGET_UNIX
#if SYSTEM == SYSTEM_DGUX
#ifndef _POSIX4A_DRAFT6_SOURCE
#define _POSIX4A_DRAFT6_SOURCE 1
#endif /* _POSIX4A_DRAFT6_SOURCE */
#elif SYSTEM == SYSTEM_DYNIX
#ifndef _SEQUENT_THREADS
#define _SEQUENT_THREADS 1
#endif /* _SEQUENT_THREADS */
#elif SYSTEM == SYSTEM_HPUX || SYSTEM == SYSTEM_LINUX || \
      SYSTEM == SYSTEM_SOLARIS
#ifndef _REENTRANT
#define _REENTRANT 1
#endif /* _REENTRANT */
#elif SYSTEM == SYSTEM_LYNXOS
#ifndef _MULTITHREADED
#define _MULTITHREADED 1
#endif /* _MULTITHREADED */
#ifndef _POSIX_THREADS_CALLS
#define _POSIX_THREADS_CALLS 1
#endif /* _POSIX_THREADS_CALLS */
#endif /* SYSTEM */
#elif TARGET == TARGET_WINDOWS
#ifndef _MT
#define _MT 1
#endif /* _MT */
#endif /* TARGET */
#endif /* MP_THREADS_SUPPORT */


/* Indicates if the system supports obtaining more information from within
 * signal handlers.  If not, then the illegal memory access signal handler will
 * not be able to determine where the faulty address was.
 */

#ifndef MP_SIGINFO_SUPPORT
#if SYSTEM == SYSTEM_DGUX || SYSTEM == SYSTEM_DYNIX || \
    SYSTEM == SYSTEM_HPUX || SYSTEM == SYSTEM_SOLARIS
#define MP_SIGINFO_SUPPORT 1
#else /* SYSTEM */
#define MP_SIGINFO_SUPPORT 0
#endif /* SYSTEM */
#endif /* MP_SIGINFO_SUPPORT */


/* Indicates if the system supports the /proc filesystem.  If this is the case
 * then it will be possible to obtain more information about the running process
 * as well as controlling certain aspects of it.
 */

#ifndef MP_PROCFS_SUPPORT
#if SYSTEM == SYSTEM_LINUX || SYSTEM == SYSTEM_SOLARIS
#define MP_PROCFS_SUPPORT 1
#else /* SYSTEM */
#define MP_PROCFS_SUPPORT 0
#endif /* SYSTEM */
#endif /* MP_PROCFS_SUPPORT */


/* The full path to the directory representing the mount point of the /proc
 * filesystem.  This isn't fixed on a particular system, but we have to assume
 * that it's somewhere by default.
 */

#if MP_PROCFS_SUPPORT
#ifndef MP_PROCFS_DIRNAME
#define MP_PROCFS_DIRNAME "/proc"
#endif /* MP_PROCFS_DIRNAME */
#endif /* MP_PROCFS_SUPPORT */


/* The filename of the executable file within /proc that allows a process to
 * examine the command it was invoked with.
 */

#if MP_PROCFS_SUPPORT
#ifndef MP_PROCFS_EXENAME
#if SYSTEM == SYSTEM_SOLARIS
#define MP_PROCFS_EXENAME "object/a.out"
#else /* SYSTEM */
#define MP_PROCFS_EXENAME "exe"
#endif /* SYSTEM */
#endif /* MP_PROCFS_EXENAME */
#endif /* MP_PROCFS_SUPPORT */


/* The filename of the control file within /proc that allows a process to
 * manipulate itself by writing commands to it.
 */

#if MP_PROCFS_SUPPORT && MP_WATCH_SUPPORT
#ifndef MP_PROCFS_CTLNAME
#define MP_PROCFS_CTLNAME "ctl"
#endif /* MP_PROCFS_CTLNAME */
#endif /* MP_PROCFS_SUPPORT && MP_WATCH_SUPPORT */


/* Indicates if the compiler supports the __builtin_frame_address() and
 * __builtin_return_address() macros, and if they should be used instead of
 * traversing the call stack directly.  Note that this method only allows a
 * finite number of call stack traversals per function.
 */

#ifndef MP_BUILTINSTACK_SUPPORT
#define MP_BUILTINSTACK_SUPPORT 0
#endif /* MP_BUILTINSTACK_SUPPORT */


/* The maximum number of call stack traversals per function if builtin
 * frame address and return address support is being used.  This number must
 * be supported by the required number of macro functions in stack.c.
 */

#if MP_BUILTINSTACK_SUPPORT
#define MP_MAXSTACK 8
#endif /* MP_BUILTINSTACK_SUPPORT */


/* Indicates if the compiler supports the long long type.  This is only used
 * to determine the minimum alignment required for a generic memory allocation.
 */

#ifndef MP_LONGLONG_SUPPORT
#ifdef __GNUC__
#define MP_LONGLONG_SUPPORT 1
#else /* __GNUC__ */
#define MP_LONGLONG_SUPPORT 0
#endif /* __GNUC__ */
#endif /* MP_LONGLONG_SUPPORT */


/* Indicates if the compiler supports the ident preprocessor directive for
 * placing a version string in the comment section of an object file.
 */

#ifndef MP_IDENT_SUPPORT
#if SYSTEM == SYSTEM_DGUX || SYSTEM == SYSTEM_DYNIX || \
    SYSTEM == SYSTEM_LINUX || SYSTEM == SYSTEM_SOLARIS
#define MP_IDENT_SUPPORT 1
#else /* SYSTEM */
#define MP_IDENT_SUPPORT 0
#endif /* SYSTEM */
#endif /* MP_IDENT_SUPPORT */


/* The format string passed to fprintf() which is used to display addresses.
 */

#ifndef MP_POINTER
#define MP_POINTER "0x%08lX"
#endif /* MP_POINTER */


/* The name of the environment variable which contains library options.
 */

#ifndef MP_OPTIONS
#define MP_OPTIONS "MPATROL_OPTIONS"
#endif /* MP_OPTIONS */


/* The name of the log file used to send library diagnostics to.  This may
 * be overridden at run-time using the LOGFILE option and may contain
 * special formatting characters.
 */

#ifndef MP_LOGFILE
#define MP_LOGFILE "mpatrol.log"
#endif /* MP_LOGFILE */


/* The overflow buffer size in bytes.  This may be overridden at run-time
 * using the OFLOWSIZE option.  The default is zero since this setting
 * may dramatically increase execution time if it is non-zero.
 */

#ifndef MP_OVERFLOW
#define MP_OVERFLOW 0
#endif /* MP_OVERFLOW */


/* The overflow byte value.  This may be overridden at run-time using the
 * OFLOWBYTE option.  The default is the bit pattern 0b10101010.
 */

#ifndef MP_OVERBYTE
#define MP_OVERBYTE 0xAA
#endif /* MP_OVERBYTE */


/* The allocation byte value.  This may be overridden at run-time using the
 * ALLOCBYTE option.  The default is the bit pattern 0b11111111.
 */

#ifndef MP_ALLOCBYTE
#define MP_ALLOCBYTE 0xFF
#endif /* MP_ALLOCBYTE */


/* The free byte value.  This may be overridden at run-time using the
 * FREEBYTE option.  The default is the bit pattern 0b01010101.
 */

#ifndef MP_FREEBYTE
#define MP_FREEBYTE 0x55
#endif /* MP_FREEBYTE */


#endif /* MP_CONFIG_H */
