# mpatrol
# A library for controlling and tracing dynamic memory allocations.
# Copyright (C) 1997-1999 Graeme S. Roy <graeme@epc.co.uk>
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


# GNU Makefile to build all formats of documentation


# $Id: Makefile,v 1.2 1999-11-14 11:41:52 graeme Exp $


# Definitions

DOCUMENT = mpatrol

VERBOSE  = false


# Rules

.SUFFIXES: .texi .info .html .dvi .ps .pdf .jpg .eps

# Build the info file from the Texinfo file using makeinfo

$(DOCUMENT).info: $(DOCUMENT).texi
	@ echo creating $@ from $<...
ifeq ($(VERBOSE),true)
	@ makeinfo --no-split --number-sections --verbose $<
else
	@ makeinfo --no-split --number-sections $<
endif

# Build the HTML file from the Texinfo file using makeinfo

$(DOCUMENT).html: $(DOCUMENT).texi images/$(DOCUMENT).jpg
	@ echo creating $@ from $<...
ifeq ($(VERBOSE),true)
	@ makeinfo --html --number-sections --verbose $<
else
	@ makeinfo --html --number-sections $<
endif

# Build the DVI file from the Texinfo file using texi2dvi

$(DOCUMENT).dvi: $(DOCUMENT).texi images/$(DOCUMENT).eps
	@ echo creating $@ from $<...
ifeq ($(VERBOSE),true)
	@ texi2dvi --batch --clean --verbose $<
else
	@ texi2dvi --batch --clean --quiet $<
endif

# Build the postscript file from the DVI file using dvips

$(DOCUMENT).ps: $(DOCUMENT).dvi
	@ echo creating $@ from $<...
ifeq ($(VERBOSE),true)
	@ dvips -o $@ $<
else
	@ dvips -q -o $@ $<
endif

# Build the PDF file from the Texinfo file using texi2dvi

$(DOCUMENT).pdf: $(DOCUMENT).texi images/$(DOCUMENT).pdf
	@ echo creating $@ from $<...
ifeq ($(VERBOSE),true)
	@ texi2dvi --pdf --batch --clean --verbose $<
else
	@ texi2dvi --pdf --batch --clean --quiet $<
endif

all: $(DOCUMENT).info $(DOCUMENT).html $(DOCUMENT).dvi $(DOCUMENT).ps \
	$(DOCUMENT).pdf

clean:
	-@ rm -f $(DOCUMENT).info $(DOCUMENT).html >/dev/null 2>&1
	-@ rm -f $(DOCUMENT).dvi $(DOCUMENT).ps $(DOCUMENT).pdf >/dev/null 2>&1
