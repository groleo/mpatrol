MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
@CHECK_CFLAGS@

if MP_ENABLE_TESTS

check_PROGRAMS = mpatrol_suite

mpatrol_suite_SOURCES = \
mpatrol_suite.c \
mpatrol_test1.c

mpatrol_suite_LDADD = @CHECK_LIBS@ $(top_builddir)/src/lib/.libs/libmpatrol.a @MPATROL_LIBS@

endif

EXTRA_DIST = mpatrol_suite.h