# mpatrol
# A library for controlling and tracing dynamic memory allocations.
# Copyright (C) 1997-2000 Graeme S. Roy <graeme@epc.co.uk>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307, USA.


# GNU Makefile to build the UNIX manual pages


# $Id: Makefile,v 1.4 2000-12-13 23:21:36 graeme Exp $


# Definitions

SHELL = /bin/sh
ROFF  = groff

PAGES = cat1/mpatrol.1 cat1/mprof.1 cat1/mptrace.1 cat1/mleak.1 cat1/mpsym.1 \
	cat1/mpedit.1 cat3/libmpatrol.3


# Rules

.SUFFIXES: .1 .3

all: $(PAGES)

cat1/mpatrol.1: man1/mpatrol.1
	$(ROFF) -Tascii -man $< >$@

cat1/mprof.1: man1/mprof.1
	$(ROFF) -Tascii -man $< >$@

cat1/mptrace.1: man1/mptrace.1
	$(ROFF) -Tascii -man $< >$@

cat1/mleak.1: man1/mleak.1
	$(ROFF) -Tascii -man $< >$@

cat1/mpsym.1: man1/mpsym.1
	$(ROFF) -Tascii -man $< >$@

cat1/mpedit.1: man1/mpedit.1
	$(ROFF) -Tascii -man $< >$@

cat3/libmpatrol.3: man3/libmpatrol.3
	$(ROFF) -Tascii -man -t $< >$@

clean:
	-@ rm -f $(PAGES) >/dev/null 2>&1

clobber: clean
