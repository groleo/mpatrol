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


# $Id: Makefile,v 1.23 2000-11-08 01:42:44 graeme Exp $


# Definitions

SHELL = /bin/sh

DOCUMENT = mpatrol
REFCARD  = refcard

SPLITHTML = false
VERBOSE   = false


# Rules

.SUFFIXES: .texi .tex .txt .info .guide .html .dvi .ps .pdf .jpg .eps

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

$(DOCUMENT).html: $(DOCUMENT).texi images/$(DOCUMENT).jpg images/gcc.jpg \
	images/cpp.jpg images/cc1.jpg images/collect2.jpg
	@ echo creating $@ from $<...
ifeq ($(VERBOSE),true)
	@ texi2html -top_file $@ -split section -menu -number -verbose $<
else
	@ texi2html -top_file $@ -split section -menu -number $<
endif
else
# Build the HTML file from the Texinfo file using makeinfo

$(DOCUMENT).html: $(DOCUMENT).texi images/$(DOCUMENT).jpg images/gcc.jpg \
	images/cpp.jpg images/cc1.jpg images/collect2.jpg
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

$(DOCUMENT).dvi: $(DOCUMENT).texi texinfo.tex images/$(DOCUMENT).eps \
	images/gcc.eps images/cpp.eps images/cc1.eps images/collect2.eps
	@ echo creating $@ from $<...
ifeq ($(VERBOSE),true)
	@ TEXPSHEADERS=`pwd`: texi2dvi --batch --clean --verbose $<
else
	@ TEXPSHEADERS=`pwd`: texi2dvi --batch --clean --quiet $<
endif

# Build the postscript file from the DVI file using dvips

$(DOCUMENT).ps: $(DOCUMENT).dvi
	@ echo creating $@ from $<...
ifeq ($(VERBOSE),true)
	@ dvips -Pcmz -o $@ $<
else
	@ dvips -Pcmz -q -o $@ $<
endif

# Build the PDF file from the Texinfo file using texi2dvi

$(DOCUMENT).pdf: $(DOCUMENT).texi texinfo.tex images/$(DOCUMENT).pdf \
	images/gcc.pdf images/cpp.pdf images/cc1.pdf images/collect2.pdf
	@ echo creating $@ from $<...
ifeq ($(VERBOSE),true)
	@ TEXPSHEADERS=`pwd`: texi2dvi --pdf --batch --clean --verbose $<
else
	@ TEXPSHEADERS=`pwd`: texi2dvi --pdf --batch --clean --quiet $<
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
	@ dvips -t landscape -Pcmz -o $@ $<
else
	@ dvips -t landscape -Pcmz -q -o $@ $<
endif

# Build the PDF reference card from the postscript file using pstill

$(REFCARD).pdf: $(REFCARD).ps
	@ echo creating $@ from $<...
ifeq ($(VERBOSE),true)
	@ pstill -Fa4 -R270 -ctv -o $@ $<
else
	@ pstill -Fa4 -R270 -ct -o $@ $<
endif

all: $(DOCUMENT).txt $(DOCUMENT).info $(DOCUMENT).guide $(DOCUMENT).html \
	$(DOCUMENT).dvi $(DOCUMENT).ps $(DOCUMENT).pdf $(REFCARD).dvi \
	$(REFCARD).ps $(REFCARD).pdf

$(DOCUMENT)_doc.tar.gz: $(DOCUMENT).dvi $(DOCUMENT).ps $(DOCUMENT).pdf \
	$(REFCARD).dvi $(REFCARD).ps $(REFCARD).pdf README
	@ mkdir -p $(DOCUMENT)/doc
	@ cp $(DOCUMENT).dvi $(DOCUMENT).ps $(DOCUMENT).pdf $(DOCUMENT)/doc
	@ cp $(REFCARD).dvi $(REFCARD).ps $(REFCARD).pdf $(DOCUMENT)/doc
	@ cp README $(DOCUMENT)/doc
	@ tar cf $(DOCUMENT)_doc.tar $(DOCUMENT)
	@ gzip $(DOCUMENT)_doc.tar
	-@ rm -rf $(DOCUMENT) >/dev/null 2>&1

clean:
	-@ rm -f $(DOCUMENT).txt $(DOCUMENT).info >/dev/null 2>&1
	-@ rm -f $(DOCUMENT).guide $(DOCUMENT).html >/dev/null 2>&1
	-@ rm -f $(DOCUMENT)_*.html >/dev/null 2>&1
	-@ rm -f $(DOCUMENT).dvi $(DOCUMENT).ps $(DOCUMENT).pdf >/dev/null 2>&1
	-@ rm -f $(REFCARD).dvi $(REFCARD).ps $(REFCARD).pdf >/dev/null 2>&1
	-@ rm -f $(DOCUMENT)_doc.tar.gz >/dev/null 2>&1

clobber: clean
