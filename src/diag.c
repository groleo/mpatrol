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
 * Diagnostics.  Most of the diagnostics go to a log file which is opened
 * as soon as possible, but if this fails then the standard error is used.
 * Many of these functions also deal with the formatting and displaying of
 * certain data structures used by the mpatrol library.
 */


#include "diag.h"
#include "utils.h"
#include "version.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>


#if MP_IDENT_SUPPORT
#ident "$Id: diag.c,v 1.1.1.1 1999-10-03 11:25:21 graeme Exp $"
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


/* This array should always be kept in step with the alloctype enumeration.
 * Note that AT_MAX is used in diagnostic messages to specify that the
 * message is internal and did not come from a call to a normal memory
 * allocation function.
 */

MP_GLOBAL char *__mp_alloctypenames[AT_MAX] =
{
    "malloc",
    "calloc",
    "memalign",
    "valloc",
    "pvalloc",
    "strdup",
    "strndup",
    "strsave",
    "strnsave",
    "realloc",
    "recalloc",
    "free",
    "cfree",
    "operator new",
    "operator new[]",
    "operator delete",
    "operator delete[]"
};


/* Process the log file name, expanding any special characters.
 * Note that this function is not currently re-entrant.
 */

MP_GLOBAL char *__mp_logfile(char *s)
{
    static char b[256];
    size_t i;

    if (s == NULL)
        s = MP_LOGFILE;
    for (i = 0; (i < sizeof(b) - 1) && (*s != '\0'); i++, s++)
        if (*s == '%')
            switch (s[1])
            {
              case 'n':
                sprintf(b + i, "%lu", __mp_processid());
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
    return b;
}


/* Attempt to open the log file.
 */

MP_GLOBAL int __mp_openlogfile(char *s)
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
        __mp_error(AT_MAX, "%s: cannot open file\n", s);
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

MP_GLOBAL int __mp_closelogfile(void)
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

MP_GLOBAL void __mp_diag(char *s, ...)
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

MP_GLOBAL void __mp_warn(alloctype f, char *s, ...)
{
    va_list v;

    if (logfile == NULL)
        __mp_openlogfile(NULL);
    __mp_diag("WARNING: ");
    if (f != AT_MAX)
        __mp_diag("%s: ", __mp_alloctypenames[f]);
    va_start(v, s);
    vfprintf(logfile, s, v);
    va_end(v);
    __mp_diag("\n");
}


/* Sends an error message to the log file.
 */

MP_GLOBAL void __mp_error(alloctype f, char *s, ...)
{
    va_list v;

    if (logfile == NULL)
        __mp_openlogfile(NULL);
    __mp_diag("ERROR: ");
    if (f != AT_MAX)
        __mp_diag("%s: ", __mp_alloctypenames[f]);
    va_start(v, s);
    vfprintf(logfile, s, v);
    va_end(v);
    __mp_diag("\n");
}


/* Display up to sixteen bytes of memory in one line.
 */

static void printline(char *s, size_t l)
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

MP_GLOBAL void __mp_printmemory(void *p, size_t l)
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

MP_GLOBAL void __mp_printsize(size_t l)
{
    __mp_diag("%lu byte", l);
    if (l != 1)
        __mp_diag("s");
}


/* Display the type of memory allocation along with the allocation count
 * and reallocation count.
 */

MP_GLOBAL void __mp_printtype(infonode *n)
{
    __mp_diag("{%s:%lu:%lu}", __mp_alloctypenames[n->data.type],
              n->data.alloc, n->data.realloc);
}


/* Display the function name, file name and line number where a memory
 * allocation took place.  Also display the thread identifier if the thread
 * safe library is being used.
 */

MP_GLOBAL void __mp_printloc(infonode *n)
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


/* Display the name of a symbol associated with a particular address.
 */

MP_GLOBAL void __mp_printsymbol(symhead *y, void *a)
{
    symnode *n;

    if (n = __mp_findsymbol(y, a))
        __mp_diag("%s", n->data.name);
    else
        __mp_diag("???");
}


/* Display details of all of the symbols that have been read.
 */

MP_GLOBAL void __mp_printsymbols(symhead *y)
{
    symnode *n;

    __mp_diag("\nsymbols read: %lu\n", y->dtree.size);
    for (n = (symnode *) __mp_minimum(y->dtree.root); n != NULL;
         n = (symnode *) __mp_successor(&n->data.node))
    {
        if (n->data.size == 0)
            __mp_diag("\t       " MP_POINTER, n->data.addr);
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

MP_GLOBAL void __mp_printaddrs(symhead *y, addrnode *n)
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

MP_GLOBAL void __mp_printstack(symhead *y, stackinfo *p)
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

MP_GLOBAL void __mp_printalloc(symhead *y, allocnode *n)
{
    infonode *m;

    m = (infonode *) n->info;
    __mp_diag("    " MP_POINTER " (", n->block);
    __mp_printsize(n->size);
    __mp_diag(") ");
    __mp_printtype(m);
    __mp_diag(" ");
    __mp_printloc(m);
    __mp_diag("\n");
    __mp_printaddrs(y, m->data.stack);
}


/* Display the details of all allocation nodes on the allocation tree.
 */

MP_GLOBAL void __mp_printallocs(infohead *h, int e)
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

MP_GLOBAL void __mp_printfreed(infohead *h)
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


/* Display a complete memory map of the heap.
 */

MP_GLOBAL void __mp_printmap(infohead *h)
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
            __mp_diag("    --------------------- gap (");
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
        else if (m->data.freed == 1)
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

MP_GLOBAL void __mp_printversion(void)
{
    __mp_diag("%s\n%s\n\n", __mp_version, __mp_copyright);
    __mp_diag("This is free software, and you are welcome to redistribute it "
              "under certain\n");
    __mp_diag("conditions; see the GNU Library General Public License for "
              "details.\n\n");
}


/* Display a summary of all mpatrol library settings and statistics.
 */

MP_GLOBAL void __mp_printsummary(infohead *h)
{
    size_t n;

    __mp_diag("system page size:  ");
    __mp_printsize(h->alloc.heap.memory.page);
    __mp_diag("\ndefault alignment: ");
    __mp_printsize(h->alloc.heap.memory.align);
    __mp_diag("\noverflow size:     ");
    __mp_printsize(h->alloc.oflow);
    __mp_diag("\noverflow byte:     0x%02lX", h->alloc.obyte);
    __mp_diag("\nallocation byte:   0x%02lX", h->alloc.abyte);
    __mp_diag("\nfree byte:         0x%02lX", h->alloc.fbyte);
    __mp_diag("\nallocation stop:   %lu", h->astop);
    __mp_diag("\nreallocation stop: %lu", h->rstop);
    __mp_diag("\nfree stop:         %lu", h->fstop);
    __mp_diag("\nunfreed abort:     %lu", h->uabort);
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
    __mp_diag("\nprogram filename:  ");
    if (h->alloc.heap.memory.prog == NULL)
        __mp_diag("<not found>");
    else
        __mp_diag("%s", h->alloc.heap.memory.prog);
    __mp_diag("\nsymbols read:      %lu", h->syms.dtree.size);
    __mp_diag("\nallocation count:  %lu", h->count);
    __mp_diag("\nallocation peak:   ");
    __mp_printsize(h->peak);
    __mp_diag("\nallocation limit:  ");
    __mp_printsize(h->limit);
    __mp_diag("\nallocated blocks:  %lu (", h->alloc.atree.size);
    __mp_printsize(h->alloc.asize);
    __mp_diag(")\nfreed blocks:      %lu (", h->alloc.gtree.size);
    __mp_printsize(h->alloc.gsize);
    __mp_diag(")\nfree blocks:       %lu (", h->alloc.ftree.size);
    __mp_printsize(h->alloc.fsize);
    n = h->alloc.heap.itree.size + h->alloc.itree.size + h->addr.list.size +
        h->syms.strings.tree.size + h->syms.itree.size + h->list.size;
    __mp_diag(")\ninternal blocks:   %lu (", n);
    n = h->alloc.heap.isize + h->alloc.isize + h->addr.size +
        h->syms.strings.size + h->syms.size + h->size;
    __mp_printsize(n);
    __mp_diag(")\ntotal heap usage:  ");
    n = h->alloc.heap.isize + h->alloc.heap.dsize;
    __mp_printsize(n);
    __mp_diag("\n");
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
