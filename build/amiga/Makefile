# mpatrol
# A library for controlling and tracing dynamic memory allocations.
# Copyright (C) 1997-2001 Graeme S. Roy <graeme@epc.co.uk>
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


# GNU Makefile to compile on AmigaOS with GCC


# $Id: Makefile,v 1.37 2001-01-22 20:40:11 graeme Exp $


# Definitions

CC = gcc
AR = ar

SOURCE = ../../src
VPATH  = $(SOURCE)

CFLAGS = -I$(SOURCE)
OFLAGS = -O3 -fno-inline-functions
TFLAGS = -DMP_THREADS_SUPPORT

MPATROL	= mpatrol

ARCHIVE_OBJECTS	= list.o tree.o slots.o utils.o memory.o heap.o alloc.o info.o \
		  stack.o addr.o strtab.o symbol.o signals.o diag.o option.o \
		  profile.o trace.o inter.o malloc.o sbrk.o version.o
ARCHTS_OBJECTS	= list.t.o tree.t.o slots.t.o utils.t.o memory.t.o heap.t.o \
		  alloc.t.o info.t.o stack.t.o addr.t.o strtab.t.o symbol.t.o \
		  signals.t.o mutex.t.o diag.t.o option.t.o profile.t.o \
		  trace.t.o inter.t.o malloc.t.o sbrk.t.o version.t.o

ARCHIVE_MPATROL	= lib$(MPATROL).a
OBJECT_MPATROL	= lib$(MPATROL).o
ARCHTS_MPATROL	= lib$(MPATROL)mt.a
OBJTS_MPATROL	= lib$(MPATROL)mt.o


# Rules

.SUFFIXES: .c .o .t.o

.c.o:
	$(CC) $(CFLAGS) $(OFLAGS) -c $<

.c.t.o:
	$(CC) $(CFLAGS) $(OFLAGS) $(TFLAGS) -c -o $@ $<

$(ARCHIVE_MPATROL): $(ARCHIVE_OBJECTS)
	$(AR) cr $@ $(ARCHIVE_OBJECTS)

$(OBJECT_MPATROL): $(SOURCE)/library.c
	$(CC) $(CFLAGS) $(OFLAGS) -c -o $@ $<

$(ARCHTS_MPATROL): $(ARCHTS_OBJECTS)
	$(AR) cr $@ $(ARCHTS_OBJECTS)

$(OBJTS_MPATROL): $(SOURCE)/library.c
	$(CC) $(CFLAGS) $(OFLAGS) $(TFLAGS) -c -o $@ $<

mpatrol: mpatrol.o getopt.o version.o
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ mpatrol.o getopt.o version.o

mprof: mprof.o graph.o getopt.o list.o tree.o version.o
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ mprof.o graph.o getopt.o list.o tree.o version.o

mptrace: mptrace.o getopt.o tree.o utils.o version.o
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ mptrace.o getopt.o tree.o utils.o version.o

mleak: mleak.o getopt.o tree.o version.o
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ mleak.o getopt.o tree.o version.o

all: $(ARCHIVE_MPATROL) $(ARCHTS_MPATROL) mpatrol mprof mptrace mleak

clean:
	-@ rm -f $(ARCHIVE_OBJECTS) $(ARCHTS_OBJECTS) 2>&1 >/dev/null
	-@ rm -f mpatrol.o mprof.o mptrace.o mleak.o graph.o getopt.o 2>&1 >/dev/null

clobber: clean
	-@ rm -f $(ARCHIVE_MPATROL) $(ARCHTS_MPATROL) 2>&1 >/dev/null
	-@ rm -f $(OBJECT_MPATROL) $(OBJTS_MPATROL) 2>&1 >/dev/null
	-@ rm -f mpatrol mprof mptrace mleak 2>&1 >/dev/null


# Dependencies

list.o list.t.o: $(SOURCE)/list.c $(SOURCE)/list.h $(SOURCE)/config.h \
	$(SOURCE)/target.h
tree.o tree.t.o: $(SOURCE)/tree.c $(SOURCE)/tree.h $(SOURCE)/config.h \
	$(SOURCE)/target.h
slots.o slots.t.o: $(SOURCE)/slots.c $(SOURCE)/slots.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/utils.h
utils.o utils.t.o: $(SOURCE)/utils.c $(SOURCE)/utils.h $(SOURCE)/config.h \
	$(SOURCE)/target.h
memory.o memory.t.o: $(SOURCE)/memory.c $(SOURCE)/memory.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/stack.h $(SOURCE)/utils.h
heap.o heap.t.o: $(SOURCE)/heap.c $(SOURCE)/heap.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/memory.h $(SOURCE)/slots.h \
	$(SOURCE)/tree.h $(SOURCE)/trace.h $(SOURCE)/utils.h
alloc.o alloc.t.o: $(SOURCE)/alloc.c $(SOURCE)/alloc.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/heap.h $(SOURCE)/memory.h \
	$(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h $(SOURCE)/utils.h
info.o: $(SOURCE)/info.c $(SOURCE)/info.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/addr.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/alloc.h $(SOURCE)/signals.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/diag.h \
	$(SOURCE)/utils.h $(SOURCE)/profile.h $(SOURCE)/trace.h
info.t.o: $(SOURCE)/info.c $(SOURCE)/info.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/addr.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/alloc.h $(SOURCE)/signals.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/diag.h \
	$(SOURCE)/mutex.h $(SOURCE)/utils.h $(SOURCE)/profile.h \
	$(SOURCE)/trace.h
stack.o stack.t.o: $(SOURCE)/stack.c $(SOURCE)/stack.h $(SOURCE)/memory.h \
	$(SOURCE)/machine.h $(SOURCE)/config.h $(SOURCE)/target.h
addr.o addr.t.o: $(SOURCE)/addr.c $(SOURCE)/addr.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/heap.h $(SOURCE)/memory.h \
	$(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h $(SOURCE)/stack.h \
	$(SOURCE)/utils.h
strtab.o strtab.t.o: $(SOURCE)/strtab.c $(SOURCE)/strtab.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/heap.h $(SOURCE)/memory.h \
	$(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h $(SOURCE)/utils.h
symbol.o symbol.t.o: $(SOURCE)/symbol.c $(SOURCE)/symbol.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/heap.h $(SOURCE)/memory.h \
	$(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/strtab.h $(SOURCE)/diag.h \
	$(SOURCE)/info.h $(SOURCE)/addr.h $(SOURCE)/list.h $(SOURCE)/stack.h \
	$(SOURCE)/alloc.h $(SOURCE)/signals.h $(SOURCE)/utils.h \
	$(SOURCE)/profile.h $(SOURCE)/trace.h
signals.o: $(SOURCE)/signals.c $(SOURCE)/signals.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/diag.h $(SOURCE)/info.h $(SOURCE)/addr.h \
	$(SOURCE)/heap.h $(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h \
	$(SOURCE)/list.h $(SOURCE)/stack.h $(SOURCE)/alloc.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/inter.h \
	$(SOURCE)/profile.h $(SOURCE)/trace.h
signals.t.o: $(SOURCE)/signals.c $(SOURCE)/signals.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/diag.h $(SOURCE)/info.h $(SOURCE)/addr.h \
	$(SOURCE)/heap.h $(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h \
	$(SOURCE)/list.h $(SOURCE)/stack.h $(SOURCE)/alloc.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/inter.h \
	$(SOURCE)/profile.h $(SOURCE)/trace.h $(SOURCE)/mutex.h
mutex.t.o: $(SOURCE)/mutex.c $(SOURCE)/mutex.h $(SOURCE)/config.h \
	$(SOURCE)/target.h
diag.o: $(SOURCE)/diag.c $(SOURCE)/diag.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/info.h $(SOURCE)/addr.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/alloc.h $(SOURCE)/signals.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/utils.h \
	$(SOURCE)/profile.h $(SOURCE)/trace.h $(SOURCE)/version.h
diag.t.o: $(SOURCE)/diag.c $(SOURCE)/diag.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/info.h $(SOURCE)/addr.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/alloc.h $(SOURCE)/signals.h \
	$(SOURCE)/mutex.h $(SOURCE)/symbol.h $(SOURCE)/strtab.h \
	$(SOURCE)/utils.h $(SOURCE)/profile.h $(SOURCE)/trace.h \
	$(SOURCE)/version.h
option.o option.t.o: $(SOURCE)/option.c $(SOURCE)/option.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/info.h $(SOURCE)/addr.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/alloc.h $(SOURCE)/signals.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/diag.h \
	$(SOURCE)/utils.h $(SOURCE)/profile.h $(SOURCE)/trace.h
profile.o profile.t.o: $(SOURCE)/profile.c $(SOURCE)/profile.h \
	$(SOURCE)/config.h $(SOURCE)/target.h $(SOURCE)/alloc.h \
	$(SOURCE)/heap.h $(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h \
	$(SOURCE)/list.h $(SOURCE)/info.h $(SOURCE)/addr.h $(SOURCE)/stack.h \
	$(SOURCE)/signals.h $(SOURCE)/symbol.h $(SOURCE)/strtab.h \
	$(SOURCE)/diag.h $(SOURCE)/utils.h $(SOURCE)/trace.h $(SOURCE)/version.h
trace.o trace.t.o: $(SOURCE)/trace.c $(SOURCE)/trace.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/memory.h $(SOURCE)/diag.h \
	$(SOURCE)/info.h $(SOURCE)/alloc.h $(SOURCE)/heap.h $(SOURCE)/slots.h \
	$(SOURCE)/tree.h $(SOURCE)/list.h $(SOURCE)/addr.h $(SOURCE)/stack.h \
	$(SOURCE)/profile.h $(SOURCE)/symbol.h $(SOURCE)/strtab.h \
	$(SOURCE)/signals.h $(SOURCE)/utils.h $(SOURCE)/version.h
inter.o: $(SOURCE)/inter.c $(SOURCE)/inter.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/info.h $(SOURCE)/addr.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/alloc.h $(SOURCE)/signals.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/diag.h \
	$(SOURCE)/option.h $(SOURCE)/sbrk.h $(SOURCE)/profile.h \
	$(SOURCE)/trace.h $(SOURCE)/machine.h
inter.t.o: $(SOURCE)/inter.c $(SOURCE)/inter.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/info.h $(SOURCE)/addr.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/alloc.h $(SOURCE)/signals.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/diag.h \
	$(SOURCE)/mutex.h $(SOURCE)/option.h $(SOURCE)/sbrk.h \
	$(SOURCE)/profile.h $(SOURCE)/trace.h $(SOURCE)/machine.h
malloc.o malloc.t.o: $(SOURCE)/malloc.c $(SOURCE)/inter.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/info.h $(SOURCE)/addr.h $(SOURCE)/heap.h \
	$(SOURCE)/memory.h $(SOURCE)/slots.h $(SOURCE)/tree.h $(SOURCE)/list.h \
	$(SOURCE)/stack.h $(SOURCE)/alloc.h $(SOURCE)/signals.h \
	$(SOURCE)/symbol.h $(SOURCE)/strtab.h $(SOURCE)/profile.h \
	$(SOURCE)/trace.h
sbrk.o sbrk.t.o: $(SOURCE)/sbrk.c $(SOURCE)/sbrk.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/memory.h $(SOURCE)/utils.h
version.o version.t.o: $(SOURCE)/version.c $(SOURCE)/version.h \
	$(SOURCE)/config.h $(SOURCE)/target.h
$(OBJECT_MPATROL): $(SOURCE)/library.c $(SOURCE)/config.h $(SOURCE)/target.h \
	$(SOURCE)/list.c $(SOURCE)/list.h $(SOURCE)/tree.c $(SOURCE)/tree.h \
	$(SOURCE)/slots.c $(SOURCE)/slots.h $(SOURCE)/utils.h \
	$(SOURCE)/utils.c $(SOURCE)/memory.c $(SOURCE)/memory.h \
	$(SOURCE)/stack.h $(SOURCE)/heap.c $(SOURCE)/heap.h $(SOURCE)/alloc.c \
	$(SOURCE)/alloc.h $(SOURCE)/info.c $(SOURCE)/info.h $(SOURCE)/addr.h \
	$(SOURCE)/signals.h $(SOURCE)/symbol.h $(SOURCE)/strtab.h \
	$(SOURCE)/diag.h $(SOURCE)/stack.c $(SOURCE)/addr.c $(SOURCE)/strtab.c \
	$(SOURCE)/symbol.c $(SOURCE)/signals.c $(SOURCE)/inter.h \
	$(SOURCE)/diag.c $(SOURCE)/version.h $(SOURCE)/option.c \
	$(SOURCE)/option.h $(SOURCE)/inter.c $(SOURCE)/profile.c \
	$(SOURCE)/profile.h $(SOURCE)/trace.c $(SOURCE)/trace.h \
	$(SOURCE)/version.c
$(OBJTS_MPATROL): $(SOURCE)/library.c $(SOURCE)/config.h $(SOURCE)/target.h \
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
	$(SOURCE)/inter.c $(SOURCE)/profile.c $(SOURCE)/profile.h \
	$(SOURCE)/trace.c $(SOURCE)/trace.h $(SOURCE)/version.c
mpatrol.o: $(SOURCE)/mpatrol.c $(SOURCE)/getopt.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/version.h
mprof.o: $(SOURCE)/mprof.c $(SOURCE)/getopt.h $(SOURCE)/tree.h \
	$(SOURCE)/graph.h $(SOURCE)/list.h $(SOURCE)/config.h \
	$(SOURCE)/target.h $(SOURCE)/version.h
mptrace.o: $(SOURCE)/mptrace.c $(SOURCE)/getopt.h $(SOURCE)/tree.h \
	$(SOURCE)/config.h $(SOURCE)/target.h $(SOURCE)/utils.h \
	$(SOURCE)/version.h
mleak.o: $(SOURCE)/mleak.c $(SOURCE)/getopt.h $(SOURCE)/tree.h \
	$(SOURCE)/config.h $(SOURCE)/target.h $(SOURCE)/version.h
graph.o: $(SOURCE)/graph.c $(SOURCE)/graph.h $(SOURCE)/list.h \
	$(SOURCE)/config.h $(SOURCE)/target.h
getopt.o: $(SOURCE)/getopt.c $(SOURCE)/getopt.h $(SOURCE)/config.h \
	$(SOURCE)/target.h
