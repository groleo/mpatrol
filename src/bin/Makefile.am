
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib

bin_PROGRAMS = mleak mpatrol mprof mptrace

mpatrol_SOURCES = mpatrol.c getopt.c
mpatrol_LDADD = $(top_builddir)/src/lib/libmpatrol.la
mpatrol_LDFLAGS = $(MPATROL_LDFLAGS)

mprof_SOURCES = mprof.c graph.c getopt.c
mprof_LDADD = $(top_builddir)/src/lib/libmpatrol.la
mprof_LDFLAGS = $(MPATROL_LDFLAGS)

mptrace_SOURCES = mptrace.c getopt.c
mptrace_LDADD = $(top_builddir)/src/lib/libmpatrol.la
mptrace_LDFLAGS = $(MPATROL_LDFLAGS)

mleak_SOURCES = mleak.c getopt.c
mleak_LDADD = $(top_builddir)/src/lib/libmpatrol.la
mleak_LDFLAGS = $(MPATROL_LDFLAGS)

EXTRA_DIST = getopt.h graph.h
