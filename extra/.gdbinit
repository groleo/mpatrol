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


# GDB command file for use with mpatrol


# $Id: .gdbinit,v 1.5 2001-02-14 01:03:44 graeme Exp $


# Set mpatrol library options in the current process environment.
# GDB doesn't support commands with variable arguments so we have
# to make use of the shell in order to set the new environment.
# Note that there is a known problem when using option arguments
# that contain spaces, so try to avoid using such arguments.

define mallopt
printf "Enter mpatrol library options: "
shell read arg; echo set environment MPATROL_OPTIONS `mpatrol --show-env $arg` >/tmp/mpatrol.gdb
source /tmp/mpatrol.gdb
shell rm -f /tmp/mpatrol.gdb
show environment MPATROL_OPTIONS
end
document mallopt
Sets mpatrol library options in the current process environment.
end


# Display information about an address in the heap.

define printalloc
call __mp_printinfo($arg0)
end
document printalloc
Displays information about an address in the heap.
end


# Break at a specific allocation index.

define allocstop
call __mp_setoption(-3, $arg0)
end
document allocstop
Breaks at a specific allocation index.
end


# Break at a specific reallocation index.

define reallocstop
call __mp_setoption(-4, $arg0)
end
document reallocstop
Breaks at a specific reallocation index.
end


# Break when a specific allocation is freed.

define freestop
call __mp_setoption(-5, $arg0)
end
document freestop
Breaks when a specific allocation is freed.
end


# Start the process by running until it reaches _start() since we cannot
# set a breakpoint at __mp_trap() if it exists in a shared library until
# the process has been started.  The _start() function exists on most
# UNIX systems and it normally calls _init(), main() and then _fini().
# If your system does not have _start() then change it to main() instead.

break _start
run
break __mp_trap
