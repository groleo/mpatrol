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


# GNU Makefile to build all formats of documentation


# $Id: Makefile,v 1.15 2000-10-10 19:29:05 graeme Exp $


# Definitions

SHELL = /bin/sh

DOCUMENT = mpatrol
REFCARD  = refcard

SPLITHTML = false
VERBOSE   = false


# Rules

.SUFFIXES: .texi .tex .txt .info .guide .html .dvi .ps .pdf .jpg .eps .png

# Build the text file from the Texinfo file using makeinfo

$(DOCUMENT).txt: $(DOCUMENT).texi images/$(DOCUMENT).txt
	@ echo creating $@ from $<...
ifeq ($(VERBOSE),true)
	@ makeinfo --no-headers --number-sections --verbose $< >$@
else
	@ makeinfo --no-headers --number-sections $< >$@
endif

# Build the info file from the Texinfo file using makeinfo

$(DOCUMENT).info: $(DOCUMENT).texi images/$(DOCUMENT).txt
	@ echo creating $@ from $<...
ifeq ($(VERBOSE),true)
	@ makeinfo --no-split --verbose $<
else
	@ makeinfo --no-split $<
endif

# Build the AmigaGuide file from the Texinfo file using makeguide

$(DOCUMENT).guide: $(DOCUMENT).texi images/$(DOCUMENT).txt
	@ echo creating $@ from $<...
ifeq ($(VERBOSE),true)
	@ makeguide --amiga --verbose $<
else
	@ makeguide --amiga $<
endif

ifeq ($(SPLITHTML),true)
# Build the HTML files from the Texinfo file using texi2html

$(DOCUMENT).html: $(DOCUMENT).texi images/$(DOCUMENT).jpg
	@ echo creating $@ from $<...
ifeq ($(VERBOSE),true)
	@ texi2html -top_file $@ -split node -menu -number -verbose $<
else
	@ texi2html -top_file $@ -split node -menu -number $<
endif
else
# Build the HTML file from the Texinfo file using makeinfo

$(DOCUMENT).html: $(DOCUMENT).texi images/$(DOCUMENT).jpg
	@ echo creating $@ from $<...
ifeq ($(VERBOSE),true)
	@ makeinfo --html --number-sections --verbose $<
else
	@ makeinfo --html --number-sections $<
endif
	@ sed "s/<[ ]*<[ ]*[Pp][ ]*>[ ]*[Ii][Mm][Gg][ ]*/<img /g" $@ >$@.new
	@ mv $@.new $@
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
	@ dvips -r0 -o $@ $<
else
	@ dvips -r0 -q -o $@ $<
endif

# Build the PDF file from the Texinfo file using texi2dvi

$(DOCUMENT).pdf: $(DOCUMENT).texi images/$(DOCUMENT).png
	@ echo creating $@ from $<...
ifeq ($(VERBOSE),true)
	@ texi2dvi --pdf --batch --clean --verbose $<
else
	@ texi2dvi --pdf --batch --clean --quiet $<
endif

# Build the DVI reference card from the LaTeX file using texi2dvi

$(REFCARD).dvi: $(REFCARD).tex
	@ echo creating $@ from $<...
ifeq ($(VERBOSE),true)
	@ texi2dvi --language=latex --batch --clean --verbose $<
else
	@ texi2dvi --language=latex --batch --clean --quiet $<
endif

# Build the postscript reference card from the DVI file using dvips

$(REFCARD).ps: $(REFCARD).dvi
	@ echo creating $@ from $<...
ifeq ($(VERBOSE),true)
	@ dvips -t landscape -r0 -o $@ $<
else
	@ dvips -t landscape -r0 -q -o $@ $<
endif

# Build the PDF reference card from the LaTeX file using texi2dvi

$(REFCARD).pdf: $(REFCARD).tex
	@ echo creating $@ from $<...
ifeq ($(VERBOSE),true)
	@ texi2dvi --pdf --language=latex --batch --clean --verbose $<
else
	@ texi2dvi --pdf --language=latex --batch --clean --quiet $<
endif

all: $(DOCUMENT).txt $(DOCUMENT).info $(DOCUMENT).guide $(DOCUMENT).html \
	$(DOCUMENT).dvi $(DOCUMENT).ps $(DOCUMENT).pdf $(REFCARD).dvi \
	$(REFCARD).ps $(REFCARD).pdf

$(DOCUMENT)_doc.tar.gz: all README
	@ mkdir -p $(DOCUMENT)/doc
	@ cp $(DOCUMENT).info $(DOCUMENT).guide $(DOCUMENT)/doc
	@ cp $(DOCUMENT).html $(DOCUMENT).dvi $(DOCUMENT)/doc
	@ cp $(DOCUMENT).ps $(DOCUMENT).pdf $(DOCUMENT)/doc
	@ cp $(REFCARD).dvi $(REFCARD).ps $(DOCUMENT)/doc
	@ cp $(REFCARD).pdf README $(DOCUMENT)/doc
	@ tar cf $(DOCUMENT)_doc.tar $(DOCUMENT)
	@ gzip $(DOCUMENT)_doc.tar
	-@ rm -rf $(DOCUMENT) >/dev/null 2>&1

clean:
	-@ rm -f $(DOCUMENT).txt $(DOCUMENT).info >/dev/null 2>&1
	-@ rm -f $(DOCUMENT).guide $(DOCUMENT).html >/dev/null 2>&1
ifeq ($(SPLITHTML),true)
	-@ rm -f $(DOCUMENT)_*.html >/dev/null 2>&1
endif
	-@ rm -f $(DOCUMENT).dvi $(DOCUMENT).ps $(DOCUMENT).pdf >/dev/null 2>&1
	-@ rm -f $(REFCARD).dvi $(REFCARD).ps $(REFCARD).pdf >/dev/null 2>&1
	-@ rm -f $(DOCUMENT)_doc.tar.gz >/dev/null 2>&1

clobber: clean
