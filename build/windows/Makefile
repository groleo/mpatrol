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


# GNU Makefile to compile on Windows with Microsoft Visual C/C++


# $Id: Makefile,v 1.5 2000-03-23 19:48:20 graeme Exp $


# Definitions

SHELL = sh
CC    = cl
AR    = lib
LD    = link

SOURCE = ../../src
VPATH  = $(SOURCE)

CFLAGS = -nologo -I$(SOURCE)
OFLAGS = -Ogt
TFLAGS = -DMP_THREADS_SUPPORT

LIBRARY	= mpatrol

ARCHIVE_OBJECTS	= list.obj tree.obj slots.obj utils.obj memory.obj heap.obj \
		  alloc.obj info.obj stack.obj addr.obj strtab.obj symbol.obj \
		  signals.obj diag.obj option.obj inter.obj malloc.obj \
		  sbrk.obj version.obj
SHARED_OBJECTS	= list.obj tree.obj slots.obj utils.obj memory.obj heap.obj \
		  alloc.obj info.obj stack.obj addr.obj strtab.obj symbol.obj \
		  signals.obj diag.obj option.obj inter.obj malloc.obj \
		  sbrk.obj main.obj version.obj
ARCHTS_OBJECTS	= list.ts.obj tree.ts.obj slots.ts.obj utils.ts.obj \
		  memory.ts.obj heap.ts.obj alloc.ts.obj info.ts.obj \
		  stack.ts.obj addr.ts.obj strtab.ts.obj symbol.ts.obj \
		  signals.ts.obj mutex.ts.obj diag.ts.obj option.ts.obj \
		  inter.ts.obj malloc.ts.obj sbrk.ts.obj version.ts.obj
SHARTS_OBJECTS	= list.ts.obj tree.ts.obj slots.ts.obj utils.ts.obj \
		  memory.ts.obj heap.ts.obj alloc.ts.obj info.ts.obj \
		  stack.ts.obj addr.ts.obj strtab.ts.obj symbol.ts.obj \
		  signals.ts.obj mutex.ts.obj diag.ts.obj option.ts.obj \
		  inter.ts.obj malloc.ts.obj sbrk.ts.obj main.ts.obj \
		  version.ts.obj

ARCHIVE_LIBRARY	= lib$(LIBRARY).lib
SHARED_LIBRARY	= $(LIBRARY).dll
OBJECT_LIBRARY	= $(LIBRARY).obj
ARCHTS_LIBRARY	= lib$(LIBRARY)mt.lib
SHARTS_LIBRARY	= $(LIBRARY)mt.dll
OBJTS_LIBRARY	= $(LIBRARY)mt.obj


# Rules

.SUFFIXES: .c .obj .ts.obj

.c.obj:
	$(CC) $(CFLAGS) $(OFLAGS) -c $<

.c.ts.obj:
	$(CC) $(CFLAGS) $(OFLAGS) $(TFLAGS) -c -Fo$@ $<

$(ARCHIVE_LIBRARY): $(ARCHIVE_OBJECTS)
	$(AR) -nologo -out:$@ $(ARCHIVE_OBJECTS)

$(SHARED_LIBRARY): $(SHARED_OBJECTS) $(LIBRARY).def
	$(LD) -dll -nologo -def:$(LIBRARY).def -out:$@ $(SHARED_OBJECTS)

$(OBJECT_LIBRARY): $(SOURCE)/mpatrol.c
	$(CC) $(CFLAGS) $(OFLAGS) -c -Fo$@ $<

$(ARCHTS_LIBRARY): $(ARCHTS_OBJECTS)
	$(AR) -nologo -out:$@ $(ARCHTS_OBJECTS)

$(SHARTS_LIBRARY): $(SHARTS_OBJECTS) $(LIBRARY).def
	$(LD) -dll -nologo -def:$(LIBRARY).def -out:$@ $(SHARTS_OBJECTS)

$(OBJTS_LIBRARY): $(SOURCE)/mpatrol.c
	$(CC) $(CFLAGS) $(OFLAGS) $(TFLAGS) -c -Fo$@ $<

all: $(ARCHIVE_LIBRARY) $(SHARED_LIBRARY) $(ARCHTS_LIBRARY) $(SHARTS_LIBRARY)

clean:
	-@ rm -f $(ARCHIVE_OBJECTS) $(ARCHTS_OBJECTS) 2>&1 >/dev/null
	-@ rm -f $(SHARED_OBJECTS) $(SHARTS_OBJECTS) 2>&1 >/dev/null

clobber: clean
	-@ rm -f $(ARCHIVE_LIBRARY) $(ARCHTS_LIBRARY) 2>&1 >/dev/null
	-@ rm -f $(SHARED_LIBRARY) $(SHARTS_LIBRARY) 2>&1 >/dev/null
	-@ rm -f $(LIBRARY).lib $(LIBRARY).exp 2>&1 >/dev/null
	-@ rm -f $(LIBRARY)mt.lib $(LIBRARY)mt.exp 2>&1 >/dev/null
	-@ rm -f $(OBJECT_LIBRARY) $(OBJTS_LIBRARY) 2>&1 >/dev/null


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
malloc.obj malloc.ts.obj: $(SOURCE)/malloc.c $(SOURCE)/inter.h \
	$(SOURCE)/config.h $(SOURCE)/target.h $(SOURCE)/info.h \
	$(SOURCE)/addr.h $(SOURCE)/heap.h $(SOURCE)/memory.h $(SOURCE)/slots.h \
	$(SOURCE)/tree.h $(SOURCE)/list.h $(SOURCE)/stack.h $(SOURCE)/alloc.h \
	$(SOURCE)/signals.h $(SOURCE)/symbol.h $(SOURCE)/strtab.h
sbrk.obj sbrk.ts.obj: $(SOURCE)/sbrk.c $(SOURCE)/sbrk.h $(SOURCE)/config.h \
        $(SOURCE)/target.h $(SOURCE)/memory.h
main.obj main.ts.obj: $(SOURCE)/malloc.c $(SOURCE)/inter.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/info.h $(SOURCE)/addr.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/alloc.h $(SOURCE)/signals.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h
version.obj version.ts.obj: $(SOURCE)/version.c $(SOURCE)/version.h \
	$(SOURCE)/config.h $(SOURCE)/target.h
$(OBJECT_LIBRARY): $(SOURCE)/mpatrol.c $(SOURCE)/config.h $(SOURCE)/target.h \
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
$(OBJTS_LIBRARY): $(SOURCE)/mpatrol.c $(SOURCE)/config.h $(SOURCE)/target.h \
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
