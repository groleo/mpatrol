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
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#elif TARGET == TARGET_WINDOWS
#include <process.h>
#endif /* TARGET */


#if MP_IDENT_SUPPORT
#ident "$Id: mpatrol.c,v 1.12 2000-04-20 22:24:18 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#define VERSION "2.1" /* the current version of this program */


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
static char *autosave, *check, *pagealloc;
static char *smallbound, *mediumbound, *largebound;


/* The following boolean options correspond to their uppercase equivalents when
 * setting the environment variable containing mpatrol library options.
 */

static int showmap, showfreed;
static int checkall, prof;
static int safesignals, noprotect;
static int nofree, preserve;
static int oflowwatch;
static int usemmap, usedebug;


/* The table describing a summary of all recognised options.
 */

static char *options_help[] =
{
    "1", "unsigned integer",
    "", "Specifies the limit in bytes up to which memory allocations should be",
    "", "classified as small allocations for profiling purposes.",
    "2", "unsigned integer",
    "", "Specifies the limit in bytes up to which memory allocations should be",
    "", "classified as medium allocations for profiling purposes.",
    "3", "unsigned integer",
    "", "Specifies the limit in bytes up to which memory allocations should be",
    "", "classified as large allocations for profiling purposes.",
    "A", "unsigned integer",
    "", "Specifies an allocation index at which to stop the program when it is",
    "", "being allocated.",
    "a", "unsigned integer",
    "", "Specifies an 8-bit byte pattern with which to prefill newly-allocated",
    "", "memory.",
    "C", "unsigned range",
    "", "Specifies a range of allocation indices at which to check the",
    "", "integrity of free memory and overflow buffers.",
    "c", NULL,
    "", "Specifies that all arguments to functions which allocate, reallocate",
    "", "and deallocate memory have rigorous checks performed on them.",
    "D", "unsigned integer",
    "", "Specifies the default alignment for general-purpose memory",
    "", "allocations, which must be a power of two.",
    "d", NULL,
    "", "Specifies that the LD_PRELOAD environment variable should be set so",
    "", "that even programs that were not compiled with the mpatrol library",
    "", "can be traced, but only if they were dynamically linked.",
    "e", "string",
    "", "Specifies an alternative filename with which to locate the executable",
    "", "file containing the program's symbols.",
    "F", "unsigned integer",
    "", "Specifies an allocation index at which to stop the program when it is",
    "", "being freed.",
    "f", "unsigned integer",
    "", "Specifies an 8-bit byte pattern with which to prefill newly-freed",
    "", "memory.",
    "G", NULL,
    "", "Instructs the library to save and replace certain signal handlers",
    "", "during the execution of library code and to restore them afterwards.",
    "g", NULL,
    "", "Specifies that any debugging information in the executable file",
    "", "should be used to obtain additional source-level information.",
    "L", "unsigned integer",
    "", "Specifies the limit in bytes at which all memory allocations should",
    "", "fail if the total allocated memory should increase beyond this.",
    "l", "string",
    "", "Specifies an alternative file in which to place all diagnostics from",
    "", "the mpatrol library.",
    "m", NULL,
    "", "Specifies that the library should use mmap() instead of sbrk() to",
    "", "allocate system memory.",
    "N", NULL,
    "", "Specifies that the mpatrol library's internal data structures should",
    "", "not be made read-only after every memory allocation, reallocation or",
    "", "deallocation.",
    "n", NULL,
    "", "Specifies that the mpatrol library should keep all reallocated and",
    "", "freed memory allocations.",
    "O", "unsigned integer",
    "", "Specifies the size in bytes to use for all overflow buffers, which",
    "", "must be a power of two.",
    "o", "unsigned integer",
    "", "Specifies an 8-bit byte pattern with which to fill the overflow",
    "", "buffers of all memory allocations.",
    "P", "string",
    "", "Specifies an alternative file in which to place all memory allocation",
    "", "profiling information from the mpatrol library.",
    "p", NULL,
    "", "Specifies that all memory allocations are to be profiled and sent to",
    "", "the profiling output file.",
    "Q", "unsigned integer",
    "", "Specifies the frequency at which to periodically write the profiling",
    "", "data to the profiling output file.",
    "R", "unsigned integer",
    "", "Specifies an allocation index at which to stop the program when a",
    "", "memory allocation is being reallocated.",
    "S", NULL,
    "", "Specifies that a memory map of the entire heap and a summary of all",
    "", "of the function symbols read from the program's executable file",
    "", "should be displayed at the end of program execution.",
    "s", NULL,
    "", "Specifies that a summary of all of the freed and unfreed memory",
    "", "allocations should be displayed at the end of program execution.",
    "U", "unsigned integer",
    "", "Specifies the minimum number of unfreed allocations at which to abort",
    "", "the program just before program termination.",
    "V", NULL,
    "", "Displays the version number of this program.",
    "v", NULL,
    "", "Specifies that any reallocated or freed memory allocations should",
    "", "preserve their original contents.",
    "w", NULL,
    "", "Specifies that watch point areas should be used for overflow buffers",
    "", "rather than filling with the overflow byte.",
    "X", NULL,
    "", "Specifies that each individual memory allocation should occupy at",
    "", "least one page of virtual memory and should be placed at the highest",
    "", "point within these pages.",
    "x", NULL,
    "", "Specifies that each individual memory allocation should occupy at",
    "", "least one page of virtual memory and should be placed at the lowest",
    "", "point within these pages.",
    "Z", "unsigned integer",
    "", "Specifies the random number seed which will be used when determining",
    "", "which memory allocations will randomly fail.",
    "z", "unsigned integer",
    "", "Specifies the frequency at which all memory allocations will randomly",
    "", "fail.",
    NULL
};


/* Display the quick-reference help summary.
 */

static void showoptions(void)
{
    char **s, **t;

    fputs("Options:\n", stderr);
    for (s = options_help, t = s + 1; *s != NULL; s += 2, t += 2)
        if (**s != '\0')
        {
            fprintf(stderr, "    -%s", *s);
            if (*t != NULL)
                fprintf(stderr, " <%s>", *t);
            fputc('\n', stderr);
        }
        else
            fprintf(stderr, "\t%s\n", *t);
}


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

static void setoptions(void)
{
    sprintf(options, "%s=", MP_OPTIONS);
    optlen = strlen(options);
    addoption("LOGFILE", logfile, 1);
    if (allocbyte)
        addoption("ALLOCBYTE", allocbyte, 0);
    if (allocstop)
        addoption("ALLOCSTOP", allocstop, 0);
    if (autosave)
        addoption("AUTOSAVE", autosave, 0);
    if (check)
        addoption("CHECK", check, 0);
    if (checkall)
        addoption("CHECKALL", NULL, 0);
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
    addoption("LOGALL", NULL, 0);
    if (mediumbound)
        addoption("MEDIUMBOUND", mediumbound, 0);
    if (nofree)
        addoption("NOFREE", NULL, 0);
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
    if (showfreed)
        addoption("SHOWFREED", NULL, 0);
    if (showmap)
    {
        addoption("SHOWMAP", NULL, 0);
        addoption("SHOWSYMBOLS", NULL, 0);
    }
    if (showfreed)
        addoption("SHOWUNFREED", NULL, 0);
    if (smallbound)
        addoption("SMALLBOUND", smallbound, 0);
    if (unfreedabort)
        addoption("UNFREEDABORT", unfreedabort, 0);
    if (usedebug)
        addoption("USEDEBUG", NULL, 0);
    if (usemmap)
        addoption("USEMMAP", NULL, 0);
    if (putenv(options))
    {
        fprintf(stderr, "%s: Cannot set environment variable `%s'\n", progname,
                MP_OPTIONS);
        exit(EXIT_FAILURE);
    }
}


/* Convert the command line options to mpatrol library options and run the
 * specified command and arguments.
 */

int main(int argc, char **argv)
{
#if MP_PRELOAD_SUPPORT
    static char p[256];
#endif /* MP_PRELOAD_SUPPORT */
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
    int c, d, e, r, v;

    d = e = r = v = 0;
    progname = argv[0];
    logfile = "mpatrol.%n.log";
    while ((c = __mp_getopt(argc, argv,
             "1:2:3:A:a:C:cD:de:F:f:GgL:l:mNnO:o:P:pQ:R:SsU:VvwXxZ:z:")) != EOF)
        switch (c)
        {
          case '1':
            smallbound = __mp_optarg;
            break;
          case '2':
            mediumbound = __mp_optarg;
            break;
          case '3':
            largebound = __mp_optarg;
            break;
          case 'A':
            allocstop = __mp_optarg;
            break;
          case 'a':
            allocbyte = __mp_optarg;
            break;
          case 'C':
            check = __mp_optarg;
            break;
          case 'c':
            checkall = 1;
            break;
          case 'D':
            defalign = __mp_optarg;
            break;
          case 'd':
            d = 1;
            break;
          case 'e':
            progfile = __mp_optarg;
            break;
          case 'F':
            freestop = __mp_optarg;
            break;
          case 'f':
            freebyte = __mp_optarg;
            break;
          case 'G':
            safesignals = 1;
            break;
          case 'g':
            usedebug = 1;
            break;
          case 'L':
            limit = __mp_optarg;
            break;
          case 'l':
            logfile = __mp_optarg;
            break;
          case 'm':
            usemmap = 1;
            break;
          case 'N':
            noprotect = 1;
            break;
          case 'n':
            nofree = 1;
            break;
          case 'O':
            oflowsize = __mp_optarg;
            break;
          case 'o':
            oflowbyte = __mp_optarg;
            break;
          case 'P':
            proffile = __mp_optarg;
            break;
          case 'p':
            prof = 1;
            break;
          case 'Q':
            autosave = __mp_optarg;
            break;
          case 'R':
            reallocstop = __mp_optarg;
            break;
          case 'S':
            showmap = 1;
            break;
          case 's':
            showfreed = 1;
            break;
          case 'U':
            unfreedabort = __mp_optarg;
            break;
          case 'V':
            v = 1;
            break;
          case 'v':
            preserve = 1;
            break;
          case 'w':
            oflowwatch = 1;
            break;
          case 'X':
            pagealloc = "UPPER";
            break;
          case 'x':
            pagealloc = "LOWER";
            break;
          case 'Z':
            failseed = __mp_optarg;
            break;
          case 'z':
            failfreq = __mp_optarg;
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
    if ((argc == 0) || (e == 1))
    {
        if ((v == 0) || (e == 1))
        {
            fprintf(stderr, "Usage: %s [options] <command> [arguments]\n\n",
                    progname);
            showoptions();
        }
        exit(EXIT_FAILURE);
    }
    setoptions();
#if MP_PRELOAD_SUPPORT
    /* The dynamic linker on some UNIX systems supports requests for it to
     * preload a specified list of shared libraries before running a process,
     * via the LD_PRELOAD environment variable.  If any of the specified
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
