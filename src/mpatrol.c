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
 * A tool for setting various mpatrol library options when running a program
 * that has been linked with the mpatrol library.
 */


#include "getopt.h"
#include "version.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#if TARGET == TARGET_UNIX
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#elif TARGET == TARGET_WINDOWS
#include <process.h>
#endif /* TARGET */


#if MP_IDENT_SUPPORT
#ident "$Id: mpatrol.c,v 1.27 2000-11-02 17:51:38 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#define VERSION "2.3" /* the current version of this program */


/* The flags used to parse the command line options.
 */

typedef enum options_flags
{
    OF_SMALLBOUND     = '1',
    OF_MEDIUMBOUND    = '2',
    OF_LARGEBOUND     = '3',
    OF_ALLOCSTOP      = 'A',
    OF_ALLOCBYTE      = 'a',
    OF_CHECK          = 'C',
    OF_CHECKALL       = 'c',
    OF_DEFALIGN       = 'D',
    OF_DYNAMIC        = 'd',
    OF_PROGFILE       = 'e',
    OF_FREESTOP       = 'F',
    OF_FREEBYTE       = 'f',
    OF_SAFESIGNALS    = 'G',
    OF_USEDEBUG       = 'g',
    OF_HELP           = 'h',
    OF_LIMIT          = 'L',
    OF_LOGFILE        = 'l',
    OF_ALLOWOFLOW     = 'M',
    OF_USEMMAP        = 'm',
    OF_NOPROTECT      = 'N',
    OF_NOFREE         = 'n',
    OF_OFLOWSIZE      = 'O',
    OF_OFLOWBYTE      = 'o',
    OF_PROFFILE       = 'P',
    OF_PROF           = 'p',
    OF_AUTOSAVE       = 'Q',
    OF_REALLOCSTOP    = 'R',
    OF_SHOWMAP        = 'S',
    OF_SHOWFREED      = 's',
    OF_UNFREEDABORT   = 'U',
    OF_VERSION        = 'V',
    OF_PRESERVE       = 'v',
    OF_OFLOWWATCH     = 'w',
    OF_PAGEALLOCUPPER = 'X',
    OF_PAGEALLOCLOWER = 'x',
    OF_FAILSEED       = 'Z',
    OF_FAILFREQ       = 'z',
    OF_CHECKALLOCS    = SHORTOPT_MAX + 1,
    OF_CHECKFREES,
    OF_CHECKMEMORY,
    OF_CHECKREALLOCS,
    OF_LOGALL,
    OF_LOGALLOCS,
    OF_LOGFREES,
    OF_LOGMEMORY,
    OF_LOGREALLOCS,
    OF_SHOWALL,
    OF_SHOWENV,
    OF_SHOWSYMBOLS,
    OF_SHOWUNFREED
}
options_flags;


/* The buffer used to build up the environment variable containing options
 * for the mpatrol library.
 */

static char options[1024];


/* The current length of the options buffer.
 */

static size_t optlen;


/* The filename used to invoke this tool.
 */

static char *progname;


/* The following string options correspond to their uppercase equivalents when
 * setting the environment variable containing mpatrol library options.
 */

static char *allocstop, *reallocstop, *freestop;
static char *allocbyte, *freebyte;
static char *oflowbyte, *oflowsize;
static char *defalign, *limit;
static char *failfreq, *failseed, *unfreedabort;
static char *logfile, *proffile, *progfile;
static char *autosave, *check;
static char *nofree, *pagealloc;
static char *smallbound, *mediumbound, *largebound;


/* The following boolean options correspond to their uppercase equivalents when
 * setting the environment variable containing mpatrol library options.
 */

static int logallocs, logreallocs;
static int logfrees, logmemory;
static int showmap, showsymbols;
static int showfreed, showunfreed;
static int checkallocs, checkreallocs;
static int checkfrees, checkmemory;
static int prof, safesignals;
static int noprotect, preserve;
static int oflowwatch, usemmap;
static int usedebug, allowoflow;


/* The table describing all recognised options.
 */

static option options_table[] =
{
    {"alloc-byte", OF_ALLOCBYTE, "unsigned integer",
     "\tSpecifies an 8-bit byte pattern with which to prefill newly-allocated\n"
     "\tmemory.\n"},
    {"alloc-stop", OF_ALLOCSTOP, "unsigned integer",
     "\tSpecifies an allocation index at which to stop the program when it is\n"
     "\tbeing allocated.\n"},
    {"allow-oflow", OF_ALLOWOFLOW, NULL,
     "\tSpecifies that a warning rather than an error should be produced if\n"
     "\tany memory operation function overflows the boundaries of a memory\n"
     "\tallocation, and that the operation should still be performed.\n"},
    {"auto-save", OF_AUTOSAVE, "unsigned integer",
     "\tSpecifies the frequency at which to periodically write the profiling\n"
     "\tdata to the profiling output file.\n"},
    {"check", OF_CHECK, "unsigned range",
     "\tSpecifies a range of allocation indices at which to check the\n"
     "\tintegrity of free memory and overflow buffers.\n"},
    {"check-all", OF_CHECKALL, NULL,
     "\tEquivalent to the --check-allocs, --check-reallocs, --check-frees and\n"
     "\t--check-memory options specified together.\n"},
    {"check-allocs", OF_CHECKALLOCS, NULL,
     "\tChecks that no attempt is made to allocate a block of memory of size\n"
     "\tzero.\n"},
    {"check-frees", OF_CHECKFREES, NULL,
     "\tChecks that no attempt is made to deallocate a NULL pointer.\n"},
    {"check-memory", OF_CHECKMEMORY, NULL,
     "\tChecks that no attempt is made to perform a zero-length memory\n"
     "\toperation on a NULL pointer.\n"},
    {"check-reallocs", OF_CHECKREALLOCS, NULL,
     "\tChecks that no attempt is made to reallocate a NULL pointer or resize\n"
     "\tan existing block of memory to size zero.\n"},
    {"def-align", OF_DEFALIGN, "unsigned integer",
     "\tSpecifies the default alignment for general-purpose memory\n"
     "\tallocations, which must be a power of two.\n"},
    {"dynamic", OF_DYNAMIC, NULL,
     "\tSpecifies that programs which were not linked with the mpatrol\n"
     "\tlibrary should also be traced, but only if they were dynamically\n"
     "\tlinked.\n"},
    {"fail-freq", OF_FAILFREQ, "unsigned integer",
     "\tSpecifies the frequency at which all memory allocations will randomly\n"
     "\tfail.\n"},
    {"fail-seed", OF_FAILSEED, "unsigned integer",
     "\tSpecifies the random number seed which will be used when determining\n"
     "\twhich memory allocations will randomly fail.\n"},
    {"free-byte", OF_FREEBYTE, "unsigned integer",
     "\tSpecifies an 8-bit byte pattern with which to prefill newly-freed\n"
     "\tmemory.\n"},
    {"free-stop", OF_FREESTOP, "unsigned integer",
     "\tSpecifies an allocation index at which to stop the program when it is\n"
     "\tbeing freed.\n"},
    {"help", OF_HELP, NULL,
     "\tDisplays this quick-reference option summary.\n"},
    {"large-bound", OF_LARGEBOUND, "unsigned integer",
     "\tSpecifies the limit in bytes up to which memory allocations should be\n"
     "\tclassified as large allocations for profiling purposes.\n"},
    {"limit", OF_LIMIT, "unsigned integer",
     "\tSpecifies the limit in bytes at which all memory allocations should\n"
     "\tfail if the total allocated memory should increase beyond this.\n"},
    {"log-all", OF_LOGALL, NULL,
     "\tEquivalent to the --log-allocs, --log-reallocs, --log-frees and\n"
     "\t--log-memory options specified together.\n"},
    {"log-allocs", OF_LOGALLOCS, NULL,
     "\tSpecifies that all memory allocations are to be logged and sent to\n"
     "\tthe log file.\n"},
    {"log-file", OF_LOGFILE, "string",
     "\tSpecifies an alternative file in which to place all diagnostics from\n"
     "\tthe mpatrol library.\n"},
    {"log-frees", OF_LOGFREES, NULL,
     "\tSpecifies that all memory deallocations are to be logged and sent to\n"
     "\tthe log file.\n"},
    {"log-memory", OF_LOGMEMORY, NULL,
     "\tSpecifies that all memory operations are to be logged and sent to the\n"
     "\tlog file.\n"},
    {"log-reallocs", OF_LOGREALLOCS, NULL,
     "\tSpecifies that all memory reallocations are to be logged and sent to\n"
     "\tthe log file.\n"},
    {"medium-bound", OF_MEDIUMBOUND, "unsigned integer",
     "\tSpecifies the limit in bytes up to which memory allocations should be\n"
     "\tclassified as medium allocations for profiling purposes.\n"},
    {"no-free", OF_NOFREE, "unsigned integer",
     "\tSpecifies that a number of recently-freed memory allocations should\n"
     "\tbe prevented from being returned to the free memory pool.\n"},
    {"no-protect", OF_NOPROTECT, NULL,
     "\tSpecifies that the mpatrol library's internal data structures should\n"
     "\tnot be made read-only after every memory allocation, reallocation or\n"
     "\tdeallocation.\n"},
    {"oflow-byte", OF_OFLOWBYTE, "unsigned integer",
     "\tSpecifies an 8-bit byte pattern with which to fill the overflow\n"
     "\tbuffers of all memory allocations.\n"},
    {"oflow-size", OF_OFLOWSIZE, "unsigned integer",
     "\tSpecifies the size in bytes to use for all overflow buffers, which\n"
     "\tmust be a power of two.\n"},
    {"oflow-watch", OF_OFLOWWATCH, NULL,
     "\tSpecifies that watch point areas should be used for overflow buffers\n"
     "\trather than filling with the overflow byte.\n"},
    {"page-alloc-lower", OF_PAGEALLOCLOWER, NULL,
     "\tSpecifies that each individual memory allocation should occupy at\n"
     "\tleast one page of virtual memory and should be placed at the lowest\n"
     "\tpoint within these pages.\n"},
    {"page-alloc-upper", OF_PAGEALLOCUPPER, NULL,
     "\tSpecifies that each individual memory allocation should occupy at\n"
     "\tleast one page of virtual memory and should be placed at the highest\n"
     "\tpoint within these pages.\n"},
    {"preserve", OF_PRESERVE, NULL,
     "\tSpecifies that any reallocated or freed memory allocations should\n"
     "\tpreserve their original contents.\n"},
    {"prof", OF_PROF, NULL,
     "\tSpecifies that all memory allocations are to be profiled and sent to\n"
     "\tthe profiling output file.\n"},
    {"prof-file", OF_PROFFILE, "string",
     "\tSpecifies an alternative file in which to place all memory allocation\n"
     "\tprofiling information from the mpatrol library.\n"},
    {"prog-file", OF_PROGFILE, "string",
     "\tSpecifies an alternative filename with which to locate the executable\n"
     "\tfile containing the program's symbols.\n"},
    {"realloc-stop", OF_REALLOCSTOP, "unsigned integer",
     "\tSpecifies an allocation index at which to stop the program when a\n"
     "\tmemory allocation is being reallocated.\n"},
    {"safe-signals", OF_SAFESIGNALS, NULL,
     "\tInstructs the library to save and replace certain signal handlers\n"
     "\tduring the execution of library code and to restore them\n"
     "\tafterwards.\n"},
    {"show-all", OF_SHOWALL, NULL,
     "\tEquivalent to the --show-freed, --show-unfreed, --show-map and\n"
     "\t--show-symbols options specified together.\n"},
    {"show-env", OF_SHOWENV, NULL,
     "\tDisplays the contents of the " MP_OPTIONS " environment variable.\n"},
    {"show-freed", OF_SHOWFREED, NULL,
     "\tSpecifies that a summary of all of the freed memory allocations\n"
     "\tshould be displayed at the end of program execution.\n"},
    {"show-map", OF_SHOWMAP, NULL,
     "\tSpecifies that a memory map of the entire heap should be displayed at\n"
     "\tthe end of program execution.\n"},
    {"show-symbols", OF_SHOWSYMBOLS, NULL,
     "\tSpecifies that a summary of all of the function symbols read from\n"
     "\tthe program's executable file should be displayed at the end of\n"
     "\tprogram execution.\n"},
    {"show-unfreed", OF_SHOWUNFREED, NULL,
     "\tSpecifies that a summary of all of the unfreed memory allocations\n"
     "\tshould be displayed at the end of program execution.\n"},
    {"small-bound", OF_SMALLBOUND, "unsigned integer",
     "\tSpecifies the limit in bytes up to which memory allocations should be\n"
     "\tclassified as small allocations for profiling purposes.\n"},
    {"unfreed-abort", OF_UNFREEDABORT, "unsigned integer",
     "\tSpecifies the minimum number of unfreed allocations at which to abort\n"
     "\tthe program just before program termination.\n"},
    {"use-debug", OF_USEDEBUG, NULL,
     "\tSpecifies that any debugging information in the executable file\n"
     "\tshould be used to obtain additional source-level information.\n"},
    {"use-mmap", OF_USEMMAP, NULL,
     "\tSpecifies that the library should use mmap() instead of sbrk() to\n"
     "\tallocate system memory.\n"},
    {"version", OF_VERSION, NULL,
     "\tDisplays the version number of this program.\n"},
    NULL
};


/* Add an option and possibly an associated value to the options buffer.
 */

static void addoption(char *o, char *v, int s)
{
    size_t l;
    int q;

    q = 0;
    l = strlen(o) + (s == 0);
    if (v != NULL)
    {
        l += strlen(v) + 1;
        if (strchr(v, ' '))
        {
            l += 2;
            q = 1;
        }
    }
    if (optlen + l >= sizeof(options))
    {
        fprintf(stderr, "%s: Environment variable too long\n", progname);
        exit(EXIT_FAILURE);
    }
    if (q == 1)
        sprintf(options + optlen, "%s%s=\"%s\"", (s == 0) ? " " : "", o, v);
    else if (v != NULL)
        sprintf(options + optlen, "%s%s=%s", (s == 0) ? " " : "", o, v);
    else
        sprintf(options + optlen, "%s%s", (s == 0) ? " " : "", o);
    optlen += l;
}


/* Build the environment variable containing mpatrol library options.
 */

static void setoptions(int s)
{
    sprintf(options, "%s=", MP_OPTIONS);
    optlen = strlen(options);
    addoption("LOGFILE", logfile, 1);
    if (allocbyte)
        addoption("ALLOCBYTE", allocbyte, 0);
    if (allocstop)
        addoption("ALLOCSTOP", allocstop, 0);
    if (allowoflow)
        addoption("ALLOWOFLOW", NULL, 0);
    if (autosave)
        addoption("AUTOSAVE", autosave, 0);
    if (check)
        addoption("CHECK", check, 0);
    if (checkallocs && checkfrees && checkmemory && checkreallocs)
        addoption("CHECKALL", NULL, 0);
    else
    {
        if (checkallocs)
            addoption("CHECKALLOCS", NULL, 0);
        if (checkfrees)
            addoption("CHECKFREES", NULL, 0);
        if (checkmemory)
            addoption("CHECKMEMORY", NULL, 0);
        if (checkreallocs)
            addoption("CHECKREALLOCS", NULL, 0);
    }
    if (defalign)
        addoption("DEFALIGN", defalign, 0);
    if (failfreq)
        addoption("FAILFREQ", failfreq, 0);
    if (failseed)
        addoption("FAILSEED", failseed, 0);
    if (freebyte)
        addoption("FREEBYTE", freebyte, 0);
    if (freestop)
        addoption("FREESTOP", freestop, 0);
    if (largebound)
        addoption("LARGEBOUND", largebound, 0);
    if (limit)
        addoption("LIMIT", limit, 0);
    if (logallocs && logfrees && logmemory && logreallocs)
        addoption("LOGALL", NULL, 0);
    else
    {
        if (logallocs)
            addoption("LOGALLOCS", NULL, 0);
        if (logfrees)
            addoption("LOGFREES", NULL, 0);
        if (logmemory)
            addoption("LOGMEMORY", NULL, 0);
        if (logreallocs)
            addoption("LOGREALLOCS", NULL, 0);
    }
    if (mediumbound)
        addoption("MEDIUMBOUND", mediumbound, 0);
    if (nofree)
        addoption("NOFREE", nofree, 0);
    if (noprotect)
        addoption("NOPROTECT", NULL, 0);
    if (oflowbyte)
        addoption("OFLOWBYTE", oflowbyte, 0);
    if (oflowsize)
        addoption("OFLOWSIZE", oflowsize, 0);
    if (oflowwatch)
        addoption("OFLOWWATCH", NULL, 0);
    if (pagealloc)
        addoption("PAGEALLOC", pagealloc, 0);
    if (preserve)
        addoption("PRESERVE", NULL, 0);
    if (prof)
        addoption("PROF", NULL, 0);
    if (proffile)
        addoption("PROFFILE", proffile, 0);
    if (progfile)
        addoption("PROGFILE", progfile, 0);
    if (reallocstop)
        addoption("REALLOCSTOP", reallocstop, 0);
    if (safesignals)
        addoption("SAFESIGNALS", NULL, 0);
    if (showfreed && showmap && showsymbols && showunfreed)
        addoption("SHOWALL", NULL, 0);
    else
    {
        if (showfreed)
            addoption("SHOWFREED", NULL, 0);
        if (showmap)
            addoption("SHOWMAP", NULL, 0);
        if (showsymbols)
            addoption("SHOWSYMBOLS", NULL, 0);
        if (showunfreed)
            addoption("SHOWUNFREED", NULL, 0);
    }
    if (smallbound)
        addoption("SMALLBOUND", smallbound, 0);
    if (unfreedabort)
        addoption("UNFREEDABORT", unfreedabort, 0);
    if (usedebug)
        addoption("USEDEBUG", NULL, 0);
    if (usemmap)
        addoption("USEMMAP", NULL, 0);
    if (s != 0)
        fprintf(stdout, "%s\n", strchr(options, '=') + 1);
    else if (putenv(options))
    {
        fprintf(stderr, "%s: Cannot set environment variable `%s'\n", progname,
                MP_OPTIONS);
        exit(EXIT_FAILURE);
    }
}


#if TARGET == TARGET_UNIX
/* Search for an executable file in the current search path.
 */

static char *execpath(char *s)
{
    static char t[MAXPATHLEN];
    char *p, *x, *y;

    if (strchr(s, '/'))
        return s;
    if ((p = getenv("PATH")) == NULL)
        p = "";
    if ((p = strdup(p)) == NULL)
    {
        fprintf(stderr, "%s: Out of memory\n", progname);
        exit(EXIT_FAILURE);
    }
    for (x = y = p; y != NULL; x = y + 1)
    {
        if (y = strchr(x, ':'))
            *y = '\0';
        if (*x == '\0')
            x = ".";
        sprintf(t, "%s/%s", x, s);
        if (access(t, X_OK) == 0)
        {
            s = t;
            break;
        }
    }
    free(p);
    return s;
}
#endif /* TARGET */


/* Convert the command line options to mpatrol library options and run the
 * specified command and arguments.
 */

int main(int argc, char **argv)
{
#if MP_PRELOAD_SUPPORT
    static char p[256];
#endif /* MP_PRELOAD_SUPPORT */
    char b[256];
#if TARGET == TARGET_UNIX
    pid_t f;
#else /* TARGET */
#if TARGET == TARGET_WINDOWS
    char **s;
#else /* TARGET */
    char *s;
#endif /* TARGET */
    size_t i, l;
#endif /* TARGET */
    int c, d, e, h, r, v, w;

    d = e = h = r = v = w = 0;
    progname = argv[0];
    logfile = "mpatrol.%n.log";
    proffile = "mpatrol.%n.out";
    while ((c = __mp_getopt(argc, argv, __mp_shortopts(b, options_table),
             options_table)) != EOF)
        switch (c)
        {
          case OF_ALLOCBYTE:
            allocbyte = __mp_optarg;
            break;
          case OF_ALLOCSTOP:
            allocstop = __mp_optarg;
            break;
          case OF_ALLOWOFLOW:
            allowoflow = 1;
            break;
          case OF_AUTOSAVE:
            autosave = __mp_optarg;
            break;
          case OF_CHECK:
            check = __mp_optarg;
            break;
          case OF_CHECKALL:
            checkallocs = 1;
            checkreallocs = 1;
            checkfrees = 1;
            checkmemory = 1;
            break;
          case OF_CHECKALLOCS:
            checkallocs = 1;
            break;
          case OF_CHECKFREES:
            checkfrees = 1;
            break;
          case OF_CHECKMEMORY:
            checkmemory = 1;
            break;
          case OF_CHECKREALLOCS:
            checkreallocs = 1;
            break;
          case OF_DEFALIGN:
            defalign = __mp_optarg;
            break;
          case OF_DYNAMIC:
            d = 1;
            break;
          case OF_FAILFREQ:
            failfreq = __mp_optarg;
            break;
          case OF_FAILSEED:
            failseed = __mp_optarg;
            break;
          case OF_FREEBYTE:
            freebyte = __mp_optarg;
            break;
          case OF_FREESTOP:
            freestop = __mp_optarg;
            break;
          case OF_HELP:
            h = 1;
            break;
          case OF_LARGEBOUND:
            largebound = __mp_optarg;
            break;
          case OF_LIMIT:
            limit = __mp_optarg;
            break;
          case OF_LOGALL:
            logallocs = 1;
            logreallocs = 1;
            logfrees = 1;
            logmemory = 1;
            break;
          case OF_LOGALLOCS:
            logallocs = 1;
            break;
          case OF_LOGFILE:
            logfile = __mp_optarg;
            break;
          case OF_LOGFREES:
            logfrees = 1;
            break;
          case OF_LOGMEMORY:
            logmemory = 1;
            break;
          case OF_LOGREALLOCS:
            logreallocs = 1;
            break;
          case OF_MEDIUMBOUND:
            mediumbound = __mp_optarg;
            break;
          case OF_NOFREE:
            nofree = __mp_optarg;
            break;
          case OF_NOPROTECT:
            noprotect = 1;
            break;
          case OF_OFLOWBYTE:
            oflowbyte = __mp_optarg;
            break;
          case OF_OFLOWSIZE:
            oflowsize = __mp_optarg;
            break;
          case OF_OFLOWWATCH:
            oflowwatch = 1;
            break;
          case OF_PAGEALLOCLOWER:
            pagealloc = "LOWER";
            break;
          case OF_PAGEALLOCUPPER:
            pagealloc = "UPPER";
            break;
          case OF_PRESERVE:
            preserve = 1;
            break;
          case OF_PROF:
            prof = 1;
            break;
          case OF_PROFFILE:
            proffile = __mp_optarg;
            break;
          case OF_PROGFILE:
            progfile = __mp_optarg;
            break;
          case OF_REALLOCSTOP:
            reallocstop = __mp_optarg;
            break;
          case OF_SAFESIGNALS:
            safesignals = 1;
            break;
          case OF_SHOWALL:
            showmap = 1;
            showsymbols = 1;
            showfreed = 1;
            showunfreed = 1;
            break;
          case OF_SHOWENV:
            w = 1;
            break;
          case OF_SHOWFREED:
            showfreed = 1;
            break;
          case OF_SHOWMAP:
            showmap = 1;
            break;
          case OF_SHOWSYMBOLS:
            showsymbols = 1;
            break;
          case OF_SHOWUNFREED:
            showunfreed = 1;
            break;
          case OF_SMALLBOUND:
            smallbound = __mp_optarg;
            break;
          case OF_UNFREEDABORT:
            unfreedabort = __mp_optarg;
            break;
          case OF_USEDEBUG:
            usedebug = 1;
            break;
          case OF_USEMMAP:
            usemmap = 1;
            break;
          case OF_VERSION:
            v = 1;
            break;
          default:
            e = 1;
            break;
        }
    argc -= __mp_optindex;
    argv += __mp_optindex;
    if (v == 1)
    {
        fprintf(stderr, "%s %s\n%s\n\n", progname, VERSION, __mp_copyright);
        fputs("This is free software, and you are welcome to redistribute it "
              "under certain\n", stderr);
        fputs("conditions; see the GNU Library General Public License for "
              "details.\n\n", stderr);
        fputs("For the latest mpatrol release and documentation,\n", stderr);
        fprintf(stderr, "visit %s.\n\n", __mp_homepage);
    }
    else if ((argc == 0) && (h == 0) && (w == 0))
        e = 1;
    if ((argc == 0) || (e == 1) || (h == 1) || (w == 1))
    {
        if ((e == 1) || (h == 1))
        {
            fprintf(stderr, "Usage: %s [options] <command> [arguments]\n\n",
                    progname);
            if (h == 0)
                fprintf(stderr, "Type `%s --help' for a complete list of "
                        "options.\n", progname);
            else
                __mp_showopts(options_table);
        }
        if (w == 1)
            setoptions(1);
        if (e == 1)
            exit(EXIT_FAILURE);
        exit(EXIT_SUCCESS);
    }
    setoptions(0);
#if MP_PRELOAD_SUPPORT
    /* The dynamic linker on some UNIX systems supports requests for it to
     * preload a specified list of shared libraries before running a process,
     * via the MP_PRELOAD_NAME environment variable.  If any of the specified
     * libraries only exist as archive libraries then the mpatrol library
     * should be built to contain them since there is no way to preload an
     * archive library.  However, this may have repercussions when building a
     * shared library from position-dependent code.
     */
    if (d == 1)
    {
        sprintf(p, "%s=%s", MP_PRELOAD_NAME, MP_PRELOAD_LIBS);
        if (putenv(p))
        {
            fprintf(stderr, "%s: Cannot set environment variable `%s'\n",
                    progname, MP_PRELOAD_NAME);
            exit(EXIT_FAILURE);
        }
    }
#endif /* MP_PRELOAD_SUPPORT */
    /* Prepare to run the command that is to be tested.  We return the exit
     * status from the command after it has been run in case it was originally
     * run from a script.
     */
    fflush(NULL);
#if TARGET == TARGET_UNIX
    /* We need to use the fork() and execvp() combination on UNIX platforms
     * in case we are using the -d option, in which case we cannot use system()
     * since that will use the shell to invoke the command, possibly resulting
     * in an extra log file tracing the shell itself.
     */
    if ((f = fork()) < 0)
    {
        fprintf(stderr, "%s: Cannot create process\n", progname);
        exit(EXIT_FAILURE);
    }
    if (f == 0)
    {
        /* Programs invoked with the execvp() function will not contain the
         * full path that they were run with in argv[0].  We need to patch
         * this here since otherwise the mpatrol library is unlikely to find
         * the executable file to read symbols from.  Hopefully argv is located
         * on the stack, otherwise it may be read-only and cause the following
         * assignment to crash.
         */
        argv[0] = execpath(argv[0]);
        execvp(argv[0], argv);
        fprintf(stderr, "%s: Cannot execute command `%s'\n", progname, argv[0]);
        exit(EXIT_FAILURE);
    }
    while (waitpid(f, &r, 0) < 0)
        if (errno != EINTR)
        {
            fprintf(stderr, "%s: Process could not be created\n", progname);
            exit(EXIT_FAILURE);
        }
    if (!WIFEXITED(r))
        exit(EXIT_FAILURE);
    r = WEXITSTATUS(r);
#elif TARGET == TARGET_WINDOWS
    /* To avoid extra overhead we call the spawnvp() function on Windows
     * platforms.  However, for some strange reason, it concatenates all
     * of its arguments into a single string with spaces in between arguments,
     * but does NOT quote arguments with spaces in them!  For that reason,
     * we'll have to do some extra work here.
     */
    if ((s = (char **) calloc(argc + 1, sizeof(char *))) == NULL)
    {
        fprintf(stderr, "%s: Out of memory\n", progname);
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < argc; i++)
        if (strchr(argv[i], ' '))
        {
            l = strlen(argv[i]) + 3;
            if ((s[i] = (char *) malloc(l)) == NULL)
            {
                fprintf(stderr, "%s: Out of memory\n", progname);
                exit(EXIT_FAILURE);
            }
            sprintf(s[i], "\"%s\"", argv[i]);
        }
        else if ((s[i] = strdup(argv[i])) == NULL)
        {
            fprintf(stderr, "%s: Out of memory\n", progname);
            exit(EXIT_FAILURE);
        }
    if ((r = spawnvp(_P_WAIT, s[0], s)) == -1)
    {
        fprintf(stderr, "%s: Cannot execute command `%s'\n", progname, argv[0]);
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < argc; i++)
        free(s[i]);
    free(s);
#else /* TARGET */
    /* Because we are using system() to run the command, we need to ensure
     * that all arguments that contain spaces are correctly quoted.  We also
     * need to convert the argument array to a string, so we perform two
     * passes.  The first pass counts the number of characters required for
     * the final command string and allocates that amount of memory from the
     * heap.  The second pass then fills in the command string and executes
     * the command.
     */
    for (i = l = 0; i < argc; i++)
    {
        l += strlen(argv[i]) + 1;
        if (strchr(argv[i], ' '))
            l += 2;
    }
    if ((s = (char *) malloc(l)) == NULL)
    {
        fprintf(stderr, "%s: Out of memory\n", progname);
        exit(EXIT_FAILURE);
    }
    for (i = l = 0; i < argc; i++)
        if (strchr(argv[i], ' '))
        {
            sprintf(s + l, "%s\"%s\"", (i > 0) ? " " : "", argv[i]);
            l += strlen(argv[i]) + (i > 0) + 2;
        }
        else
        {
            sprintf(s + l, "%s%s", (i > 0) ? " " : "", argv[i]);
            l += strlen(argv[i]) + (i > 0);
        }
    if ((r = system(s)) == -1)
    {
        fprintf(stderr, "%s: Cannot execute command `%s'\n", progname, argv[0]);
        exit(EXIT_FAILURE);
    }
    free(s);
#if (TARGET == TARGET_AMIGA && defined(__GNUC__))
    /* When gcc is used on AmigaOS, the return value from system() is similar
     * to that on UNIX, so we need to modify it here.
     */
    r = ((unsigned int) r >> 8) & 0xFF;
#endif /* TARGET && __GNUC__ */
#endif /* TARGET */
    return r;
}
