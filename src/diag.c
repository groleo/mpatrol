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
 * Diagnostics.  Most of the diagnostics go to a log file which is opened
 * as soon as possible, but if this fails then the standard error is used.
 * Many of these functions also deal with the formatting and displaying of
 * certain data structures used by the mpatrol library.
 */


#include "diag.h"
#if MP_THREADS_SUPPORT
#include "mutex.h"
#endif /* MP_THREADS_SUPPORT */
#include "utils.h"
#include "version.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#if TARGET == TARGET_UNIX
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif /* TARGET */


#if MP_IDENT_SUPPORT
#ident "$Id: diag.c,v 1.58 2001-02-07 23:58:37 graeme Exp $"
#else /* MP_IDENT_SUPPORT */
static MP_CONST MP_VOLATILE char *diag_id = "$Id: diag.c,v 1.58 2001-02-07 23:58:37 graeme Exp $";
#endif /* MP_IDENT_SUPPORT */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* The file pointer to the log file.  This should not really be a file scope
 * variable as it prevents this module from being re-entrant.
 */

static FILE *logfile;


/* The byte array used for file buffering purposes.  Care must be taken to
 * ensure that this buffer is not used for more than one file and this should
 * not really be a file scope variable as it prevents this module from being
 * re-entrant.
 */

static char buffer[256];


/* The current date and time at which the log file is being created.  This
 * must be fixed once it has been determined since it may be used in several
 * places.
 */

static time_t currenttime;


/* The total error and warning counts.  These should really be reset after
 * every initialisation of the library, but as we are not currently allowing
 * library to be reinitialised, this doesn't matter.
 */

static unsigned long errors, warnings;


/* This array should always be kept in step with the errortype enumeration.
 */

static char *errornames[ET_MAX] =
{
    "ALLOVF",
    "ALLZER",
    "BADALN",
    "FRDCOR",
    "FRDOPN",
    "FRDOVF",
    "FRECOR",
    "FRENUL",
    "FREOPN",
    "ILLMEM",
    "INCOMP",
    "MAXALN",
    "MISMAT",
    "NOTALL",
    "NULOPN",
    "OUTMEM",
    "PRVFRD",
    "RNGOVF",
    "RNGOVL",
    "RSZNUL",
    "RSZZER",
    "STROVF",
    "ZERALN"
};


/* This array should always be kept in step with the alloctype enumeration.
 * Note that AT_MAX is used in diagnostic messages to specify that the
 * message is internal and did not come from a call to a normal memory
 * allocation function.
 */

MP_GLOBAL char *__mp_functionnames[AT_MAX] =
{
    "malloc",
    "calloc",
    "memalign",
    "valloc",
    "pvalloc",
    "alloca",
    "strdup",
    "strndup",
    "strsave",
    "strnsave",
    "strdupa",
    "strndupa",
    "realloc",
    "reallocf",
    "recalloc",
    "expand",
    "free",
    "cfree",
    "dealloca",
    "xmalloc",
    "xcalloc",
    "xstrdup",
    "xrealloc",
    "xfree",
    "operator new",
    "operator new[]",
    "operator delete",
    "operator delete[]",
    "memset",
    "bzero",
    "memccpy",
    "memcpy",
    "memmove",
    "bcopy",
    "memchr",
    "memmem",
    "memcmp",
    "bcmp"
};


/* The flags used to control the diagnostics from the mpatrol library.
 */

MP_GLOBAL unsigned long __mp_diagflags = 0;


/* Process a file name, expanding any special characters.
 */

static
void
processfile(meminfo *m, char *s, char *b, size_t l)
{
    char *p, *t;
    size_t i;

    for (i = 0; (i < l - 1) && (*s != '\0'); i++, s++)
        if (*s == '%')
            switch (s[1])
            {
              case 'd':
                /* Replace %d with the current date in the form YYYYMMDD.
                 */
                if (!currenttime)
                    currenttime = time(NULL);
                if (currenttime != (time_t) -1)
                    strftime(b + i, l - i, "%Y%m%d", localtime(&currenttime));
                else
                    strcpy(b + i, "today");
                i += strlen(b + i) - 1;
                s++;
                break;
              case 'f':
                /* Replace %f with the program filename, with all path
                 * separation characters replaced by underscores.
                 */
                if (((p = m->prog) == NULL) || (*p == '\0'))
                    p = "mpatrol";
                while (*p != '\0')
                {
#if TARGET == TARGET_UNIX
                    if (*p == '/')
#elif TARGET == TARGET_AMIGA
                    if ((*p == ':') || (*p == '/'))
#elif TARGET == TARGET_WINDOWS || TARGET == TARGET_NETWARE
                    if ((*p == ':') || (*p == '/') || (*p == '\\'))
#endif /* TARGET */
                        b[i++] = '_';
                    else
                        b[i++] = *p;
                    p++;
                }
                i--;
                s++;
                break;
              case 'n':
                /* Replace %n with the current process identifier.
                 */
                sprintf(b + i, "%lu", __mp_processid());
                i += strlen(b + i) - 1;
                s++;
                break;
              case 'p':
                /* Replace %p with the program name.
                 */
                if (p = m->prog)
#if TARGET == TARGET_UNIX
                    while (t = strchr(p, '/'))
#elif TARGET == TARGET_AMIGA
                    while (t = strpbrk(p, ":/"))
#elif TARGET == TARGET_WINDOWS || TARGET == TARGET_NETWARE
                    while (t = strpbrk(p, ":/\\"))
#endif /* TARGET */
                        p = t + 1;
                if ((p == NULL) || (*p == '\0'))
                    p = "mpatrol";
                strcpy(b + i, p);
                i += strlen(p) - 1;
                s++;
                break;
              case 't':
                /* Replace %t with the current time in the form HHMMSS.
                 */
                if (!currenttime)
                    currenttime = time(NULL);
                if (currenttime != (time_t) -1)
                    strftime(b + i, l - i, "%H%M%S", localtime(&currenttime));
                else
                    strcpy(b + i, "now");
                i += strlen(b + i) - 1;
                s++;
                break;
              default:
                if (s[1] != '\0')
                    b[i++] = *s++;
                b[i] = *s;
                break;
            }
        else
            b[i] = *s;
    b[i] = '\0';
}


/* Process the log file name, expanding any special characters.
 * Note that this function is not currently re-entrant.
 */

MP_GLOBAL
char *
__mp_logfile(meminfo *m, char *s)
{
    static char b[256];
    char p[256];
    char *d;

    if ((s != NULL) && ((strcmp(s, "stderr") == 0) ||
         (strcmp(s, "stdout") == 0)))
        return s;
    if ((d = getenv(MP_LOGDIR)) && (*d != '\0') && ((s == NULL) ||
#if TARGET == TARGET_UNIX
         !strchr(s, '/')))
#elif TARGET == TARGET_AMIGA
         !strpbrk(s, ":/")))
#elif TARGET == TARGET_WINDOWS || TARGET == TARGET_NETWARE
         !strpbrk(s, ":/\\")))
#endif /* TARGET */
    {
        /* If the environment variable specified with MP_LOGDIR is set and no
         * log file name has already been given then we use a special format
         * for the name of the output file so that all such files will be
         * written to that directory, which must exist.
         */
        if (s == NULL)
            s = "%n.%p.log";
#if TARGET == TARGET_UNIX
        sprintf(p, "%s/%s", d, s);
#elif TARGET == TARGET_AMIGA
        if ((d[strlen(d) - 1] == ':') || (d[strlen(d) - 1] == '/'))
            sprintf(p, "%s%s", d, s);
        else
            sprintf(p, "%s/%s", d, s);
#elif TARGET == TARGET_WINDOWS || TARGET == TARGET_NETWARE
        sprintf(p, "%s\\%s", d, s);
#endif /* TARGET */
        processfile(m, p, b, sizeof(b));
    }
    else
    {
        if (s == NULL)
            s = MP_LOGFILE;
        processfile(m, s, b, sizeof(b));
    }
    return b;
}


/* Process the profiling output file name, expanding any special characters.
 * Note that this function is not currently re-entrant.
 */

MP_GLOBAL
char *
__mp_proffile(meminfo *m, char *s)
{
    static char b[256];
    char p[256];
    char *d;

    if ((s != NULL) && ((strcmp(s, "stderr") == 0) ||
         (strcmp(s, "stdout") == 0)))
        return s;
    if ((d = getenv(MP_PROFDIR)) && (*d != '\0') && ((s == NULL) ||
#if TARGET == TARGET_UNIX
         !strchr(s, '/')))
#elif TARGET == TARGET_AMIGA
         !strpbrk(s, ":/")))
#elif TARGET == TARGET_WINDOWS || TARGET == TARGET_NETWARE
         !strpbrk(s, ":/\\")))
#endif /* TARGET */
    {
        /* If the environment variable specified with MP_PROFDIR is set and no
         * profiling output file name has already been given then we use a
         * special format for the name of the output file so that all such
         * files will be written to that directory, which must exist.
         */
        if (s == NULL)
            s = "%n.%p.out";
#if TARGET == TARGET_UNIX
        sprintf(p, "%s/%s", d, s);
#elif TARGET == TARGET_AMIGA
        if ((d[strlen(d) - 1] == ':') || (d[strlen(d) - 1] == '/'))
            sprintf(p, "%s%s", d, s);
        else
            sprintf(p, "%s/%s", d, s);
#elif TARGET == TARGET_WINDOWS || TARGET == TARGET_NETWARE
        sprintf(p, "%s\\%s", d, s);
#endif /* TARGET */
        processfile(m, p, b, sizeof(b));
    }
    else
    {
        if (s == NULL)
            s = MP_PROFFILE;
        processfile(m, s, b, sizeof(b));
    }
    return b;
}


/* Process the tracing output file name, expanding any special characters.
 * Note that this function is not currently re-entrant.
 */

MP_GLOBAL
char *
__mp_tracefile(meminfo *m, char *s)
{
    static char b[256];
    char p[256];
    char *d;

    if ((s != NULL) && ((strcmp(s, "stderr") == 0) ||
         (strcmp(s, "stdout") == 0)))
        return s;
    if ((d = getenv(MP_TRACEDIR)) && (*d != '\0') && ((s == NULL) ||
#if TARGET == TARGET_UNIX
         !strchr(s, '/')))
#elif TARGET == TARGET_AMIGA
         !strpbrk(s, ":/")))
#elif TARGET == TARGET_WINDOWS || TARGET == TARGET_NETWARE
         !strpbrk(s, ":/\\")))
#endif /* TARGET */
    {
        /* If the environment variable specified with MP_TRACEDIR is set and no
         * tracing output file name has already been given then we use a
         * special format for the name of the output file so that all such
         * files will be written to that directory, which must exist.
         */
        if (s == NULL)
            s = "%n.%p.trace";
#if TARGET == TARGET_UNIX
        sprintf(p, "%s/%s", d, s);
#elif TARGET == TARGET_AMIGA
        if ((d[strlen(d) - 1] == ':') || (d[strlen(d) - 1] == '/'))
            sprintf(p, "%s%s", d, s);
        else
            sprintf(p, "%s/%s", d, s);
#elif TARGET == TARGET_WINDOWS || TARGET == TARGET_NETWARE
        sprintf(p, "%s\\%s", d, s);
#endif /* TARGET */
        processfile(m, p, b, sizeof(b));
    }
    else
    {
        if (s == NULL)
            s = MP_TRACEFILE;
        processfile(m, s, b, sizeof(b));
    }
    return b;
}


/* Attempt to open the log file.
 */

MP_GLOBAL
int
__mp_openlogfile(char *s)
{
    /* The log file name can also be named as stderr and stdout which will go
     * to the standard error and standard output streams respectively.
     */
    if ((s == NULL) || (strcmp(s, "stderr") == 0))
        logfile = stderr;
    else if (strcmp(s, "stdout") == 0)
        logfile = stdout;
    else if ((logfile = fopen(s, "w")) == NULL)
    {
        /* Because logfile is NULL, the __mp_error() function will open the log
         * file as stderr, which should always work.
         */
        logfile = stderr;
        __mp_error(ET_MAX, AT_MAX, NULL, 0, "%s: cannot open file\n", s);
        return 0;
    }
    /* Attempt to set the stream buffer for the log file.  This is done here so
     * that we won't get recursive memory allocations if the standard library
     * tries to allocate space for the stream buffer.
     */
    if ((logfile == stderr) || setvbuf(logfile, buffer, _IOLBF, sizeof(buffer)))
        /* If that failed, or the log file is stderr, then we set the buffering
         * mode for the log file to none.  The standard error stream is not
         * guaranteed to be unbuffered by default on all systems.
         */
        setvbuf(logfile, NULL, _IONBF, 0);
    return 1;
}


/* Attempt to close the log file.
 */

MP_GLOBAL
int
__mp_closelogfile(void)
{
    int r;

    r = 1;
    if ((logfile == NULL) || (logfile == stderr) || (logfile == stdout))
    {
        /* We don't want to close the stderr or stdout file streams so
         * we just flush them instead.  If the log file hasn't been set,
         * this will just flush all open output files.
         */
        if (fflush(logfile))
            r = 0;
    }
    else if (fclose(logfile))
        r = 0;
    logfile = NULL;
    return r;
}


/* Sends a diagnostic message to the log file.
 */

MP_GLOBAL
void
__mp_diag(char *s, ...)
{
    va_list v;

    if (logfile == NULL)
        __mp_openlogfile(NULL);
    va_start(v, s);
    vfprintf(logfile, s, v);
    va_end(v);
}


/* Sends a warning message to the log file.
 */

MP_GLOBAL
void
__mp_warn(errortype e, alloctype f, char *n, unsigned long l, char *s, ...)
{
    va_list v;

    if (logfile == NULL)
        __mp_openlogfile(NULL);
    __mp_diag("WARNING: ");
    if (e != ET_MAX)
        __mp_diag("[%s]: ", errornames[e]);
    if (f != AT_MAX)
        __mp_diag("%s: ", __mp_functionnames[f]);
    va_start(v, s);
    vfprintf(logfile, s, v);
    va_end(v);
    __mp_diag("\n");
    if (((__mp_diagflags & FLG_EDIT) || (__mp_diagflags & FLG_LIST)) &&
        (n != NULL))
    {
        if (logfile != stderr)
        {
            fputs("WARNING: ", stderr);
            if (e != ET_MAX)
                fprintf(stderr, "[%s]: ", errornames[e]);
            if (f != AT_MAX)
                fprintf(stderr, "%s: ", __mp_functionnames[f]);
            va_start(v, s);
            vfprintf(stderr, s, v);
            va_end(v);
            fputc('\n', stderr);
        }
        if (__mp_editfile(n, l, ((__mp_diagflags & FLG_LIST) != 0)) == -1)
            fprintf(stderr, "ERROR: problems %sing file `%s'\n",
                    (__mp_diagflags & FLG_LIST) ? "list" : "edit", n);
    }
    warnings++;
}


/* Sends an error message to the log file.
 */

MP_GLOBAL
void
__mp_error(errortype e, alloctype f, char *n, unsigned long l, char *s, ...)
{
    va_list v;

    if (logfile == NULL)
        __mp_openlogfile(NULL);
    __mp_diag("ERROR: ");
    if (e != ET_MAX)
        __mp_diag("[%s]: ", errornames[e]);
    if (f != AT_MAX)
        __mp_diag("%s: ", __mp_functionnames[f]);
    va_start(v, s);
    vfprintf(logfile, s, v);
    va_end(v);
    __mp_diag("\n");
    if (((__mp_diagflags & FLG_EDIT) || (__mp_diagflags & FLG_LIST)) &&
        (n != NULL))
    {
        if (logfile != stderr)
        {
            fputs("ERROR: ", stderr);
            if (e != ET_MAX)
                fprintf(stderr, "[%s]: ", errornames[e]);
            if (f != AT_MAX)
                fprintf(stderr, "%s: ", __mp_functionnames[f]);
            va_start(v, s);
            vfprintf(stderr, s, v);
            va_end(v);
            fputc('\n', stderr);
        }
        if (__mp_editfile(n, l, ((__mp_diagflags & FLG_LIST) != 0)) == -1)
            fprintf(stderr, "ERROR: problems %sing file `%s'\n",
                    (__mp_diagflags & FLG_LIST) ? "list" : "edit", n);
    }
    errors++;
}


/* Invoke a text editor on a given source file at a specific line.
 */

MP_GLOBAL
int
__mp_editfile(char *f, unsigned long l, int d)
{
#if TARGET == TARGET_UNIX
#if MP_PRELOAD_SUPPORT
    char s[256];
#endif /* MP_PRELOAD_SUPPORT */
    char t[32];
    char *v[5];
    pid_t p;
    int r;
#endif /* TARGET */

#if TARGET == TARGET_UNIX
#if MP_PRELOAD_SUPPORT
    sprintf(s, "%s=", MP_PRELOAD_NAME);
#endif /* MP_PRELOAD_SUPPORT */
    sprintf(t, "%lu", l);
    if ((p = fork()) < 0)
        return -1;
    if (p == 0)
    {
#if MP_PRELOAD_SUPPORT
        /* We have to ensure that we don't end up debugging the editor and its
         * child processes as well!  Hopefully, if we ensure that the relevant
         * environment variable is set then putenv() will not use malloc() to
         * expand the environment.
         */
        if (getenv(MP_PRELOAD_NAME))
            putenv(s);
#endif /* MP_PRELOAD_SUPPORT */
        v[0] = MP_EDITOR;
        if (d == 0)
        {
            v[1] = f;
            v[2] = t;
            v[3] = NULL;
        }
        else
        {
            v[1] = "--listing";
            v[2] = f;
            v[3] = t;
            v[4] = NULL;
        }
        execvp(v[0], v);
        _exit(EXIT_FAILURE);
    }
    while (waitpid(p, &r, 0) < 0)
        if (errno != EINTR)
            return -1;
    if (!WIFEXITED(r) || (WEXITSTATUS(r) != 0))
        return -1;
    return 1;
#else /* TARGET */
    return 0;
#endif /* TARGET */
}


/* Display up to sixteen bytes of memory in one line.
 */

static
void
printline(char *s, size_t l)
{
    size_t i;

    __mp_diag("\t" MP_POINTER "  ", s);
    for (i = 0; i < 16; i++)
    {
        if (i < l)
            __mp_diag("%02X", (unsigned char) s[i]);
        else
            __mp_diag("  ");
        if (i % 4 == 3)
            __mp_diag(" ");
    }
    __mp_diag(" ");
    for (i = 0; i < l; i++)
        if (isprint(s[i]))
            __mp_diag("%c", s[i]);
        else
            __mp_diag(".");
    __mp_diag("\n");
}


/* Display a hex dump of a specified memory location and length.
 */

MP_GLOBAL
void
__mp_printmemory(void *p, size_t l)
{
    while (l >= 16)
    {
        printline((char *) p, 16);
        p = (char *) p + 16;
        l -= 16;
    }
    if (l > 0)
        printline((char *) p, l);
}


/* Display a size in bytes.
 */

MP_GLOBAL
void
__mp_printsize(size_t l)
{
    __mp_diag("%lu byte", l);
    if (l != 1)
        __mp_diag("s");
}


/* Display the type of memory allocation along with the allocation count
 * and reallocation count.
 */

MP_GLOBAL
void
__mp_printtype(infonode *n)
{
    __mp_diag("{%s:%lu:%lu}", __mp_functionnames[n->data.type], n->data.alloc,
              n->data.realloc);
}


/* Display the function name, file name and line number where a memory
 * allocation took place.  Also display the thread identifier if the thread
 * safe library is being used.
 */

MP_GLOBAL
void
__mp_printloc(infonode *n)
{
    __mp_diag("[");
#if MP_THREADS_SUPPORT
    __mp_diag("%lu|", n->data.thread);
#endif /* MP_THREADS_SUPPORT */
    if (n->data.func)
        __mp_diag("%s", n->data.func);
    else
        __mp_diag("-");
    __mp_diag("|");
    if (n->data.file)
        __mp_diag("%s", n->data.file);
    else
        __mp_diag("-");
    __mp_diag("|");
    if (n->data.line)
        __mp_diag("%lu", n->data.line);
    else
        __mp_diag("-");
    __mp_diag("]");
}


/* Display the type of data that is stored in the memory allocation along
 * with the number of objects stored.
 */

MP_GLOBAL
void
__mp_printtypeinfo(infonode *n, size_t s)
{
    __mp_diag("(%s", n->data.typestr);
    if ((s /= n->data.typesize) > 1)
        __mp_diag(" x %lu", s);
    __mp_diag(")");
}


/* Display the name of a symbol associated with a particular address.
 */

MP_GLOBAL
void
__mp_printsymbol(symhead *y, void *a)
{
    symnode *n;
    char *s, *t;
    unsigned long u;

    __mp_findsource(y, (char *) a - 1, &s, &t, &u);
    if (n = __mp_findsymbol(y, a))
    {
        __mp_diag("%s", n->data.name);
        if (a != n->data.addr)
            __mp_diag("%+ld", (char *) a - (char *) n->data.addr);
    }
    else if (s != NULL)
        __mp_diag("%s", s);
    else
        __mp_diag("???");
    if ((t != NULL) && (u != 0))
        __mp_diag(" at %s:%lu", t, u);
}


/* Display details of all of the symbols that have been read.
 */

MP_GLOBAL
void
__mp_printsymbols(symhead *y)
{
    symnode *n;

    __mp_diag("\nsymbols read: %lu\n", y->dtree.size);
    for (n = (symnode *) __mp_minimum(y->dtree.root); n != NULL;
         n = (symnode *) __mp_successor(&n->data.node))
    {
        if (n->data.size == 0)
        {
#if ENVIRON == ENVIRON_64
            __mp_diag("\t");
#endif /* ENVIRON */
            __mp_diag("\t       " MP_POINTER, n->data.addr);
        }
        else
            __mp_diag("    " MP_POINTER "-" MP_POINTER, n->data.addr,
                      (char *) n->data.addr + n->data.size - 1);
        __mp_diag(" %s [%s] (", n->data.name, n->data.file);
        __mp_printsize(n->data.size);
        __mp_diag(")\n");
    }
}


/* Display the call stack details for an address node.
 */

MP_GLOBAL
void
__mp_printaddrs(symhead *y, addrnode *n)
{
    while (n != NULL)
    {
        __mp_diag("\t" MP_POINTER " ", n->data.addr);
        if (n->data.name == NULL)
            __mp_printsymbol(y, n->data.addr);
        else
            __mp_diag("%s", n->data.name);
        __mp_diag("\n");
        n = n->data.next;
    }
}


/* Display the call stack details for a call stack handle.
 */

MP_GLOBAL
void
__mp_printstack(symhead *y, stackinfo *p)
{
    stackinfo s;

    s = *p;
    if ((p->frame != NULL) && (p->addr != NULL))
    {
        __mp_diag("\t" MP_POINTER " ", p->addr);
        __mp_printsymbol(y, p->addr);
        __mp_diag("\n");
        while (__mp_getframe(p) && (p->addr != NULL))
        {
            __mp_diag("\t" MP_POINTER " ", p->addr);
            __mp_printsymbol(y, p->addr);
            __mp_diag("\n");
        }
    }
    *p = s;
}


/* Display the details of an individual allocation node.
 */

MP_GLOBAL
void
__mp_printalloc(symhead *y, allocnode *n)
{
    infonode *m;

    m = (infonode *) n->info;
    __mp_diag("    " MP_POINTER " (", n->block);
    __mp_printsize(n->size);
    __mp_diag(") ");
    __mp_printtype(m);
    __mp_diag(" ");
    __mp_printloc(m);
    if ((m->data.typestr != NULL) && (m->data.typesize != 0))
    {
        __mp_diag(" ");
        __mp_printtypeinfo(m, n->size);
    }
    __mp_diag("\n");
    __mp_printaddrs(y, m->data.stack);
}


/* Log the details of where a function call came from.
 */

static
void
logcall(infohead *h, loginfo *i, size_t s)
{
    __mp_diag("[");
#if MP_THREADS_SUPPORT
    __mp_diag("%lu|", __mp_threadid());
#endif /* MP_THREADS_SUPPORT */
    __mp_diag("%s|%s|", (i->func ? i->func : "-"), (i->file ? i->file : "-"));
    if (i->line == 0)
        __mp_diag("-");
    else
        __mp_diag("%lu", i->line);
    __mp_diag("]");
    if ((i->typestr != NULL) && (i->typesize != 0))
    {
        __mp_diag(" (%s", i->typestr);
        if ((s /= i->typesize) > 1)
            __mp_diag(" x %lu", s);
        __mp_diag(")");
    }
    __mp_diag("\n");
    __mp_printstack(&h->syms, i->stack);
    __mp_diag("\n");
}


/* Log the details of a call to allocate memory.
 */

MP_GLOBAL
void
__mp_logalloc(infohead *h, size_t l, size_t a, alloctype f, loginfo *i)
{
    __mp_diag("ALLOC: %s (%lu, ", __mp_functionnames[f], h->count);
    __mp_printsize(l);
    __mp_diag(", ");
    if (a == 0)
        __mp_printsize(h->alloc.heap.memory.align);
    else
        __mp_printsize(a);
    __mp_diag(") ");
    logcall(h, i, l);
}


/* Log the details of a call to reallocate memory.
 */

MP_GLOBAL
void
__mp_logrealloc(infohead *h, void *p, size_t l, size_t a, alloctype f,
                loginfo *i)
{
    __mp_diag("REALLOC: %s (" MP_POINTER ", ", __mp_functionnames[f], p);
    __mp_printsize(l);
    __mp_diag(", ");
    if (a == 0)
        __mp_printsize(h->alloc.heap.memory.align);
    else
        __mp_printsize(a);
    __mp_diag(") ");
    logcall(h, i, l);
}


/* Log the details of a call to deallocate memory.
 */

MP_GLOBAL
void
__mp_logfree(infohead *h, void *p, alloctype f, loginfo *i)
{
    __mp_diag("FREE: %s (" MP_POINTER ") ", __mp_functionnames[f], p);
    logcall(h, i, 0);
}


/* Log the details of a call to set memory.
 */

MP_GLOBAL
void
__mp_logmemset(infohead *h, void *p, size_t l, unsigned char c, alloctype f,
               loginfo *i)
{
    __mp_diag("MEMSET: %s (" MP_POINTER ", ", __mp_functionnames[f], p);
    __mp_printsize(l);
    __mp_diag(", 0x%02X) ", c);
    logcall(h, i, 0);
}


/* Log the details of a call to copy memory.
 */

MP_GLOBAL
void
__mp_logmemcopy(infohead *h, void *p, void *q, size_t l, unsigned char c,
                alloctype f, loginfo *i)
{
    __mp_diag("MEMCOPY: %s (" MP_POINTER ", " MP_POINTER ", ",
              __mp_functionnames[f], p, q);
    __mp_printsize(l);
    __mp_diag(", 0x%02X) ", c);
    logcall(h, i, 0);
}


/* Log the details of a call to locate memory.
 */

MP_GLOBAL
void
__mp_logmemlocate(infohead *h, void *p, size_t l, void *q, size_t m,
                  alloctype f, loginfo *i)
{
    __mp_diag("MEMFIND: %s (" MP_POINTER ", ", __mp_functionnames[f], p);
    __mp_printsize(l);
    __mp_diag(", " MP_POINTER ", ", q);
    __mp_printsize(m);
    __mp_diag(") ");
    logcall(h, i, 0);
}


/* Log the details of a call to compare memory.
 */

MP_GLOBAL
void
__mp_logmemcompare(infohead *h, void *p, void *q, size_t l, alloctype f,
                   loginfo *i)
{
    __mp_diag("MEMCMP: %s (" MP_POINTER ", " MP_POINTER ", ",
              __mp_functionnames[f], p, q);
    __mp_printsize(l);
    __mp_diag(") ");
    logcall(h, i, 0);
}


/* Display the details of all allocation nodes on the allocation tree.
 */

MP_GLOBAL
void
__mp_printallocs(infohead *h, int e)
{
    allocnode *n;
    treenode *t;
    char f;

    f = 0;
    if (e != 0)
    {
        /* We are now displaying the allocation nodes for a second
         * time, except this time to the standard error stream.  If
         * the log file is already on standard error then don't bother
         * displaying them again.
         */
        if (logfile == stderr)
        {
            h->fini = 1;
            __mp_abort();
        }
        __mp_closelogfile();
        __mp_diag("\nALLOC:");
        if (h->alloc.heap.memory.prog != NULL)
            __mp_diag(" %s:", h->alloc.heap.memory.prog);
        __mp_diag("\n");
    }
    __mp_diag("\nunfreed allocations: %lu (", h->alloc.atree.size);
    __mp_printsize(h->alloc.asize);
    __mp_diag(")\n");
    for (t = __mp_minimum(h->alloc.atree.root); t != NULL;
         t = __mp_successor(t))
    {
        n = (allocnode *) ((char *) t - offsetof(allocnode, tnode));
        if (f == 0)
            f = 1;
        else
            __mp_diag("\n");
        __mp_printalloc(&h->syms, n);
    }
    if (e != 0)
    {
        h->fini = 1;
        __mp_abort();
    }
}


/* Display the details of all allocation nodes on the freed tree.
 */

MP_GLOBAL
void
__mp_printfreed(infohead *h)
{
    allocnode *n;
    treenode *t;
    char f;

    f = 0;
    __mp_diag("\nfreed allocations: %lu (", h->alloc.gtree.size);
    __mp_printsize(h->alloc.gsize);
    __mp_diag(")\n");
    for (t = __mp_minimum(h->alloc.gtree.root); t != NULL;
         t = __mp_successor(t))
    {
        n = (allocnode *) ((char *) t - offsetof(allocnode, tnode));
        if (f == 0)
            f = 1;
        else
            __mp_diag("\n");
        __mp_printalloc(&h->syms, n);
    }
}


/* Display the details of all free blocks.
 */

MP_GLOBAL
void
__mp_printfree(infohead *h)
{
    allocnode *n, *p;
    treenode *s, *t;
    size_t c;

    __mp_diag("\nfree blocks: %lu (", h->alloc.ftree.size);
    __mp_printsize(h->alloc.fsize);
    __mp_diag(")\n");
    for (t = __mp_maximum(h->alloc.ftree.root); t != NULL; t = s)
    {
        n = (allocnode *) ((char *) t - offsetof(allocnode, tnode));
        s = t;
        c = 0;
        do
        {
            if (s = __mp_predecessor(s))
                p = (allocnode *) ((char *) s - offsetof(allocnode, tnode));
            else
                p = NULL;
            c++;
        }
        while ((p != NULL) && (p->size == n->size));
        __mp_diag("   %8lu: %lu\n", n->size, c);
    }
}


/* Display a complete memory map of the heap.
 */

MP_GLOBAL
void
__mp_printmap(infohead *h)
{
    allocnode *n;
    infonode *m;
    void *a, *b;
    size_t l, s;

    a = NULL;
    __mp_diag("\nmemory map:\n");
    for (n = (allocnode *) h->alloc.list.head; n->lnode.next != NULL;
         n = (allocnode *) n->lnode.next)
    {
        m = (infonode *) n->info;
        if ((h->alloc.flags & FLG_PAGEALLOC) && (m != NULL))
        {
            b = (void *) __mp_rounddown((unsigned long) n->block,
                                        h->alloc.heap.memory.page);
            l = __mp_roundup(n->size + ((char *) n->block - (char *) b),
                             h->alloc.heap.memory.page);
        }
        else
        {
            b = n->block;
            l = n->size;
        }
        if (m != NULL)
        {
            b = (char *) b - h->alloc.oflow;
            l += h->alloc.oflow << 1;
        }
        if ((a != NULL) && (a < b))
        {
#if ENVIRON == ENVIRON_64
            __mp_diag("    ------------------------------------- gap (");
#else /* ENVIRON */
            __mp_diag("    --------------------- gap (");
#endif /* ENVIRON */
            __mp_printsize((char *) b - (char *) a);
            __mp_diag(")\n");
        }
        if (m != NULL)
            if (h->alloc.oflow > 0)
            {
                s = (char *) n->block - (char *) b;
                __mp_diag("  / " MP_POINTER "-" MP_POINTER " overflow (", b,
                          (char *) b + s - 1);
                __mp_printsize(s);
                __mp_diag(")\n |+ ");
            }
            else
                __mp_diag("  + ");
        else
            __mp_diag("--- ");
        __mp_diag(MP_POINTER "-" MP_POINTER, n->block,
                  (char *) n->block + n->size - 1);
        if (m == NULL)
            __mp_diag(" free (");
        else if (m->data.flags & FLG_FREED)
            __mp_diag(" freed (");
        else
            __mp_diag(" allocated (");
        __mp_printsize(n->size);
        __mp_diag(")");
        if (m != NULL)
        {
            __mp_diag(" ");
            __mp_printtype(m);
            __mp_diag(" ");
            __mp_printloc(m);
            if ((m->data.typestr != NULL) && (m->data.typesize != 0))
            {
                __mp_diag(" ");
                __mp_printtypeinfo(m, n->size);
            }
            if (h->alloc.oflow > 0)
            {
                s = l - n->size - s;
                __mp_diag("\n  \\ " MP_POINTER "-" MP_POINTER " overflow (",
                          (char *) n->block + n->size, (char *) b + l - 1);
                __mp_printsize(s);
                __mp_diag(")");
            }
        }
        __mp_diag("\n");
        a = (char *) b + l;
    }
}


/* Display the version and copyright details.
 */

MP_GLOBAL
void
__mp_printversion(void)
{
    __mp_diag("%s\n%s\n\n", __mp_version, __mp_copyright);
    __mp_diag("This is free software, and you are welcome to redistribute it "
              "under certain\n");
    __mp_diag("conditions; see the GNU Library General Public License for "
              "details.\n\n");
    __mp_diag("For the latest mpatrol release and documentation,\n");
    __mp_diag("visit %s.\n\n", __mp_homepage);
    if (!currenttime)
        currenttime = time(NULL);
    if (currenttime != (time_t) -1)
        __mp_diag("Log file generated on %s\n", ctime(&currenttime));
}


/* Display a summary of all mpatrol library settings and statistics.
 */

MP_GLOBAL
void
__mp_printsummary(infohead *h)
{
    size_t n;

    __mp_diag("system page size:  ");
    __mp_printsize(h->alloc.heap.memory.page);
    __mp_diag("\ndefault alignment: ");
    __mp_printsize(h->alloc.heap.memory.align);
    __mp_diag("\noverflow size:     ");
    __mp_printsize(h->alloc.oflow);
    __mp_diag("\noverflow byte:     0x%02X", h->alloc.obyte);
    __mp_diag("\nallocation byte:   0x%02X", h->alloc.abyte);
    __mp_diag("\nfree byte:         0x%02X", h->alloc.fbyte);
    __mp_diag("\nallocation stop:   %lu", h->astop);
    __mp_diag("\nreallocation stop: %lu", h->rstop);
    __mp_diag("\nfree stop:         %lu", h->fstop);
    __mp_diag("\nunfreed abort:     %lu", h->uabort);
    __mp_diag("\nsmall boundary:    ");
    __mp_printsize(h->prof.sbound);
    __mp_diag("\nmedium boundary:   ");
    __mp_printsize(h->prof.mbound);
    __mp_diag("\nlarge boundary:    ");
    __mp_printsize(h->prof.lbound);
    __mp_diag("\nlower check range: ");
    if (h->lrange == (size_t) -1)
        __mp_diag("-");
    else
        __mp_diag("%lu", h->lrange);
    __mp_diag("\nupper check range: ");
    if (h->urange == (size_t) -1)
        __mp_diag("-");
    else
        __mp_diag("%lu", h->urange);
    __mp_diag("\ncheck frequency:   %lu", h->check);
    __mp_diag("\nfailure frequency: %lu", h->ffreq);
    __mp_diag("\nfailure seed:      %lu", h->fseed);
    __mp_diag("\nprologue function: ");
    if (h->prologue == NULL)
        __mp_diag("<unset>");
    else
    {
        __mp_diag(MP_POINTER " [", h->prologue);
        __mp_printsymbol(&h->syms, (void *) h->prologue);
        __mp_diag("]");
    }
    __mp_diag("\nepilogue function: ");
    if (h->epilogue == NULL)
        __mp_diag("<unset>");
    else
    {
        __mp_diag(MP_POINTER " [", h->epilogue);
        __mp_printsymbol(&h->syms, (void *) h->epilogue);
        __mp_diag("]");
    }
    __mp_diag("\nhandler function:  ");
    if (h->nomemory == NULL)
        __mp_diag("<unset>");
    else
    {
        __mp_diag(MP_POINTER " [", h->nomemory);
        __mp_printsymbol(&h->syms, (void *) h->nomemory);
        __mp_diag("]");
    }
    __mp_diag("\nlog file:          ");
    if (h->log == NULL)
        __mp_diag("<unset>");
    else
        __mp_diag("%s", h->log);
    __mp_diag("\nprofiling file:    ");
    if (h->prof.file == NULL)
        __mp_diag("<unset>");
    else
        __mp_diag("%s", h->prof.file);
    __mp_diag("\ntracing file:      ");
    if (h->trace.file == NULL)
        __mp_diag("<unset>");
    else
        __mp_diag("%s", h->trace.file);
    __mp_diag("\nprogram filename:  ");
    if (h->alloc.heap.memory.prog == NULL)
        __mp_diag("<not found>");
    else
        __mp_diag("%s", h->alloc.heap.memory.prog);
    __mp_diag("\nsymbols read:      %lu", h->syms.dtree.size);
    __mp_diag("\nautosave count:    %lu", h->prof.autosave);
    __mp_diag("\nfreed queue size:  %lu", h->alloc.fmax);
    __mp_diag("\nallocation count:  %lu", h->count);
    __mp_diag("\nallocation peak:   %lu (", h->cpeak);
    __mp_printsize(h->peak);
    __mp_diag(")\nallocation limit:  ");
    __mp_printsize(h->limit);
    __mp_diag("\nallocated blocks:  %lu (", h->alloc.atree.size);
    __mp_printsize(h->alloc.asize);
    __mp_diag(")\nfreed blocks:      %lu (", h->alloc.gtree.size);
    __mp_printsize(h->alloc.gsize);
    __mp_diag(")\nfree blocks:       %lu (", h->alloc.ftree.size);
    __mp_printsize(h->alloc.fsize);
    n = h->alloc.heap.itree.size + h->alloc.itree.size + h->addr.list.size +
        h->syms.strings.list.size + h->syms.strings.tree.size +
        h->syms.itree.size + h->prof.ilist.size + h->list.size + h->alist.size;
    __mp_diag(")\ninternal blocks:   %lu (", n);
    n = h->alloc.heap.isize + h->alloc.isize + h->addr.size +
        h->syms.strings.size + h->syms.size + h->prof.size + h->size;
    __mp_printsize(n);
    __mp_diag(")\ntotal heap usage:  ");
    n = h->alloc.heap.isize + h->alloc.heap.dsize;
    __mp_printsize(n);
    __mp_diag("\ntotal compared:    ");
    __mp_printsize(h->dtotal);
    __mp_diag("\ntotal located:     ");
    __mp_printsize(h->ltotal);
    __mp_diag("\ntotal copied:      ");
    __mp_printsize(h->ctotal);
    __mp_diag("\ntotal set:         ");
    __mp_printsize(h->stotal);
    __mp_diag("\ntotal warnings:    %lu", warnings);
    __mp_diag("\ntotal errors:      %lu\n", errors);
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
