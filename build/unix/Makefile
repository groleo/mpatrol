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


# $Id: Makefile,v 1.22 2000-05-31 20:48:45 graeme Exp $


# Definitions

SHELL = /bin/sh
CC    = gcc
AR    = ar
LD    = ld
LINT  = lint

SOURCE = ../../src
VPATH  = $(SOURCE)

CFLAGS = -I$(SOURCE)
OFLAGS = -O3 -fno-inline-functions
SFLAGS = -fPIC
TFLAGS = -DMP_THREADS_SUPPORT

LIBRARY	 = mpatrol
VERSION	 = 1
REVISION = 2

OBJECTS	 = list.o tree.o slots.o utils.o memory.o heap.o alloc.o info.o \
	   stack.o addr.o strtab.o symbol.o signals.o diag.o option.o \
	   profile.o inter.o malloc.o version.o
TOBJECTS = list.o tree.o slots.o utils.o memory.o heap.o alloc.o info.o \
	   stack.o addr.o strtab.o symbol.o signals.o mutex.o diag.o option.o \
	   profile.o inter.o malloc.o version.o

ARCHIVE_OBJECTS	= $(OBJECTS)
SHARED_OBJECTS	= $(OBJECTS:.o=.s.o)
LINT_OBJECTS	= $(OBJECTS:.o=.ln)
ARCHTS_OBJECTS	= $(TOBJECTS:.o=.t.o)
SHARTS_OBJECTS	= $(TOBJECTS:.o=.s.t.o)
LINTTS_OBJECTS	= $(TOBJECTS:.o=.t.ln)

ARCHIVE_LIBRARY	= lib$(LIBRARY).a
SHARED_LIBRARY	= lib$(LIBRARY).so
OBJECT_LIBRARY	= lib$(LIBRARY).o
LINT_LIBRARY	= llib-l$(LIBRARY).ln
ARCHTS_LIBRARY	= lib$(LIBRARY)mt.a
SHARTS_LIBRARY	= lib$(LIBRARY)mt.so
OBJTS_LIBRARY	= lib$(LIBRARY)mt.o
LINTTS_LIBRARY	= llib-l$(LIBRARY)mt.ln


# Rules

.SUFFIXES: .c .o .s.o .ln .t.o .s.t.o .t.ln

.c.o:
	$(CC) $(CFLAGS) $(OFLAGS) -c $<

.c.s.o:
	$(CC) $(CFLAGS) $(OFLAGS) $(SFLAGS) -c -o $@ $<

.c.ln:
	$(LINT) $(CFLAGS) -y -c $<

.c.t.o:
	$(CC) $(CFLAGS) $(OFLAGS) $(TFLAGS) -c -o $@ $<

.c.s.t.o:
	$(CC) $(CFLAGS) $(OFLAGS) $(SFLAGS) $(TFLAGS) -c -o $@ $<

.c.t.ln:
	$(LINT) $(CFLAGS) $(TFLAGS) -y -C `basename $@ .ln` $<

$(ARCHIVE_LIBRARY): $(ARCHIVE_OBJECTS)
	$(AR) cr $@ $(ARCHIVE_OBJECTS)

$(SHARED_LIBRARY).$(VERSION).$(REVISION): $(SHARED_OBJECTS)
	$(LD) -G -h $@ -o $@ $(SHARED_OBJECTS)

$(SHARED_LIBRARY): $(SHARED_LIBRARY).$(VERSION).$(REVISION)
	@ ln -fs $< $@

$(OBJECT_LIBRARY): $(SOURCE)/library.c
	$(CC) $(CFLAGS) $(OFLAGS) -c -o $@ $<

$(LINT_LIBRARY): $(LINT_OBJECTS)
	$(LINT) -o $(LIBRARY) $(LINT_OBJECTS)

$(ARCHTS_LIBRARY): $(ARCHTS_OBJECTS)
	$(AR) cr $@ $(ARCHTS_OBJECTS)

$(SHARTS_LIBRARY).$(VERSION).$(REVISION): $(SHARTS_OBJECTS)
	$(LD) -G -h $@ -o $@ $(SHARTS_OBJECTS)

$(SHARTS_LIBRARY): $(SHARTS_LIBRARY).$(VERSION).$(REVISION)
	@ ln -fs $< $@

$(OBJTS_LIBRARY): $(SOURCE)/library.c
	$(CC) $(CFLAGS) $(OFLAGS) $(TFLAGS) -c -o $@ $<

$(LINTTS_LIBRARY): $(LINTTS_OBJECTS)
	$(LINT) -o $(LIBRARY)mt $(LINTTS_OBJECTS)

mpatrol: mpatrol.o getopt.o version.o
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ mpatrol.o getopt.o version.o

mprof: mprof.o getopt.o tree.o version.o
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ mprof.o getopt.o tree.o version.o

mleak: mleak.o getopt.o tree.o version.o
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ mleak.o getopt.o tree.o version.o

all: $(ARCHIVE_LIBRARY) $(SHARED_LIBRARY) $(ARCHTS_LIBRARY) $(SHARTS_LIBRARY) \
	mpatrol mprof mleak

lint: $(LINT_LIBRARY) $(LINTTS_LIBRARY)

clean:
	-@ rm -f $(ARCHIVE_OBJECTS) $(ARCHTS_OBJECTS) 2>&1 >/dev/null
	-@ rm -f $(SHARED_OBJECTS) $(SHARTS_OBJECTS) 2>&1 >/dev/null
	-@ rm -f $(LINT_OBJECTS) $(LINTTS_OBJECTS) 2>&1 >/dev/null
	-@ rm -f mpatrol.o mprof.o mleak.o getopt.o 2>&1 >/dev/null

clobber: clean
	-@ rm -f $(ARCHIVE_LIBRARY) $(ARCHTS_LIBRARY) 2>&1 >/dev/null
	-@ rm -f $(SHARED_LIBRARY).$(VERSION).$(REVISION) 2>&1 >/dev/null
	-@ rm -f $(SHARTS_LIBRARY).$(VERSION).$(REVISION) 2>&1 >/dev/null
	-@ rm -f $(SHARED_LIBRARY) $(SHARTS_LIBRARY) 2>&1 >/dev/null
	-@ rm -f $(OBJECT_LIBRARY) $(OBJTS_LIBRARY) 2>&1 >/dev/null
	-@ rm -f $(LINT_LIBRARY) $(LINTTS_LIBRARY) 2>&1 >/dev/null
	-@ rm -f mpatrol mprof mleak 2>&1 >/dev/null


# Dependencies

list.o list.s.o list.ln list.t.o list.s.t.o list.t.ln: $(SOURCE)/list.c \
	$(SOURCE)/list.h $(SOURCE)/config.h $(SOURCE)/target.h
tree.o tree.s.o tree.ln tree.t.o tree.s.t.o tree.t.ln: $(SOURCE)/tree.c \
	$(SOURCE)/tree.h $(SOURCE)/config.h $(SOURCE)/target.h
slots.o slots.s.o slots.ln slots.t.o slots.s.t.o slots.t.ln: $(SOURCE)/slots.c \
	$(SOURCE)/slots.h $(SOURCE)/config.h $(SOURCE)/target.h \
	$(SOURCE)/utils.h
utils.o utils.s.o utils.ln utils.t.o utils.s.t.o utils.t.ln: $(SOURCE)/utils.c \
	$(SOURCE)/utils.h $(SOURCE)/config.h $(SOURCE)/target.h
memory.o memory.s.o memory.ln memory.t.o memory.s.t.o memory.t.ln: \
	$(SOURCE)/memory.c $(SOURCE)/memory.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/stack.h $(SOURCE)/utils.h
heap.o heap.s.o heap.ln heap.t.o heap.s.t.o heap.t.ln: $(SOURCE)/heap.c \
	$(SOURCE)/heap.h $(SOURCE)/config.h $(SOURCE)/target.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/utils.h
alloc.o alloc.s.o alloc.ln alloc.t.o alloc.s.t.o alloc.t.ln: $(SOURCE)/alloc.c \
	$(SOURCE)/alloc.h $(SOURCE)/config.h $(SOURCE)/target.h \
	$(SOURCE)/heap.h $(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h \
	$(SOURCE)/list.h $(SOURCE)/utils.h
info.o info.s.o info.ln: $(SOURCE)/info.c $(SOURCE)/info.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/addr.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/alloc.h $(SOURCE)/signals.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/diag.h \
	$(SOURCE)/utils.h $(SOURCE)/profile.h
info.t.o info.s.t.o info.t.ln: $(SOURCE)/info.c $(SOURCE)/info.h \
	$(SOURCE)/config.h $(SOURCE)/target.h $(SOURCE)/addr.h \
	$(SOURCE)/heap.h $(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h \
	$(SOURCE)/list.h $(SOURCE)/stack.h $(SOURCE)/alloc.h \
	$(SOURCE)/signals.h $(SOURCE)/symbol.h $(SOURCE)/strtab.h \
	$(SOURCE)/diag.h $(SOURCE)/mutex.h $(SOURCE)/utils.h $(SOURCE)/profile.h
stack.o stack.s.o stack.ln stack.t.o stack.s.t.o stack.t.ln: $(SOURCE)/stack.c \
	$(SOURCE)/stack.h $(SOURCE)/memory.h $(SOURCE)/config.h \
	$(SOURCE)/target.h
addr.o addr.s.o addr.ln addr.t.o addr.s.t.o addr.t.ln: $(SOURCE)/addr.c \
	$(SOURCE)/addr.h $(SOURCE)/config.h $(SOURCE)/target.h \
	$(SOURCE)/heap.h $(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h \
	$(SOURCE)/list.h $(SOURCE)/stack.h $(SOURCE)/utils.h
strtab.o strtab.s.o strtab.ln strtab.t.o strtab.s.t.o strtab.t.ln: \
	$(SOURCE)/strtab.c $(SOURCE)/strtab.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/heap.h $(SOURCE)/memory.h \
	$(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h $(SOURCE)/utils.h
symbol.o symbol.s.o symbol.ln symbol.t.o symbol.s.t.o symbol.t.ln: \
	$(SOURCE)/symbol.c $(SOURCE)/symbol.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/heap.h $(SOURCE)/memory.h \
	$(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/strtab.h $(SOURCE)/diag.h \
	$(SOURCE)/info.h $(SOURCE)/addr.h $(SOURCE)/list.h $(SOURCE)/stack.h \
	$(SOURCE)/alloc.h $(SOURCE)/signals.h $(SOURCE)/utils.h \
	$(SOURCE)/profile.h
signals.o signals.s.o signals.ln signals.t.o signals.s.t.o signals.t.ln: \
	$(SOURCE)/signals.c $(SOURCE)/signals.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/diag.h $(SOURCE)/info.h $(SOURCE)/addr.h \
	$(SOURCE)/heap.h $(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h \
	$(SOURCE)/list.h $(SOURCE)/stack.h $(SOURCE)/alloc.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/inter.h \
	$(SOURCE)/profile.h
mutex.t.o mutex.s.t.o mutex.t.ln: $(SOURCE)/mutex.c $(SOURCE)/mutex.h \
	$(SOURCE)/config.h $(SOURCE)/target.h
diag.o diag.s.o diag.ln: $(SOURCE)/diag.c $(SOURCE)/diag.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/info.h $(SOURCE)/addr.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/alloc.h $(SOURCE)/signals.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/utils.h \
	$(SOURCE)/profile.h $(SOURCE)/version.h
diag.t.o diag.s.t.o diag.t.ln: $(SOURCE)/diag.c $(SOURCE)/diag.h \
	$(SOURCE)/config.h $(SOURCE)/target.h $(SOURCE)/info.h \
	$(SOURCE)/addr.h $(SOURCE)/heap.h $(SOURCE)/memory.h \
	$(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h $(SOURCE)/stack.h \
	$(SOURCE)/alloc.h $(SOURCE)/signals.h $(SOURCE)/mutex.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/utils.h \
	$(SOURCE)/profile.h $(SOURCE)/version.h
option.o option.s.o option.ln option.t.o option.s.t.o option.t.ln: \
	$(SOURCE)/option.c $(SOURCE)/option.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/info.h $(SOURCE)/addr.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/alloc.h $(SOURCE)/signals.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/diag.h \
	$(SOURCE)/utils.h $(SOURCE)/profile.h
profile.o profile.s.o profile.ln profile.t.o profile.s.t.o profile.t.ln: \
	$(SOURCE)/profile.c $(SOURCE)/profile.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/alloc.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/info.h $(SOURCE)/addr.h $(SOURCE)/stack.h \
	$(SOURCE)/signals.h $(SOURCE)/symbol.h $(SOURCE)/strtab.h \
	$(SOURCE)/diag.h $(SOURCE)/utils.h
inter.o inter.s.o inter.ln: $(SOURCE)/inter.c $(SOURCE)/inter.h \
	$(SOURCE)/config.h $(SOURCE)/target.h $(SOURCE)/info.h \
	$(SOURCE)/addr.h $(SOURCE)/heap.h $(SOURCE)/memory.h $(SOURCE)/slots.h \
	$(SOURCE)/tree.h $(SOURCE)/list.h $(SOURCE)/stack.h $(SOURCE)/alloc.h \
	$(SOURCE)/signals.h $(SOURCE)/symbol.h $(SOURCE)/strtab.h \
	$(SOURCE)/diag.h $(SOURCE)/option.h $(SOURCE)/profile.h
inter.t.o inter.s.t.o inter.t.ln: $(SOURCE)/inter.c $(SOURCE)/inter.h \
	$(SOURCE)/config.h $(SOURCE)/target.h $(SOURCE)/info.h \
	$(SOURCE)/addr.h $(SOURCE)/heap.h $(SOURCE)/memory.h $(SOURCE)/slots.h \
	$(SOURCE)/tree.h $(SOURCE)/list.h $(SOURCE)/stack.h $(SOURCE)/alloc.h \
	$(SOURCE)/signals.h $(SOURCE)/symbol.h $(SOURCE)/strtab.h \
	$(SOURCE)/diag.h $(SOURCE)/mutex.h $(SOURCE)/option.h \
	$(SOURCE)/profile.h
malloc.o malloc.s.o malloc.ln malloc.t.o malloc.s.t.o malloc.t.ln: \
	$(SOURCE)/malloc.c $(SOURCE)/inter.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/info.h $(SOURCE)/addr.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/alloc.h $(SOURCE)/signals.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/profile.h
version.o version.s.o version.ln version.t.o version.s.t.o version.t.ln: \
	$(SOURCE)/version.c $(SOURCE)/version.h $(SOURCE)/config.h \
	$(SOURCE)/target.h
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
	$(SOURCE)/option.h $(SOURCE)/inter.c $(SOURCE)/malloc.c \
	$(SOURCE)/profile.c $(SOURCE)/profile.h $(SOURCE)/version.c
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
	$(SOURCE)/inter.c $(SOURCE)/malloc.c $(SOURCE)/profile.c \
	$(SOURCE)/profile.h $(SOURCE)/version.c
mpatrol.o: $(SOURCE)/mpatrol.c $(SOURCE)/getopt.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/version.h
mprof.o: $(SOURCE)/mprof.c $(SOURCE)/getopt.h $(SOURCE)/tree.h \
	$(SOURCE)/config.h $(SOURCE)/target.h $(SOURCE)/version.h
mleak.o: $(SOURCE)/mleak.c $(SOURCE)/getopt.h $(SOURCE)/tree.h \
	$(SOURCE)/config.h $(SOURCE)/target.h $(SOURCE)/version.h
getopt.o: $(SOURCE)/getopt.c $(SOURCE)/getopt.h $(SOURCE)/config.h \
	$(SOURCE)/target.h
