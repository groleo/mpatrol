
ACLOCAL_AMFLAGS = -I m4

MAINTAINERCLEANFILES = \
Makefile.in \
aclocal.m4 \
config.guess \
configure \
configure.h.in \
configure.h.in~ \
config.sub \
depcomp \
install \
install-sh \
ltmain.sh \
missing \
$(PACKAGE_TARNAME)-$(VERSION).tar.bz2 \
$(PACKAGE_TARNAME)-$(VERSION).tar.gz

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
