#ifndef MP_CONFIG_H
#define MP_CONFIG_H


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


/* The keyword used to specify a constant variable or parameter.  This may
 * only be supported by ANSI C or C++ compilers so it is defined under a
 * macro just in case.  Note that constness is really only used for the
 * highest-level functions in the library to prevent clashes with any
 * functions that are being overridden.
 */

#ifndef MP_CONST
#ifdef __STDC__
#define MP_CONST const
#else /* __STDC__ */
#define MP_CONST
#endif /* __STDC__ */
#endif /* MP_CONST */


/* Indicates if preprocessor macro versions of some internal library routines
 * should be used instead of their function equivalents in order to increase
 * run-time efficiency.  This might not be desirable if the library needs to be
 * run under a debugger.
 */

#ifndef MP_MACROROUTINES
#define MP_MACROROUTINES 1
#endif /* MP_MACROROUTINES */


/* Indicates if system memory should be allocated from a static array rather
 * than the process heap.  Use this to provide support for dynamic memory
 * allocation routines on systems that don't have a system function to allocate
 * heap memory.
 */

#ifndef MP_ARRAY_SUPPORT
#define MP_ARRAY_SUPPORT 0
#endif /* MP_ARRAY_SUPPORT */


/* The size of the static memory array in bytes.  Any attempt to allocate more
 * system memory than this will fail, although it should be remembered that the
 * library will also be using this array for its internal structures.  The
 * default is 1 megabyte.
 */

#if MP_ARRAY_SUPPORT
#ifndef MP_ARRAY_SIZE
#define MP_ARRAY_SIZE 1048576
#endif /* MP_ARRAY_SIZE */
#endif /* MP_ARRAY_SUPPORT */


/* The size of the simulated UNIX heap in bytes.  This is used by the brk() and
 * sbrk() functions on non-UNIX platforms and is used to allocate a block of
 * memory of this size.  Any attempt to allocate memory beyond this block will
 * cause these functions to fail.
 */

#ifndef MP_BREAK_SIZE
#define MP_BREAK_SIZE 262144
#endif /* MP_BREAK_SIZE */


/* The size of the input line buffer in the mleak tool.  If any of the lines
 * in the log file are longer than this then an error message will be generated
 * and mleak will terminate.
 */

#ifndef MP_BUFFER_SIZE
#define MP_BUFFER_SIZE 1024
#endif /* MP_BUFFER_SIZE */


/* The number of allocation bins to use when profiling.  Details of memory
 * allocations of all sizes up to the bin size will be recorded in a table and
 * written to the profiling output file at program termination.
 */

#ifndef MP_BIN_SIZE
#define MP_BIN_SIZE 1024
#endif /* MP_BIN_SIZE */


/* The number of buckets in the hash table used to implement the string table.
 * This must be a prime number.
 */

#ifndef MP_HASHTAB_SIZE
#define MP_HASHTAB_SIZE 211
#endif /* MP_HASHTAB_SIZE */


/* The multiple of pages to allocate from the heap every time a new block of
 * internal memory is required.  The higher the value, the less distinct
 * internal blocks to keep track of, but the potential for more memory wastage
 * if not all of the block is required.
 */

#ifndef MP_ALLOCFACTOR
#define MP_ALLOCFACTOR 2
#endif /* MP_ALLOCFACTOR */


/* The maximum number of recursive calls to C++ operator delete and operator
 * delete[] that will have source level information associated with them.
 * This acts as a workaround for the fact that placement delete will only be
 * called during an exception and not if explicitly invoked.  However, the
 * current implementation is not thread-safe.
 */

#ifndef MP_MAXDELSTACK
#define MP_MAXDELSTACK 32
#endif /* MP_MAXDELSTACK */


/* Indicates if all of the heap memory used by the library should be
 * deleted when the process exits.  This should not be set on systems that
 * make dynamic memory allocations after exit() or reference freed memory
 * at process termination, and really only needs to be set on systems that
 * do not reclaim memory from a process when it terminates.
 */

#ifndef MP_DELETE
#if TARGET == TARGET_UNIX || TARGET == TARGET_WINDOWS
#define MP_DELETE 0
#elif TARGET == TARGET_AMIGA || TARGET == TARGET_NETWARE
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


/* Indicates if a UNIX system supports the mmap() function call to allocate
 * memory as well as sbrk().  This must only be set if the system also supports
 * the allocation of zero-initialised pages from either a special device file
 * or via a special flag.  Note that sbrk() will still be used by default, but
 * the USEMMAP option will instruct the library to use mmap() instead.
 */

#ifndef MP_MMAP_SUPPORT
#if SYSTEM == SYSTEM_AIX || SYSTEM == SYSTEM_DGUX || SYSTEM == SYSTEM_DRSNX || \
    SYSTEM == SYSTEM_DYNIX || SYSTEM == SYSTEM_HPUX || \
    SYSTEM == SYSTEM_IRIX || SYSTEM == SYSTEM_LINUX || \
    SYSTEM == SYSTEM_SINIX || SYSTEM == SYSTEM_SOLARIS || \
    SYSTEM == SYSTEM_UNIXWARE
#define MP_MMAP_SUPPORT 1
#else /* SYSTEM */
#define MP_MMAP_SUPPORT 0
#endif /* SYSTEM */
#endif /* MP_MMAP_SUPPORT */


/* Indicates if the mmap() function call supports the MAP_ANON or MAP_ANONYMOUS
 * flag for allocating zero-filled pages.  If it doesn't then the filename
 * specified by MP_MMAP_FILENAME will be used instead.
 */

#if MP_MMAP_SUPPORT
#ifndef MP_MMAP_ANONYMOUS
#if SYSTEM == SYSTEM_AIX || SYSTEM == SYSTEM_DYNIX || SYSTEM == SYSTEM_HPUX || \
    SYSTEM == SYSTEM_LINUX || SYSTEM == SYSTEM_UNIXWARE
#define MP_MMAP_ANONYMOUS 1
#else /* SYSTEM */
#define MP_MMAP_ANONYMOUS 0
#endif /* SYSTEM */
#endif /* MP_MMAP_ANONYMOUS */
#endif /* MP_MMAP_SUPPORT */


/* The full path to a special device file which contains an infinite number of
 * zero bytes.  This is used with mmap() in order to allocate zero-filled pages.
 */

#if MP_MMAP_SUPPORT
#ifndef MP_MMAP_FILENAME
#define MP_MMAP_FILENAME "/dev/zero"
#endif /* MP_MMAP_FILENAME */
#endif /* MP_MMAP_SUPPORT */


/* Indicates if the system supports watch areas.  If not, then the OFLOWWATCH
 * option will have no effect.
 */

#ifndef MP_WATCH_SUPPORT
#if SYSTEM == SYSTEM_IRIX || SYSTEM == SYSTEM_SOLARIS
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
#if SYSTEM == SYSTEM_AIX
#ifndef _REENTRANT
#define _REENTRANT 1
#endif /* _REENTRANT */
#ifndef _THREAD_SAFE
#define _THREAD_SAFE 1
#endif /* _THREAD_SAFE */
#elif SYSTEM == SYSTEM_DGUX
#ifndef _POSIX4A_DRAFT6_SOURCE
#define _POSIX4A_DRAFT6_SOURCE 1
#endif /* _POSIX4A_DRAFT6_SOURCE */
#elif SYSTEM == SYSTEM_DRSNX || SYSTEM == SYSTEM_HPUX || \
      SYSTEM == SYSTEM_LINUX || SYSTEM == SYSTEM_SINIX || \
      SYSTEM == SYSTEM_SOLARIS || SYSTEM == SYSTEM_UNIXWARE
#ifndef _REENTRANT
#define _REENTRANT 1
#endif /* _REENTRANT */
#elif SYSTEM == SYSTEM_DYNIX
#ifndef _SEQUENT_THREADS
#define _SEQUENT_THREADS 1
#endif /* _SEQUENT_THREADS */
#elif SYSTEM == SYSTEM_IRIX
#ifndef _SGI_REENTRANT_FUNCTIONS
#define _SGI_REENTRANT_FUNCTIONS 1
#endif /* _SGI_REENTRANT_FUNCTIONS */
#elif SYSTEM == SYSTEM_LYNXOS
#ifndef _MULTITHREADED
#define _MULTITHREADED 1
#endif /* _MULTITHREADED */
#ifndef __POSIX4_D9__
#define __POSIX4_D9__ 1
#endif /* __POSIX4_D9__ */
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
#if SYSTEM == SYSTEM_DGUX || SYSTEM == SYSTEM_DRSNX || \
    SYSTEM == SYSTEM_DYNIX || SYSTEM == SYSTEM_HPUX || \
    SYSTEM == SYSTEM_SINIX || SYSTEM == SYSTEM_SOLARIS || \
    SYSTEM == SYSTEM_UNIXWARE
#define MP_SIGINFO_SUPPORT 1
#else /* SYSTEM */
#define MP_SIGINFO_SUPPORT 0
#endif /* SYSTEM */
#endif /* MP_SIGINFO_SUPPORT */


/* Indicates if the system supports the /proc filesystem.  If this is the case
 * then it may be possible to obtain more information about the running process
 * as well as controlling certain aspects of it.
 */

#ifndef MP_PROCFS_SUPPORT
#if SYSTEM == SYSTEM_DRSNX || SYSTEM == SYSTEM_IRIX || \
    SYSTEM == SYSTEM_LINUX || SYSTEM == SYSTEM_SOLARIS || \
    SYSTEM == SYSTEM_UNIXWARE
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


/* The filename of the command line file within /proc that allows a process to
 * examine the arguments it was invoked with.
 */

#if MP_PROCFS_SUPPORT
#ifndef MP_PROCFS_CMDNAME
#if SYSTEM == SYSTEM_LINUX
#define MP_PROCFS_CMDNAME MP_PROCFS_DIRNAME "/%lu/cmdline"
#endif /* SYSTEM */
#endif /* MP_PROCFS_CMDNAME */
#endif /* MP_PROCFS_SUPPORT */


/* The filename of the executable file within /proc that allows a process to
 * examine the command it was invoked with.
 */

#if MP_PROCFS_SUPPORT
#ifndef MP_PROCFS_EXENAME
#if SYSTEM == SYSTEM_LINUX
#define MP_PROCFS_EXENAME MP_PROCFS_DIRNAME "/%lu/exe"
#elif SYSTEM == SYSTEM_SOLARIS || SYSTEM == SYSTEM_UNIXWARE
#define MP_PROCFS_EXENAME MP_PROCFS_DIRNAME "/%lu/object/a.out"
#endif /* SYSTEM */
#endif /* MP_PROCFS_EXENAME */
#endif /* MP_PROCFS_SUPPORT */


/* The filename of the control file within /proc that allows a process to
 * manipulate itself by writing commands to it.
 */

#if MP_PROCFS_SUPPORT && MP_WATCH_SUPPORT
#ifndef MP_PROCFS_CTLNAME
#if SYSTEM == SYSTEM_IRIX
#define MP_PROCFS_CTLNAME MP_PROCFS_DIRNAME "/%05lu"
#elif SYSTEM == SYSTEM_SOLARIS
#define MP_PROCFS_CTLNAME MP_PROCFS_DIRNAME "/%lu/ctl"
#endif /* SYSTEM */
#endif /* MP_PROCFS_CTLNAME */
#endif /* MP_PROCFS_SUPPORT && MP_WATCH_SUPPORT */


/* Indicates if the compiler supports the __builtin_frame_address() and
 * __builtin_return_address() macros, and if they should be used instead of
 * traversing the call stack directly.  Note that this method only allows a
 * finite number of call stack traversals per function.
 */

#ifndef MP_BUILTINSTACK_SUPPORT
#if TARGET == TARGET_AMIGA && defined(__GNUC__)
#define MP_BUILTINSTACK_SUPPORT 1
#else /* TARGET && __GNUC__ */
#define MP_BUILTINSTACK_SUPPORT 0
#endif /* TARGET && __GNUC__ */
#endif /* MP_BUILTINSTACK_SUPPORT */


/* The maximum number of call stack traversals per function if builtin
 * frame address and return address support is being used.  This number must
 * be supported by the required number of macro functions in stack.c.
 */

#if MP_BUILTINSTACK_SUPPORT
#ifndef MP_MAXSTACK
#if TARGET == TARGET_AMIGA && defined(__GNUC__)
#define MP_MAXSTACK 3
#else /* TARGET && __GNUC__ */
#define MP_MAXSTACK 8
#endif /* TARGET && __GNUC__ */
#endif /* MP_MAXSTACK */
#endif /* MP_BUILTINSTACK_SUPPORT */


/* Indicates if the operating system provides support routines for traversing
 * call stacks in an external library.  This is currently only available for
 * HP/UX, IRIX and Windows, but the IRIX unwind() library routine calls malloc()
 * and free() so the non-library method of call stack traversal is used instead
 * as it is much faster.  Note that MP_BUILTINSTACK_SUPPORT takes precedence.
 */

#ifndef MP_LIBRARYSTACK_SUPPORT
#if !MP_BUILTINSTACK_SUPPORT
#if (TARGET == TARGET_UNIX && SYSTEM == SYSTEM_HPUX) || TARGET == TARGET_WINDOWS
#define MP_LIBRARYSTACK_SUPPORT 1
#else /* TARGET && SYSTEM */
#define MP_LIBRARYSTACK_SUPPORT 0
#endif /* TARGET && SYSTEM */
#else /* MP_BUILTINSTACK_SUPPORT */
#define MP_LIBRARYSTACK_SUPPORT 0
#endif /* MP_BUILTINSTACK_SUPPORT */
#endif /* MP_LIBRARYSTACK_SUPPORT */


/* Indicates if support for full stack tracebacks is available.  This
 * information can then be used to decide if we should compare two call stacks
 * for the purpose of determining when memory allocations made by the alloca()
 * family of functions go out of scope.  Otherwise we have to use a less
 * accurate system which takes the addresses of local variables.
 */

#ifndef MP_FULLSTACK
#if MP_BUILTINSTACK_SUPPORT
#define MP_FULLSTACK 0
#elif MP_LIBRARYSTACK_SUPPORT || (TARGET == TARGET_UNIX && \
       (ARCH == ARCH_IX86 || ARCH == ARCH_M68K || ARCH == ARCH_M88K || \
        ARCH == ARCH_MIPS || ARCH == ARCH_POWER || ARCH == ARCH_POWERPC || \
        ARCH == ARCH_SPARC)) || ((TARGET == TARGET_WINDOWS || \
        TARGET == TARGET_NETWARE) && ARCH == ARCH_IX86)
#define MP_FULLSTACK 1
#else /* MP_BUILTINSTACK_SUPPORT && MP_LIBRARYSTACK_SUPPORT && ... */
#define MP_FULLSTACK 0
#endif /* MP_BUILTINSTACK_SUPPORT && MP_LIBRARYSTACK_SUPPORT && ... */
#endif /* MP_FULLSTACK */


/* Indicates if the system dynamic linker supports preloading a set of shared
 * libraries specified in an environment variable.
 */

#ifndef MP_PRELOAD_SUPPORT
#if SYSTEM == SYSTEM_DGUX || SYSTEM == SYSTEM_DYNIX || \
    SYSTEM == SYSTEM_IRIX || SYSTEM == SYSTEM_LINUX || SYSTEM == SYSTEM_SOLARIS
#define MP_PRELOAD_SUPPORT 1
#else /* SYSTEM */
#define MP_PRELOAD_SUPPORT 0
#endif /* SYSTEM */
#endif /* MP_PRELOAD_SUPPORT */


/* The name of the environment variable which is used to specify a list of
 * shared libraries to preload, and the string which is used to delimit the
 * name of every library specified in that environment variable.
 */

#if MP_PRELOAD_SUPPORT
#ifndef MP_PRELOAD_NAME
#if SYSTEM == SYSTEM_IRIX
#define MP_PRELOAD_NAME "_RLD_LIST"
#else /* SYSTEM */
#define MP_PRELOAD_NAME "LD_PRELOAD"
#endif /* SYSTEM */
#endif /* MP_PRELOAD_NAME */

#ifndef MP_PRELOAD_SEP
#if SYSTEM == SYSTEM_IRIX
#define MP_PRELOAD_SEP ":"
#else /* SYSTEM */
#define MP_PRELOAD_SEP " "
#endif /* SYSTEM */
#endif /* MP_PRELOAD_SEP */
#endif /* MP_PRELOAD_SUPPORT */


/* The macro function which will be used to calculate the filenames for each
 * of the shared libraries that must be preloaded.
 */

#if MP_PRELOAD_SUPPORT
#ifndef MP_LIBNAME
#if SYSTEM == SYSTEM_HPUX
#define MP_LIBNAME(l) "lib" #l ".sl"
#else /* SYSTEM */
#define MP_LIBNAME(l) "lib" #l ".so"
#endif /* SYSTEM */
#endif /* MP_LIBNAME */
#endif /* MP_PRELOAD_SUPPORT */


/* The filenames of all the different categories of libraries that must be
 * included in the lists of shared libraries to preload.
 */

#if MP_PRELOAD_SUPPORT
#ifndef MP_MPATROL_LIBS
#define MP_MPATROL_LIBS MP_LIBNAME(mpatrol)
#endif /* MP_MPATROL_LIBS */

#ifndef MP_MPATROLMT_LIBS
#define MP_MPATROLMT_LIBS MP_LIBNAME(mpatrolmt)
#endif /* MP_MPATROLMT_LIBS */

#ifndef MP_SYMBOL_LIBS
#if FORMAT == FORMAT_ELF32 || FORMAT == FORMAT_ELF64
#define MP_SYMBOL_LIBS , MP_LIBNAME(elf)
#elif FORMAT == FORMAT_BFD
#define MP_SYMBOL_LIBS , MP_LIBNAME(bfd), MP_LIBNAME(iberty)
#else /* FORMAT */
#define MP_SYMBOL_LIBS
#endif /* FORMAT */
#endif /* MP_SYMBOL_LIBS */

#ifndef MP_THREADS_LIBS
#if SYSTEM == SYSTEM_DGUX
#define MP_THREADS_LIBS , MP_LIBNAME(thread)
#elif SYSTEM == SYSTEM_LINUX || SYSTEM == SYSTEM_SOLARIS || \
      SYSTEM == SYSTEM_UNIXWARE
#define MP_THREADS_LIBS , MP_LIBNAME(pthread)
#else /* SYSTEM */
#define MP_THREADS_LIBS
#endif /* SYSTEM */
#endif /* MP_THREADS_LIBS */

#ifndef MP_SYSTEM_LIBS
#if SYSTEM == SYSTEM_HPUX
#if MP_LIBRARYSTACK_SUPPORT
#define MP_SYSTEM_LIBS , MP_LIBNAME(cl)
#else /* MP_LIBRARYSTACK_SUPPORT */
#define MP_SYSTEM_LIBS
#endif /* MP_LIBRARYSTACK_SUPPORT */
#elif SYSTEM == SYSTEM_IRIX
#if MP_LIBRARYSTACK_SUPPORT
#define MP_SYSTEM_LIBS , MP_LIBNAME(exc), "DEFAULT"
#else /* MP_LIBRARYSTACK_SUPPORT */
#define MP_SYSTEM_LIBS , "DEFAULT"
#endif /* MP_LIBRARYSTACK_SUPPORT */
#else /* SYSTEM */
#define MP_SYSTEM_LIBS
#endif /* SYSTEM */
#endif /* MP_SYSTEM_LIBS */
#endif /* MP_PRELOAD_SUPPORT */


/* The final lists of normal and thread-safe shared libraries to preload.
 * These must all exist in the shared library path used by the dynamic linker,
 * otherwise full paths to the libraries must be explicitly given.
 */

#if MP_PRELOAD_SUPPORT
#ifndef MP_PRELOAD_LIBS
#define MP_PRELOAD_LIBS MP_MPATROL_LIBS MP_SYMBOL_LIBS MP_SYSTEM_LIBS
#endif /* MP_PRELOAD_LIBS */

#ifndef MP_PRELOADMT_LIBS
#define MP_PRELOADMT_LIBS MP_MPATROLMT_LIBS MP_SYMBOL_LIBS MP_THREADS_LIBS \
                          MP_SYSTEM_LIBS
#endif /* MP_PRELOADMT_LIBS */
#endif /* MP_PRELOAD_SUPPORT */


/* Indicates if support for Parasoft Inuse is enabled.  This is a commercial
 * product which graphically displays process memory usage and is provided as
 * an add-on for Parasoft Insure++.
 */

#ifndef MP_INUSE_SUPPORT
#define MP_INUSE_SUPPORT 0
#endif /* MP_INUSE_SUPPORT */


/* Indicates if the C functions defined in malloc.c are to have duplicate
 * functions defined with an alternative name.
 */

#ifndef MP_ALTFUNCNAMES
#if TARGET == TARGET_UNIX
#define MP_ALTFUNCNAMES 1
#else /* TARGET */
#define MP_ALTFUNCNAMES 0
#endif /* TARGET */
#endif /* MP_ALTFUNCNAMES */


/* The macro function which will be used to calculate the alternative names
 * for all C functions that are defined in malloc.c.
 */

#if MP_ALTFUNCNAMES
#ifndef MP_ALTFUNCNAME
#define MP_ALTFUNCNAME(f) _ ## f
#endif /* MP_ALTFUNCNAME */
#endif /* MP_ALTFUNCNAMES */


/* Indicates if the system supports an .init section for placing calls to
 * routines that will be called before main().  This is only likely to be
 * true for ELF systems and will only be required for initialising the
 * mpatrol mutexes and data structures for thread-safe code.
 */

#ifndef MP_INIT_SUPPORT
#if SYSTEM == SYSTEM_DGUX || SYSTEM == SYSTEM_DRSNX || \
    SYSTEM == SYSTEM_DYNIX || SYSTEM == SYSTEM_LINUX || \
    SYSTEM == SYSTEM_SOLARIS || SYSTEM == SYSTEM_UNIXWARE
#define MP_INIT_SUPPORT 1
#else /* SYSTEM */
#define MP_INIT_SUPPORT 0
#endif /* SYSTEM */
#endif /* MP_INIT_SUPPORT */


/* Indicates if the compiler supports the ident preprocessor directive for
 * placing a version string in the comment section of an object file.
 */

#ifndef MP_IDENT_SUPPORT
#if SYSTEM == SYSTEM_DGUX || SYSTEM == SYSTEM_DRSNX || \
    SYSTEM == SYSTEM_DYNIX || SYSTEM == SYSTEM_IRIX || \
    SYSTEM == SYSTEM_LINUX || SYSTEM == SYSTEM_SINIX || \
    SYSTEM == SYSTEM_SOLARIS || SYSTEM == SYSTEM_UNIXWARE
#define MP_IDENT_SUPPORT 1
#else /* SYSTEM */
#define MP_IDENT_SUPPORT 0
#endif /* SYSTEM */
#endif /* MP_IDENT_SUPPORT */


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


/* The format string passed to fprintf() which is used to display addresses.
 */

#ifndef MP_POINTER
#if ENVIRON == ENVIRON_64
#define MP_POINTER "0x%016lX"
#else /* ENVIRON */
#define MP_POINTER "0x%08lX"
#endif /* ENVIRON */
#endif /* MP_POINTER */


/* The name of the command used to invoke a text editor on a source file.
 */

#ifndef MP_EDITOR
#define MP_EDITOR "mpedit"
#endif /* MP_EDITOR */


/* The name of the environment variable which contains library options.
 */

#ifndef MP_OPTIONS
#define MP_OPTIONS "MPATROL_OPTIONS"
#endif /* MP_OPTIONS */


/* The name of the log file used to send library diagnostics to.  This may
 * be overridden at run-time using the LOGFILE option and may contain
 * special formatting characters.  It may also be affected by the environment
 * variable specified in MP_LOGDIR.
 */

#ifndef MP_LOGFILE
#define MP_LOGFILE "mpatrol.log"
#endif /* MP_LOGFILE */


/* The name of the environment variable used to specify a directory into which
 * all mpatrol log files should be written.  The directory specified must
 * actually exist.
 */

#ifndef MP_LOGDIR
#define MP_LOGDIR "LOGDIR"
#endif /* MP_LOGDIR */


/* The name of the file used to send memory allocation profiling information
 * to.  This may be overridden at run-time using the PROFFILE option and may
 * contain special formatting characters.  It may also be affected by the
 * environment variable specified in MP_PROFDIR.
 */

#ifndef MP_PROFFILE
#define MP_PROFFILE "mpatrol.out"
#endif /* MP_PROFFILE */


/* The name of the environment variable used to specify a directory into which
 * all mpatrol profiling output files should be written.  The directory
 * specified must actually exist.
 */

#ifndef MP_PROFDIR
#define MP_PROFDIR "PROFDIR"
#endif /* MP_PROFDIR */


/* The magic sequence of bytes to use at the beginning and end of every
 * profiling output file for verification purposes.  This must be exactly
 * four bytes in length and will be truncated if it is greater than that.
 */

#ifndef MP_PROFMAGIC
#define MP_PROFMAGIC "MPTL"
#endif /* MP_PROFMAGIC */


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


/* The number of bytes that distinguish where medium-sized allocations
 * begin and small-sized allocations end for profiling purposes.  This
 * may be overridden at run-time using the SMALLBOUND option.
 */

#ifndef MP_SMALLBOUND
#define MP_SMALLBOUND 32
#endif /* MP_SMALLBOUND */


/* The number of bytes that distinguish where large-sized allocations
 * begin and medium-sized allocations end for profiling purposes.  This
 * may be overridden at run-time using the MEDIUMBOUND option.
 */

#ifndef MP_MEDIUMBOUND
#define MP_MEDIUMBOUND 256
#endif /* MP_MEDIUMBOUND */


/* The number of bytes that distinguish where extra large-sized allocations
 * begin and large-sized allocations end for profiling purposes.  This
 * may be overridden at run-time using the LARGEBOUND option.
 */

#ifndef MP_LARGEBOUND
#define MP_LARGEBOUND 2048
#endif /* MP_LARGEBOUND */


/* Ensure that the small allocation boundary is less than the medium
 * allocation boundary and the medium allocation boundary is less than the
 * large allocation boundary.
 */

#if MP_SMALLBOUND >= MP_MEDIUMBOUND || MP_MEDIUMBOUND >= MP_LARGEBOUND
#error allocation boundary overlap
#endif


#endif /* MP_CONFIG_H */
