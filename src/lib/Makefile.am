
MAINTAINERCLEANFILES = Makefile.in

installed_mainheaderdir = $(includedir)/mpatrol-1
dist_installed_mainheader_DATA = mpatrol.h mpalloc.h mpdebug.h

lib_LTLIBRARIES = libmpatrol.la libmpatrolmt.la libmpalloc.la

MPATROL_SOURCES = \
list.c \
tree.c \
slots.c \
utils.c \
memory.c\
heap.c \
alloc.c \
info.c \
stack.c \
addr.c \
strtab.c \
symbol.c \
signals.c \
diag.c \
option.c \
leaktab.c \
profile.c \
trace.c \
inter.c \
malloc.c \
cplus.cpp \
version.c

if HAVE_WINDOWS

MPATROL_SOURCES += sbrk.c main.c

else # Unix

MPATROL_SOURCES += machine.c

endif

libmpatrol_la_SOURCES  = $(MPATROL_SOURCES)

libmpatrol_la_CPPFLAGS =
libmpatrol_la_CFLAGS = $(MPATROL_CFLAGS)
libmpatrol_la_LIBADD = $(MPATROL_LIBS)
libmpatrol_la_LDFLAGS = $(MPATROL_LDFLAGS) -no-undefined -version-info @version_info@

if HAVE_WINDOWS

libmpatrol_la_LIBTOOLFLAGS = --tag=disable-shared

endif

libmpatrol_la-machine.lo: machine.c config.h target.h
	$(COMPILE) -E -P $< > libmpatrol_la-machine.S
	$(LTCXXCOMPILE) -c libmpatrol_la-machine.S
	rm -f libmpatrol_la-machine.S


libmpatrolmt_la_SOURCES  = $(MPATROL_SOURCES) mutex.c

libmpatrolmt_la_CPPFLAGS = -DMP_THREADS_SUPPORT
libmpatrolmt_la_CFLAGS = $(MPATROL_CFLAGS)
libmpatrolmt_la_LIBADD = $(MPATROL_LIBS)
libmpatrolmt_la_LDFLAGS = $(MPATROL_LDFLAGS) -no-undefined -version-info @version_info@

if HAVE_WINDOWS

libmpatrolmt_la_LIBTOOLFLAGS = --tag=disable-shared

endif

libmpatrolmt_la-machine.lo: machine.c config.h target.h
	$(COMPILE) -E -P $< > libmpatrolmt_la-machine.S
	$(LTCOMPILE) -c libmpatrolmt_la-machine.S
	rm -f libmpatrolmt_la-machine.S

libmpalloc_la_SOURCES  = \
mpalloc.c \
mpfailure.c \
mpsetfail.c \
mpmalloc.c \
mpcalloc.c \
mpstrdup.c \
mprealloc.c

if HAVE_WINDOWS

libmpalloc_la_SOURCES += main2.c

endif

libmpalloc_la_CPPFLAGS =
libmpalloc_la_CFLAGS = $(MPATROL_CFLAGS)
libmpalloc_la_LIBADD = $(MPATROL_LIBS)
libmpalloc_la_LDFLAGS = $(MPATROL_LDFLAGS) -no-undefined -version-info @version_info@

if HAVE_WINDOWS

libmpalloc_la_LIBTOOLFLAGS = --tag=disable-shared

endif

EXTRA_DIST = \
config.h \
target.h \
list.h \
tree.h \
slots.h \
utils.h \
memory.h \
heap.h \
alloc.h \
info.h \
stack.h \
addr.h \
strtab.h \
symbol.h \
signals.h \
diag.h \
option.h \
leaktab.h \
mutex.h \
profile.h \
trace.h \
inter.h \
machine.h \
version.h \
sbrk.h