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


# GNU Makefile to compile on Windows with EPC C/C++ Netware Cross-Compiler


# $Id: Makefile,v 1.7 2000-04-06 00:41:35 graeme Exp $


# Definitions

SHELL = sh
CC    = ncc
AR    = ncc

SOURCE = ../../src
VPATH  = $(SOURCE)

CFLAGS = -I$(SOURCE)
OFLAGS = -O
TFLAGS = -DMP_THREADS_SUPPORT

LIBRARY	= mpatrol

OBJECTS	 = list.obj tree.obj slots.obj utils.obj memory.obj heap.obj alloc.obj \
	   info.obj stack.obj addr.obj strtab.obj symbol.obj signals.obj \
	   diag.obj option.obj inter.obj version.obj
TOBJECTS = list.ts.obj tree.ts.obj slots.ts.obj utils.ts.obj memory.ts.obj \
	   heap.ts.obj alloc.ts.obj info.ts.obj stack.ts.obj addr.ts.obj \
	   strtab.ts.obj symbol.ts.obj signals.ts.obj mutex.ts.obj diag.ts.obj \
	   option.ts.obj inter.ts.obj version.ts.obj

ARCHIVE_LIBRARY	= $(LIBRARY).lib
OBJECT_LIBRARY	= $(LIBRARY).obj
ARCHTS_LIBRARY	= $(LIBRARY)mt.lib
OBJTS_LIBRARY	= $(LIBRARY)mt.obj


# Rules

.SUFFIXES: .c .obj .ts.obj

.c.obj:
	$(CC) $(CFLAGS) $(OFLAGS) -c $<

.c.ts.obj:
	$(CC) $(CFLAGS) $(OFLAGS) $(TFLAGS) -c -o $@ $<

$(ARCHIVE_LIBRARY): $(OBJECTS)
	$(AR) -xar -o $@ $(OBJECTS)

$(OBJECT_LIBRARY): $(SOURCE)/library.c
	$(CC) $(CFLAGS) $(OFLAGS) -c -o $@ $<

$(ARCHTS_LIBRARY): $(TOBJECTS)
	$(AR) -xar -o $@ $(TOBJECTS)

$(OBJTS_LIBRARY): $(SOURCE)/library.c
	$(CC) $(CFLAGS) $(OFLAGS) $(TFLAGS) -c -o $@ $<

mpatrol.nlm: mpatrol.obj getopt.obj
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ mpatrol.obj getopt.obj

memleak.nlm: memleak.obj getopt.obj tree.obj
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ memleak.obj getopt.obj tree.obj

all: $(ARCHIVE_LIBRARY) $(ARCHTS_LIBRARY) mpatrol.nlm memleak.nlm

clean:
	-@ rm -f $(OBJECTS) $(TOBJECTS) 2>&1 >/dev/null
	-@ rm -f mpatrol.obj memleak.obj getopt.obj 2>&1 >/dev/null

clobber: clean
	-@ rm -f $(ARCHIVE_LIBRARY) $(ARCHTS_LIBRARY) 2>&1 >/dev/null
	-@ rm -f $(OBJECT_LIBRARY) $(OBJTS_LIBRARY) 2>&1 >/dev/null
	-@ rm -f mpatrol.nlm memleak.nlm 2>&1 >/dev/null


# Dependencies

list.obj list.ts.obj: $(SOURCE)/list.c $(SOURCE)/list.h $(SOURCE)/config.h \
	$(SOURCE)/target.h
tree.obj tree.ts.obj: $(SOURCE)/tree.c $(SOURCE)/tree.h $(SOURCE)/config.h \
	$(SOURCE)/target.h
slots.obj slots.ts.obj: $(SOURCE)/slots.c $(SOURCE)/slots.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/utils.h
utils.obj utils.ts.obj: $(SOURCE)/utils.c $(SOURCE)/utils.h $(SOURCE)/config.h \
	$(SOURCE)/target.h
memory.obj memory.ts.obj: $(SOURCE)/memory.c $(SOURCE)/memory.h \
	$(SOURCE)/config.h $(SOURCE)/target.h $(SOURCE)/stack.h \
	$(SOURCE)/utils.h
heap.obj heap.ts.obj: $(SOURCE)/heap.c $(SOURCE)/heap.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/memory.h $(SOURCE)/slots.h \
	$(SOURCE)/tree.h $(SOURCE)/utils.h
alloc.obj alloc.ts.obj: $(SOURCE)/alloc.c $(SOURCE)/alloc.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/heap.h $(SOURCE)/memory.h \
	$(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h $(SOURCE)/utils.h
info.obj: $(SOURCE)/info.c $(SOURCE)/info.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/addr.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/alloc.h $(SOURCE)/signals.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/diag.h $(SOURCE)/utils.h
info.ts.obj: $(SOURCE)/info.c $(SOURCE)/info.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/addr.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/alloc.h $(SOURCE)/signals.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/diag.h \
	$(SOURCE)/mutex.h $(SOURCE)/utils.h
stack.obj stack.ts.obj: $(SOURCE)/stack.c $(SOURCE)/stack.h $(SOURCE)/config.h \
	$(SOURCE)/target.h
addr.obj addr.ts.obj: $(SOURCE)/addr.c $(SOURCE)/addr.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/heap.h $(SOURCE)/memory.h \
	$(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h $(SOURCE)/stack.h \
	$(SOURCE)/utils.h
strtab.obj strtab.ts.obj: $(SOURCE)/strtab.c $(SOURCE)/strtab.h \
	$(SOURCE)/config.h $(SOURCE)/target.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/utils.h
symbol.obj symbol.ts.obj: $(SOURCE)/symbol.c $(SOURCE)/symbol.h \
	$(SOURCE)/config.h $(SOURCE)/target.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h \
	$(SOURCE)/strtab.h $(SOURCE)/diag.h $(SOURCE)/info.h $(SOURCE)/addr.h \
	$(SOURCE)/list.h $(SOURCE)/stack.h $(SOURCE)/alloc.h \
	$(SOURCE)/signals.h $(SOURCE)/utils.h
signals.obj signals.ts.obj: $(SOURCE)/signals.c $(SOURCE)/signals.h \
	$(SOURCE)/config.h $(SOURCE)/target.h $(SOURCE)/diag.h \
	$(SOURCE)/info.h $(SOURCE)/addr.h $(SOURCE)/heap.h $(SOURCE)/memory.h \
	$(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h $(SOURCE)/stack.h \
	$(SOURCE)/alloc.h $(SOURCE)/symbol.h $(SOURCE)/strtab.h \
	$(SOURCE)/inter.h
mutex.ts.obj: $(SOURCE)/mutex.c $(SOURCE)/mutex.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/inter.h $(SOURCE)/info.h $(SOURCE)/addr.h \
	$(SOURCE)/heap.h $(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h \
	$(SOURCE)/list.h $(SOURCE)/stack.h $(SOURCE)/alloc.h \
	$(SOURCE)/signals.h $(SOURCE)/symbol.h $(SOURCE)/strtab.h
diag.obj diag.ts.obj: $(SOURCE)/diag.c $(SOURCE)/diag.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/info.h $(SOURCE)/addr.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/alloc.h $(SOURCE)/signals.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/utils.h \
	$(SOURCE)/version.h
option.obj option.ts.obj: $(SOURCE)/option.c $(SOURCE)/option.h \
	$(SOURCE)/config.h $(SOURCE)/target.h $(SOURCE)/info.h \
	$(SOURCE)/addr.h $(SOURCE)/heap.h $(SOURCE)/memory.h $(SOURCE)/slots.h \
	$(SOURCE)/tree.h $(SOURCE)/list.h $(SOURCE)/stack.h $(SOURCE)/alloc.h \
	$(SOURCE)/signals.h $(SOURCE)/symbol.h $(SOURCE)/strtab.h \
	$(SOURCE)/diag.h $(SOURCE)/utils.h
inter.obj: $(SOURCE)/inter.c $(SOURCE)/inter.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/info.h $(SOURCE)/addr.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/alloc.h $(SOURCE)/signals.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/diag.h \
	$(SOURCE)/option.h
inter.ts.obj: $(SOURCE)/inter.c $(SOURCE)/inter.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/info.h $(SOURCE)/addr.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/alloc.h $(SOURCE)/signals.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/diag.h \
	$(SOURCE)/mutex.h $(SOURCE)/option.h
version.obj version.ts.obj: $(SOURCE)/version.c $(SOURCE)/version.h \
	$(SOURCE)/config.h $(SOURCE)/target.h
$(OBJECT_LIBRARY): $(SOURCE)/library.c $(SOURCE)/config.h $(SOURCE)/target.h \
	$(SOURCE)/list.c $(SOURCE)/list.h $(SOURCE)/tree.c $(SOURCE)/tree.h \
	$(SOURCE)/slots.c $(SOURCE)/slots.h $(SOURCE)/utils.h \
	$(SOURCE)/utils.c $(SOURCE)/memory.c $(SOURCE)/memory.h \
	$(SOURCE)/stack.h $(SOURCE)/heap.c $(SOURCE)/heap.h $(SOURCE)/alloc.c \
	$(SOURCE)/alloc.h $(SOURCE)/info.c $(SOURCE)/info.h $(SOURCE)/addr.h \
	$(SOURCE)/signals.h $(SOURCE)/symbol.h $(SOURCE)/strtab.h \
	$(SOURCE)/diag.h $(SOURCE)/stack.c $(SOURCE)/addr.c $(SOURCE)/strtab.c \
	$(SOURCE)/symbol.c $(SOURCE)/signals.c $(SOURCE)/inter.h \
	$(SOURCE)/diag.c $(SOURCE)/version.h $(SOURCE)/option.c \
	$(SOURCE)/option.h $(SOURCE)/inter.c $(SOURCE)/version.c
$(OBJTS_LIBRARY): $(SOURCE)/library.c $(SOURCE)/config.h $(SOURCE)/target.h \
	$(SOURCE)/list.c $(SOURCE)/list.h $(SOURCE)/tree.c $(SOURCE)/tree.h \
	$(SOURCE)/slots.c $(SOURCE)/slots.h $(SOURCE)/utils.h \
	$(SOURCE)/utils.c $(SOURCE)/memory.c $(SOURCE)/memory.h \
	$(SOURCE)/stack.h $(SOURCE)/heap.c $(SOURCE)/heap.h $(SOURCE)/alloc.c \
	$(SOURCE)/alloc.h $(SOURCE)/info.c $(SOURCE)/info.h $(SOURCE)/addr.h \
	$(SOURCE)/signals.h $(SOURCE)/symbol.h $(SOURCE)/strtab.h \
	$(SOURCE)/diag.h $(SOURCE)/mutex.h $(SOURCE)/stack.c $(SOURCE)/addr.c \
	$(SOURCE)/strtab.c $(SOURCE)/symbol.c $(SOURCE)/signals.c \
	$(SOURCE)/inter.h $(SOURCE)/mutex.c $(SOURCE)/diag.c \
	$(SOURCE)/version.h $(SOURCE)/option.c $(SOURCE)/option.h \
	$(SOURCE)/inter.c $(SOURCE)/version.c
mpatrol.obj: $(SOURCE)/mpatrol.c $(SOURCE)/getopt.h $(SOURCE)/config.h \
	$(SOURCE)/target.h
memleak.obj: $(SOURCE)/memleak.c $(SOURCE)/getopt.h $(SOURCE)/tree.h \
	$(SOURCE)/config.h $(SOURCE)/target.h
getopt.obj: $(SOURCE)/getopt.c $(SOURCE)/getopt.h $(SOURCE)/config.h \
	$(SOURCE)/target.h
