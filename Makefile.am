
ACLOCAL_AMFLAGS = -I m4

MAINTAINERCLEANFILES = \
Makefile.in \
aclocal.m4 \
config.guess \
config.h.in \
config.sub

SUBDIRS = src

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = mpatrol.pc mpatrol-mt.pc mpalloc.pc

# Unit tests

if MP_ENABLE_TESTS

check-local:
	@./src/tests/mpatrol_suite

else

check-local:
	@echo "reconfigure with --enable-tests"

endif
