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
 * A tool designed to read a tracing output file produced by the mpatrol
 * library and display the tracing information that was obtained.
 */


#include "tree.h"
#include "getopt.h"
#include "utils.h"
#include "version.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if MP_GUI_SUPPORT
#include <Xm/DrawingA.h>
#include <Xm/ScrolledW.h>
#endif /* MP_GUI_SUPPORT */


#if MP_IDENT_SUPPORT
#ident "$Id: mptrace.c,v 1.11 2001-01-02 23:06:09 graeme Exp $"
#endif /* MP_IDENT_SUPPORT */


#define VERSION "1.0" /* the current version of this program */


/* The flags used to parse the command line options.
 */

typedef enum options_flags
{
    OF_HELP    = 'h',
    OF_VERSION = 'V'
}
options_flags;


/* Structure containing the allocation details for a single memory allocation.
 */

typedef struct allocation
{
    treenode node;       /* tree node */
    unsigned long event; /* event number */
    void *addr;          /* allocation address */
    size_t size;         /* allocation size */
    unsigned long time;  /* allocation lifetime */
}
allocation;


/* The version of the mpatrol library which produced the tracing output file.
 */

static unsigned long version;


/* The tree containing information about each memory allocation.
 */

static treeroot alloctree;


/* The number of the current event in the tracing output file.
 */

static unsigned long currentevent;


/* The input buffer used to read from the tracing output file, and also the
 * current buffer position and length.
 */

static char buffer[MP_BUFFER_SIZE];
static char *bufferpos;
static size_t bufferlen;


/* The tracing output file produced by mpatrol.
 */

static FILE *tracefile;


/* The filename used to invoke this tool.
 */

static char *progname;


#if MP_GUI_SUPPORT
/* The X toolkit context for this application.
 */

static XtAppContext appcontext;


/* The X display and screen pointers for this application.
 */

static Display *appdisplay;
static Screen *appscreen;


/* The X widgets for the application shell, main application and drawing area.
 */

static Widget appwidget, mainwidget, drawwidget;


/* The X pixmap for the backup drawing area.
 */

static Pixmap pixmap;


/* The X graphics contexts for unallocated, internal, free and allocated memory.
 */

static GC ungc, ingc, frgc, algc;


/* The colours for unallocated, internal, free and allocated memory.
 */

static Pixel uncol, incol, frcol, alcol;


/* The width and height (in pixels) of the window and the drawing area.
 */

static Dimension vwidth, vheight, width, height;


/* The delay between drawing each event.
 */

static unsigned long delay;


/* The base address and the size (in megabytes) of the address space covered by
 * the drawing area.
 */

static void *addrbase;
static unsigned long addrspace;


/* The scaling factor of the drawing area.
 */

static unsigned long addrscale;


/* The X resources for this application.
 */

static XtResource resources[] =
{
    {"alloc", XmCColor, XmRPixel, sizeof(Pixel),
     (Cardinal) &alcol, XmRString, (XtPointer) "black"},
    {"base", "Base", XmRInt, sizeof(void *),
     (Cardinal) &addrbase, XmRImmediate, (XtPointer) NULL},
    {"delay", "Delay", XmRInt, sizeof(unsigned long),
     (Cardinal) &delay, XmRImmediate, (XtPointer) 0},
    {"free", XmCColor, XmRPixel, sizeof(Pixel),
     (Cardinal) &frcol, XmRString, (XtPointer) "white"},
    {"height", XmCHeight, XmRShort, sizeof(Dimension),
     (Cardinal) &height, XmRImmediate, (XtPointer) 512},
    {"internal", XmCColor, XmRPixel, sizeof(Pixel),
     (Cardinal) &incol, XmRString, (XtPointer) "red"},
    {"space", "Space", XmRInt, sizeof(unsigned long),
     (Cardinal) &addrspace, XmRImmediate, (XtPointer) 4},
    {"unalloc", XmCColor, XmRPixel, sizeof(Pixel),
     (Cardinal) &uncol, XmRString, (XtPointer) "blue"},
    {"view-height", XmCHeight, XmRShort, sizeof(Dimension),
     (Cardinal) &vheight, XmRImmediate, (XtPointer) 256},
    {"view-width", XmCWidth, XmRShort, sizeof(Dimension),
     (Cardinal) &vwidth, XmRImmediate, (XtPointer) 256},
    {"width", XmCWidth, XmRShort, sizeof(Dimension),
     (Cardinal) &width, XmRImmediate, (XtPointer) 512}
};


/* The X options for this application.
 */

static XrmOptionDescRec options[] =
{
    {"--alloc", "alloc", XrmoptionSepArg, NULL},
    {"--base", "base", XrmoptionSepArg, NULL},
    {"--delay", "delay", XrmoptionSepArg, NULL},
    {"--free", "free", XrmoptionSepArg, NULL},
    {"--height", "height", XrmoptionSepArg, NULL},
    {"--internal", "internal", XrmoptionSepArg, NULL},
    {"--space", "space", XrmoptionSepArg, NULL},
    {"--unalloc", "unalloc", XrmoptionSepArg, NULL},
    {"--view-height", "view-height", XrmoptionSepArg, NULL},
    {"--view-width", "view-width", XrmoptionSepArg, NULL},
    {"--width", "width", XrmoptionSepArg, NULL}
};
#endif /* MP_GUI_SUPPORT */


/* The table describing all recognised options.
 */

static option options_table[] =
{
    {"help", OF_HELP, NULL,
     "\tDisplays this quick-reference option summary.\n"},
    {"version", OF_VERSION, NULL,
     "\tDisplays the version number of this program.\n"},
    NULL
};


/* Create a new memory allocation.
 */

static
void
newalloc(unsigned long i, unsigned long e, void *a, size_t l)
{
    allocation *n;

    if (n = (allocation *) __mp_search(alloctree.root, i))
    {
        if (n->time == 0)
            fprintf(stderr, "%s: Allocation index `%lu' has been allocated "
                    "twice without being freed\n", progname, i);
    }
    else
    {
        if ((n = (allocation *) malloc(sizeof(allocation))) == NULL)
        {
            fprintf(stderr, "%s: Out of memory\n", progname);
            exit(EXIT_FAILURE);
        }
        __mp_treeinsert(&alloctree, &n->node, i);
        n->event = e;
    }
    n->addr = a;
    n->size = l;
    n->time = 0;
}


/* Free all existing memory allocations.
 */

static
void
freeallocs(void)
{
    allocation *n, *p;

    for (n = (allocation *) __mp_minimum(alloctree.root); n != NULL; n = p)
    {
        p = (allocation *) __mp_successor(&n->node);
        __mp_treeremove(&alloctree, &n->node);
        free(n);
    }
}


/* Byte-swap a block of memory.
 */

static
void
byteswap(void *b, size_t n)
{
    char *s, *t;
    char c;

    s = (char *) b;
    t = (char *) b + n - 1;
    while (s < t)
    {
        c = *s;
        *s++ = *t;
        *t-- = c;
    }
}


/* Refill the input buffer.  The input buffer is necessary since we need to
 * have a minimum number of bytes to read an LEB128 number from the input
 * file.
 */

static
size_t
refill(size_t s)
{
    static int e;
    size_t l, n;

    /* We only need to refill the input buffer if there are not enough bytes
     * in the buffer and we have not reached the end of the file.
     */
    if ((e == 1) || (bufferlen >= s))
        return bufferlen;
    /* Check that the requested number of bytes will fit into the buffer.
     */
    if (s > MP_BUFFER_SIZE)
    {
        fprintf(stderr, "%s: Buffer overflow\n", progname);
        exit(EXIT_FAILURE);
    }
    /* If there are any remaining bytes in the buffer then move them to the
     * start of the buffer so that we can fill up the rest of the buffer.
     */
    if ((bufferpos != buffer) && (bufferlen > 0))
        memmove(buffer, bufferpos, bufferlen);
    bufferpos = buffer;
    /* Attempt to fill up the buffer with bytes from the input file.
     */
    l = MP_BUFFER_SIZE - bufferlen;
    if ((n = fread(buffer + bufferlen, sizeof(char), l, tracefile)) != l)
        if (feof(tracefile))
            e = 1;
        else
        {
            fprintf(stderr, "%s: Error reading file\n", progname);
            exit(EXIT_FAILURE);
        }
    bufferlen += n;
    /* If the buffer has not been completely filled then we zero the remaining
     * bytes.  This is done simply to prevent running off the end of the buffer
     * if we are reading an LEB128 number from an unterminated file.
     */
    if (l = MP_BUFFER_SIZE - bufferlen)
        memset(buffer + bufferlen, 0, l);
    return bufferlen;
}


/* Read an entry from the tracing output file.
 */

static
void
getentry(void *d, size_t l, size_t n, int b)
{
    size_t i, s;

    s = l * n;
    if (refill(s) < s)
    {
        fprintf(stderr, "%s: Error reading file\n", progname);
        exit(EXIT_FAILURE);
    }
    memcpy(d, bufferpos, s);
    bufferpos += s;
    bufferlen -= s;
    /* Byte-swap all of the elements if necessary.
     */
    if (b != 0)
        for (i = 0; i < n; i++)
        {
            byteswap(d, l);
            d = (char *) d + l;
        }
}


/* Read a signed LEB128 number from the tracing output file.
 */

static
long
getsleb128(void)
{
    size_t s;
    long n;

    /* Since the tracing output file must end with a magic sequence of 4 bytes,
     * it is not unreasonable to request at least 5 bytes from the input file
     * since an LEB128 number must be at least 1 byte.
     */
    if (refill(5) < 5)
    {
        fprintf(stderr, "%s: Error reading file\n", progname);
        exit(EXIT_FAILURE);
    }
    n = __mp_decodesleb128(bufferpos, &s);
    bufferpos += s;
    bufferlen -= s;
    return n;
}


/* Read an unsigned LEB128 number from the tracing output file.
 */

static
unsigned long
getuleb128(void)
{
    size_t s;
    unsigned long n;

    /* Since the tracing output file must end with a magic sequence of 4 bytes,
     * it is not unreasonable to request at least 5 bytes from the input file
     * since an LEB128 number must be at least 1 byte.
     */
    if (refill(5) < 5)
    {
        fprintf(stderr, "%s: Error reading file\n", progname);
        exit(EXIT_FAILURE);
    }
    n = __mp_decodeuleb128(bufferpos, &s);
    bufferpos += s;
    bufferlen -= s;
    return n;
}


#if MP_GUI_SUPPORT
/* Refresh the memory display window.
 */

static
void
redrawmemory(Widget w, XtPointer d, XmDrawingAreaCallbackStruct *s)
{
    XCopyArea(appdisplay, pixmap, s->window, ungc, 0, 0, width - 1, height - 1,
              0, 0);
}
#endif /* MP_GUI_SUPPORT */


#if MP_GUI_SUPPORT
/* Update the memory display window.
 */

static
void
drawmemory(void *a, size_t s, GC g)
{
    unsigned long i, j, l, u, x1, x2, y1, y2;

    if (a < addrbase)
        return;
    l = (unsigned long) a - (unsigned long) addrbase;
    u = l + s - 1;
    l /= addrscale;
    u /= addrscale;
    x1 = l % width;
    y1 = l / width;
    x2 = u % width;
    y2 = u / width;
    if (y2 >= height)
        return;
    if (y1 == y2)
    {
        XDrawLine(appdisplay, XtWindow(drawwidget), g, x1, y1, x2, y1);
        XDrawLine(appdisplay, pixmap, g, x1, y1, x2, y1);
    }
    else
    {
        for (i = x1, j = y1; j < y2; i = 0, j++)
        {
            XDrawLine(appdisplay, XtWindow(drawwidget), g, i, j, width - 1, j);
            XDrawLine(appdisplay, pixmap, g, i, j, width - 1, j);
        }
        XDrawLine(appdisplay, XtWindow(drawwidget), g, 0, y2, x2, y2);
        XDrawLine(appdisplay, pixmap, g, 0, y2, x2, y2);
    }
    /* We just do a busy loop here since sleep() does not have enough accuracy
     * and usleep() is only available on a small number of platforms.  We need
     * to modify the global variable "delay" in case some clever compilers
     * optimise away the pointless modification of a local variable.
     */
    if (delay > 0)
    {
        i = delay;
        for (delay *= 100000; delay > 0; delay--);
        delay = i;
    }
}
#endif /* MP_GUI_SUPPORT */


/* Read an event from the tracing output file.
 */

#if MP_GUI_SUPPORT
static
int
readevent(XtPointer p)
#else /* MP_GUI_SUPPORT */
static
int
readevent(void)
#endif /* MP_GUI_SUPPORT */
{
    char s[4];
    allocation *f;
    void *a;
    size_t l;
    unsigned long n;

    if (refill(1))
        switch (*bufferpos)
        {
          case 'A':
            bufferpos++;
            bufferlen--;
            currentevent++;
            n = getuleb128();
            a = (void *) getuleb128();
            l = getuleb128();
            newalloc(n, currentevent, a, l);
            fprintf(stdout, "%6lu  alloc   %6lu  " MP_POINTER "  %8lu\n",
                    currentevent, n, a, l);
#if MP_GUI_SUPPORT
            if (addrbase == NULL)
                addrbase = (void *) __mp_rounddown((unsigned long) a, 1024);
            drawmemory(a, l, algc);
            return 0;
#else /* MP_GUI_SUPPORT */
            return 1;
#endif /* MP_GUI_SUPPORT */
          case 'F':
            bufferpos++;
            bufferlen--;
            currentevent++;
            n = getuleb128();
            if (f = (allocation *) __mp_search(alloctree.root, n))
            {
                if (f->time != 0)
                    fprintf(stderr, "%s: Allocation index `%lu' has already "
                            "been freed\n", progname, n);
                f->time = currentevent - f->event;
                fprintf(stdout, "%6lu  free    %6lu  " MP_POINTER "  %8lu  "
                        "%6lu\n", currentevent, n, f->addr, f->size, f->time);
#if MP_GUI_SUPPORT
                drawmemory(f->addr, f->size, frgc);
#endif /* MP_GUI_SUPPORT */
            }
            else
                fprintf(stderr, "%s: Unknown allocation index `%lu'\n",
                        progname, n);
#if MP_GUI_SUPPORT
            return 0;
#else /* MP_GUI_SUPPORT */
            return 1;
#endif /* MP_GUI_SUPPORT */
          case 'H':
            bufferpos++;
            bufferlen--;
            a = (void *) getuleb128();
            l = getuleb128();
            fprintf(stdout, "        reserve         " MP_POINTER "  %8lu\n", a,
                    l);
#if MP_GUI_SUPPORT
            if (addrbase == NULL)
                addrbase = (void *) __mp_rounddown((unsigned long) a, 1024);
            drawmemory(a, l, frgc);
            return 0;
#else /* MP_GUI_SUPPORT */
            return 1;
#endif /* MP_GUI_SUPPORT */
          case 'I':
            bufferpos++;
            bufferlen--;
            a = (void *) getuleb128();
            l = getuleb128();
            fprintf(stdout, "        internal        " MP_POINTER "  %8lu\n", a,
                    l);
#if MP_GUI_SUPPORT
            drawmemory(a, l, ingc);
            return 0;
#else /* MP_GUI_SUPPORT */
            return 1;
#endif /* MP_GUI_SUPPORT */
          default:
            break;
        }
    getentry(s, sizeof(char), 4, 0);
    if (memcmp(s, MP_TRACEMAGIC, 4) != 0)
    {
        fprintf(stderr, "%s: Invalid file format\n", progname);
        exit(EXIT_FAILURE);
    }
    freeallocs();
    fclose(tracefile);
#if MP_GUI_SUPPORT
    return 1;
#else /* MP_GUI_SUPPORT */
    return 0;
#endif /* MP_GUI_SUPPORT */
}


/* Log the allocations and deallocations from the tracing output file.
 */

static
void
readfile(void)
{
    char s[4];
    size_t i;
    int b;

    /* When reading the tracing output file, we assume that if it begins and
     * ends with the magic sequence of characters then it is a valid tracing
     * output file from the mpatrol library.  There are probably an infinite
     * number of checks we could do to ensure that the rest of the data in the
     * file is valid, but that would be overcomplicated and probably slow this
     * program down.  However, if the file is only partially written then the
     * getentry() function will catch the error before we do something silly.
     */
    getentry(s, sizeof(char), 4, 0);
    if (memcmp(s, MP_TRACEMAGIC, 4) != 0)
    {
        fprintf(stderr, "%s: Invalid file format\n", progname);
        exit(EXIT_FAILURE);
    }
    /* The following test allows us to read tracing output files that were
     * produced on a different processor architecture.  If the next word in the
     * file does not contain the value 1 then we have to byte-swap any further
     * data that we read from the file.  Note that this test only works if the
     * word size is the same on both machines.
     */
    getentry(&i, sizeof(size_t), 1, 0);
    b = (i != 1);
    /* Get the version number of the mpatrol library which produced the
     * tracing output file.  We assume that we can't read files produced by
     * later versions of mpatrol.
     */
    getentry(&version, sizeof(unsigned long), 1, b);
    if (version / 100 > MP_VERNUM / 100)
    {
        fprintf(stderr, "%s: Tracing file version too new\n", progname);
        exit(EXIT_FAILURE);
    }
    /* Display the tracing table headings.
     */
    fputs(" event  type     index  ", stdout);
#if ENVIRON == ENVIRON_64
    fputs("    allocation    ", stdout);
#else /* ENVIRON */
    fputs("allocation", stdout);
#endif /* ENVIRON */
    fputs("      size    life\n", stdout);
    fputs("------  ------  ------  ", stdout);
#if ENVIRON == ENVIRON_64
    fputs("------------------", stdout);
#else /* ENVIRON */
    fputs("----------", stdout);
#endif /* ENVIRON */
    fputs("  --------  ------\n", stdout);
#if !MP_GUI_SUPPORT
    /* Read each allocation or deallocation entry.
     */
    while (readevent());
#endif /* MP_GUI_SUPPORT */
}


/* Read the tracing output file and display all specified information.
 */

int
main(int argc, char **argv)
{
    char b[256];
    char *f;
#if MP_GUI_SUPPORT
    XGCValues g;
#endif /* MP_GUI_SUPPORT */
    int c, e, h, v;

#if MP_GUI_SUPPORT
    appwidget = XtVaAppInitialize(&appcontext, "MPTrace", options,
                                  XtNumber(options), &argc, argv, NULL, NULL);
    XtVaGetApplicationResources(appwidget, NULL, resources, XtNumber(resources),
                                NULL);
    appdisplay = XtDisplay(appwidget);
    appscreen = XtScreen(appwidget);
#endif /* MP_GUI_SUPPORT */
    e = h = v = 0;
    progname = argv[0];
    while ((c = __mp_getopt(argc, argv, __mp_shortopts(b, options_table),
             options_table)) != EOF)
        switch (c)
        {
          case OF_HELP:
            h = 1;
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
        fprintf(stdout, "%s %s\n%s\n\n", progname, VERSION, __mp_copyright);
        fputs("This is free software, and you are welcome to redistribute it "
              "under certain\n", stdout);
        fputs("conditions; see the GNU Library General Public License for "
              "details.\n\n", stdout);
        fputs("For the latest mpatrol release and documentation,\n", stdout);
        fprintf(stdout, "visit %s.\n\n", __mp_homepage);
    }
    if (argc > 1)
        e = 1;
    if ((e == 1) || (h == 1))
    {
        fprintf(stdout, "Usage: %s [options] [file]\n\n", progname);
        if (h == 0)
            fprintf(stdout, "Type `%s --help' for a complete list of "
                    "options.\n", progname);
        else
            __mp_showopts(options_table);
        if (e == 1)
            exit(EXIT_FAILURE);
        exit(EXIT_SUCCESS);
    }
    if (argc == 1)
        f = argv[0];
    else
        f = MP_TRACEFILE;
    __mp_newtree(&alloctree);
    if (strcmp(f, "-") == 0)
        tracefile = stdin;
    else if ((tracefile = fopen(f, "rb")) == NULL)
    {
        fprintf(stderr, "%s: Cannot open file `%s'\n", progname, f);
        exit(EXIT_FAILURE);
    }
    currentevent = 0;
    bufferpos = buffer;
    bufferlen = 0;
    readfile();
#if MP_GUI_SUPPORT
    addrscale = (((addrspace * 1048576) - 1) / (width * height)) + 1;
    /* Set up the main application window and scrollable drawing area.  Also
     * set up a pixmap to backup the drawing area.
     */
    mainwidget = XtVaCreateManagedWidget("main", xmScrolledWindowWidgetClass,
                                         appwidget, XmNwidth, vwidth, XmNheight,
                                         vheight, XmNscrollingPolicy,
                                         XmAUTOMATIC, XmNscrollBarDisplayPolicy,
                                         XmAS_NEEDED, NULL);
    drawwidget = XtVaCreateManagedWidget("draw", xmDrawingAreaWidgetClass,
                                         mainwidget, XmNwidth, width, XmNheight,
                                         height, NULL);
    pixmap = XCreatePixmap(appdisplay, RootWindowOfScreen(appscreen), width,
                           height, DefaultDepthOfScreen(appscreen));
    /* Set up the graphics contexts that are used for drawing in different
     * colours.
     */
    g.foreground = uncol;
    ungc = XCreateGC(appdisplay, RootWindowOfScreen(appscreen), GCForeground,
                     &g);
    g.foreground = incol;
    ingc = XCreateGC(appdisplay, RootWindowOfScreen(appscreen), GCForeground,
                     &g);
    g.foreground = frcol;
    frgc = XCreateGC(appdisplay, RootWindowOfScreen(appscreen), GCForeground,
                     &g);
    g.foreground = alcol;
    algc = XCreateGC(appdisplay, RootWindowOfScreen(appscreen), GCForeground,
                     &g);
    /* Add a callback procedure to handle the refreshing of the drawing area
     * and also a work procedure to read events from the tracing output file.
     * Then initialise the drawing area and enter the main X application loop.
     */
    XtAddCallback(drawwidget, XmNexposeCallback, (XtCallbackProc) redrawmemory,
                  NULL);
    XtAppAddWorkProc(appcontext, (XtWorkProc) readevent, NULL);
    XtRealizeWidget(appwidget);
    XFillRectangle(appdisplay, XtWindow(drawwidget), ungc, 0, 0, width - 1,
                   height - 1);
    XFillRectangle(appdisplay, pixmap, ungc, 0, 0, width - 1, height - 1);
    XtAppMainLoop(appcontext);
#endif /* MP_GUI_SUPPORT */
    return EXIT_SUCCESS;
}
