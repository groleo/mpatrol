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


# GNU Makefile to compile on UNIX with GCC


# $Id: Makefile,v 1.2 2000-01-09 22:01:57 graeme Exp $


# Definitions

SHELL = /bin/sh
CC    = gcc
AR    = ar
LD    = ld

SOURCE = ../../src
VPATH  = $(SOURCE)

CFLAGS = -I$(SOURCE)
OFLAGS = -O3 -fno-inline-functions
SFLAGS = -fPIC
TFLAGS = -DMP_THREADS_SUPPORT

LIBRARY	 = mpatrol
VERSION	 = 1
REVISION = 1

OBJECTS	 = list.o tree.o slots.o utils.o memory.o heap.o alloc.o info.o \
	   stack.o addr.o strtab.o symbol.o signals.o diag.o option.o inter.o \
	   malloc.o version.o
TOBJECTS = list.o tree.o slots.o utils.o memory.o heap.o alloc.o info.o \
	   stack.o addr.o strtab.o symbol.o signals.o mutex.o diag.o option.o \
	   inter.o malloc.o version.o

ARCHIVE_OBJECTS	= $(OBJECTS)
SHARED_OBJECTS	= $(OBJECTS:.o=.s.o)
ARCHTS_OBJECTS	= $(TOBJECTS:.o=.t.o)
SHARTS_OBJECTS	= $(TOBJECTS:.o=.s.t.o)

ARCHIVE_LIBRARY	= lib$(LIBRARY).a
SHARED_LIBRARY	= lib$(LIBRARY).so
OBJECT_LIBRARY	= lib$(LIBRARY).o
ARCHTS_LIBRARY	= lib$(LIBRARY)mt.a
SHARTS_LIBRARY	= lib$(LIBRARY)mt.so
OBJTS_LIBRARY	= lib$(LIBRARY)mt.o


# Rules

.SUFFIXES: .c .o .s.o .t.o .s.t.o

.c.o:
	$(CC) $(CFLAGS) $(OFLAGS) -c $<

.c.s.o:
	$(CC) $(CFLAGS) $(OFLAGS) $(SFLAGS) -c -o $@ $<

.c.t.o:
	$(CC) $(CFLAGS) $(OFLAGS) $(TFLAGS) -c -o $@ $<

.c.s.t.o:
	$(CC) $(CFLAGS) $(OFLAGS) $(SFLAGS) $(TFLAGS) -c -o $@ $<

$(ARCHIVE_LIBRARY): $(ARCHIVE_OBJECTS)
	$(AR) cr $@ $(ARCHIVE_OBJECTS)

$(SHARED_LIBRARY).$(VERSION).$(REVISION): $(SHARED_OBJECTS)
	$(LD) -G -h $@ -o $@ $(SHARED_OBJECTS)

$(SHARED_LIBRARY): $(SHARED_LIBRARY).$(VERSION).$(REVISION)
	@ ln -fs $< $@

$(OBJECT_LIBRARY): $(SOURCE)/mpatrol.c
	$(CC) $(CFLAGS) $(OFLAGS) -c -o $@ $<

$(ARCHTS_LIBRARY): $(ARCHTS_OBJECTS)
	$(AR) cr $@ $(ARCHTS_OBJECTS)

$(SHARTS_LIBRARY).$(VERSION).$(REVISION): $(SHARTS_OBJECTS)
	$(LD) -G -h $@ -o $@ $(SHARTS_OBJECTS)

$(SHARTS_LIBRARY): $(SHARTS_LIBRARY).$(VERSION).$(REVISION)
	@ ln -fs $< $@

$(OBJTS_LIBRARY): $(SOURCE)/mpatrol.c
	$(CC) $(CFLAGS) $(OFLAGS) $(TFLAGS) -c -o $@ $<

all: $(ARCHIVE_LIBRARY) $(SHARED_LIBRARY) $(ARCHTS_LIBRARY) $(SHARTS_LIBRARY)

clean:
	-@ rm -f $(ARCHIVE_OBJECTS) $(ARCHTS_OBJECTS) 2>&1 >/dev/null
	-@ rm -f $(SHARED_OBJECTS) $(SHARTS_OBJECTS) 2>&1 >/dev/null

clobber: clean
	-@ rm -f $(ARCHIVE_LIBRARY) $(ARCHTS_LIBRARY) 2>&1 >/dev/null
	-@ rm -f $(SHARED_LIBRARY).$(VERSION).$(REVISION) 2>&1 >/dev/null
	-@ rm -f $(SHARTS_LIBRARY).$(VERSION).$(REVISION) 2>&1 >/dev/null
	-@ rm -f $(SHARED_LIBRARY) $(SHARTS_LIBRARY) 2>&1 >/dev/null
	-@ rm -f $(OBJECT_LIBRARY) $(OBJTS_LIBRARY) 2>&1 >/dev/null


# Dependencies

list.o list.s.o list.t.o list.s.t.o: $(SOURCE)/list.c $(SOURCE)/list.h \
	$(SOURCE)/config.h $(SOURCE)/target.h
tree.o tree.s.o tree.t.o tree.s.t.o: $(SOURCE)/tree.c $(SOURCE)/tree.h \
	$(SOURCE)/config.h $(SOURCE)/target.h
slots.o slots.s.o slots.t.o slots.s.t.o: $(SOURCE)/slots.c $(SOURCE)/slots.h \
	$(SOURCE)/config.h $(SOURCE)/target.h $(SOURCE)/utils.h
utils.o utils.s.o utils.t.o utils.s.t.o: $(SOURCE)/utils.c $(SOURCE)/utils.h \
	$(SOURCE)/config.h $(SOURCE)/target.h
memory.o memory.s.o memory.t.o memory.s.t.o: $(SOURCE)/memory.c \
	$(SOURCE)/memory.h $(SOURCE)/config.h $(SOURCE)/target.h \
	$(SOURCE)/stack.h $(SOURCE)/utils.h
heap.o heap.s.o heap.t.o heap.s.t.o: $(SOURCE)/heap.c $(SOURCE)/heap.h \
	$(SOURCE)/config.h $(SOURCE)/target.h $(SOURCE)/memory.h \
	$(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/utils.h
alloc.o alloc.s.o alloc.t.o alloc.s.t.o: $(SOURCE)/alloc.c $(SOURCE)/alloc.h \
	$(SOURCE)/config.h $(SOURCE)/target.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/utils.h
info.o info.s.o: $(SOURCE)/info.c $(SOURCE)/info.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/addr.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/alloc.h $(SOURCE)/signals.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/diag.h $(SOURCE)/utils.h
info.t.o info.s.t.o: $(SOURCE)/info.c $(SOURCE)/info.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/addr.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/alloc.h $(SOURCE)/signals.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/diag.h \
	$(SOURCE)/mutex.h $(SOURCE)/utils.h
stack.o stack.s.o stack.t.o stack.s.t.o: $(SOURCE)/stack.c $(SOURCE)/stack.h \
	$(SOURCE)/config.h $(SOURCE)/target.h
addr.o addr.s.o addr.t.o addr.s.t.o: $(SOURCE)/addr.c $(SOURCE)/addr.h \
	$(SOURCE)/config.h $(SOURCE)/target.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/utils.h
strtab.o strtab.s.o strtab.t.o strtab.s.t.o: $(SOURCE)/strtab.c \
	$(SOURCE)/strtab.h $(SOURCE)/config.h $(SOURCE)/target.h \
	$(SOURCE)/heap.h $(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h \
	$(SOURCE)/utils.h
symbol.o symbol.s.o symbol.t.o symbol.s.t.o: $(SOURCE)/symbol.c \
	$(SOURCE)/symbol.h $(SOURCE)/config.h $(SOURCE)/target.h \
	$(SOURCE)/heap.h $(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h \
	$(SOURCE)/strtab.h $(SOURCE)/diag.h $(SOURCE)/info.h $(SOURCE)/addr.h \
	$(SOURCE)/list.h $(SOURCE)/stack.h $(SOURCE)/alloc.h \
	$(SOURCE)/signals.h $(SOURCE)/utils.h
signals.o signals.s.o signals.t.o signals.s.t.o: $(SOURCE)/signals.c \
	$(SOURCE)/signals.h $(SOURCE)/config.h $(SOURCE)/target.h \
	$(SOURCE)/diag.h $(SOURCE)/info.h $(SOURCE)/addr.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/alloc.h $(SOURCE)/symbol.h \
	$(SOURCE)/strtab.h $(SOURCE)/inter.h
mutex.t.o mutex.s.t.o: $(SOURCE)/mutex.c $(SOURCE)/mutex.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/inter.h $(SOURCE)/info.h $(SOURCE)/addr.h \
	$(SOURCE)/heap.h $(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h \
	$(SOURCE)/list.h $(SOURCE)/stack.h $(SOURCE)/alloc.h \
	$(SOURCE)/signals.h $(SOURCE)/symbol.h $(SOURCE)/strtab.h
diag.o diag.s.o diag.t.o diag.s.t.o: $(SOURCE)/diag.c $(SOURCE)/diag.h \
	$(SOURCE)/config.h $(SOURCE)/target.h $(SOURCE)/info.h \
	$(SOURCE)/addr.h $(SOURCE)/heap.h $(SOURCE)/memory.h $(SOURCE)/slots.h \
	$(SOURCE)/tree.h $(SOURCE)/list.h $(SOURCE)/stack.h $(SOURCE)/alloc.h \
	$(SOURCE)/signals.h $(SOURCE)/symbol.h $(SOURCE)/strtab.h \
	$(SOURCE)/utils.h $(SOURCE)/version.h
option.o option.s.o option.t.o option.s.t.o: $(SOURCE)/option.c \
	$(SOURCE)/option.h $(SOURCE)/config.h $(SOURCE)/target.h \
	$(SOURCE)/info.h $(SOURCE)/addr.h $(SOURCE)/heap.h $(SOURCE)/memory.h \
	$(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h $(SOURCE)/stack.h \
	$(SOURCE)/alloc.h $(SOURCE)/signals.h $(SOURCE)/symbol.h \
	$(SOURCE)/strtab.h $(SOURCE)/diag.h $(SOURCE)/utils.h
inter.o inter.s.o: $(SOURCE)/inter.c $(SOURCE)/inter.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/info.h $(SOURCE)/addr.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/alloc.h $(SOURCE)/signals.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/diag.h \
	$(SOURCE)/option.h
inter.t.o inter.s.t.o: $(SOURCE)/inter.c $(SOURCE)/inter.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/info.h $(SOURCE)/addr.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/alloc.h $(SOURCE)/signals.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/diag.h \
	$(SOURCE)/mutex.h $(SOURCE)/option.h
malloc.o malloc.s.o malloc.t.o malloc.s.t.o: $(SOURCE)/malloc.c \
	$(SOURCE)/inter.h $(SOURCE)/config.h $(SOURCE)/target.h \
	$(SOURCE)/info.h $(SOURCE)/addr.h $(SOURCE)/heap.h $(SOURCE)/memory.h \
	$(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h $(SOURCE)/stack.h \
	$(SOURCE)/alloc.h $(SOURCE)/signals.h $(SOURCE)/symbol.h \
	$(SOURCE)/strtab.h
version.o version.s.o version.t.o version.s.t.o: $(SOURCE)/version.c \
	$(SOURCE)/version.h $(SOURCE)/config.h $(SOURCE)/target.h
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
	$(SOURCE)/option.h $(SOURCE)/inter.c $(SOURCE)/malloc.c \
	$(SOURCE)/version.c
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
	$(SOURCE)/inter.c $(SOURCE)/malloc.c $(SOURCE)/version.c
