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


# RPM package specification file


# $Id: mpatrol.spec,v 1.27 2000-07-18 22:59:07 graeme Exp $


%define version 1.3.0
%define libversion 1.3


Summary: A library for controlling and tracing dynamic memory allocations
Name: mpatrol
Version: %{version}
Release: 1
Copyright: LGPL
Group: Development/Debuggers
Source0: http://www.cbmamiga.demon.co.uk/mpatrol/files/mpatrol_%{version}.tar.gz
Source1: http://www.cbmamiga.demon.co.uk/mpatrol/files/mpatrol_doc.tar.gz
URL: http://www.cbmamiga.demon.co.uk/mpatrol/index.html
Buildroot: /var/tmp/mpatrol-root
Prereq: /sbin/install-info /sbin/ldconfig
Packager: Graeme S. Roy <graeme@epc.co.uk>


%description
A link library that attempts to diagnose run-time errors that are caused
by the wrong use of dynamically allocated memory.  Along with providing a
comprehensive and configurable log of all dynamic memory operations that
occurred during the lifetime of a program, the mpatrol library performs
extensive checking to detect any misuse of dynamically allocated memory.
All of this functionality can be integrated into existing code through
the inclusion of a single header file at compile-time.  All logging and
tracing output from the mpatrol library is sent to a separate log file in
order to keep its diagnostics separate from any that the program being
tested might generate.  A wide variety of library settings can also be
changed at run-time via an environment variable, thus removing the need
to recompile or relink in order to change the library's behaviour.


%prep
%setup -q -n mpatrol -b 1


%build
cd build/unix
make libmpatrol.a libmpatrol.so.%{libversion}
make libmpatrolmt.a libmpatrolmt.so.%{libversion}
make mpatrol mprof mleak


%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/bin
cp build/unix/mpatrol $RPM_BUILD_ROOT/usr/bin
cp build/unix/mprof $RPM_BUILD_ROOT/usr/bin
cp build/unix/mleak $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}/images
cp README $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}
cp doc/README $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}/README.DOC
cp COPYING $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}
cp COPYING.LIB $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}
cp NEWS $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}
cp ChangeLog $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}
cp doc/mpatrol.txt $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}
cp doc/mpatrol.guide $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}
cp doc/mpatrol.html $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}
cp doc/mpatrol.dvi $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}
cp doc/mpatrol.ps $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}
cp doc/mpatrol.pdf $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}
cp doc/refcard.dvi $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}
cp doc/refcard.ps $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}
cp doc/refcard.pdf $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}
cp doc/images/mpatrol.txt $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}/images
cp doc/images/mpatrol.jpg $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}/images
cp doc/images/mpatrol.eps $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}/images
cp doc/images/mpatrol.pdf $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}/images
mkdir -p $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}/tests/pass
cp tests/pass/test*.c $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}/tests/pass
mkdir -p $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}/tests/fail
cp tests/fail/test*.c $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}/tests/fail
mkdir -p $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}/tests/profile
cp tests/profile/test*.c $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}/tests/profile
mkdir -p $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}/tests/tutorial
cp tests/tutorial/test*.c $RPM_BUILD_ROOT/usr/doc/mpatrol-%{version}/tests/tutorial
mkdir -p $RPM_BUILD_ROOT/usr/include
cp src/mpatrol.h $RPM_BUILD_ROOT/usr/include
mkdir -p $RPM_BUILD_ROOT/usr/info
cp doc/mpatrol.info $RPM_BUILD_ROOT/usr/info
mkdir -p $RPM_BUILD_ROOT/usr/lib
cp build/unix/libmpatrol.a $RPM_BUILD_ROOT/usr/lib
cp build/unix/libmpatrol.so.%{libversion} $RPM_BUILD_ROOT/usr/lib
cp build/unix/libmpatrolmt.a $RPM_BUILD_ROOT/usr/lib
cp build/unix/libmpatrolmt.so.%{libversion} $RPM_BUILD_ROOT/usr/lib
mkdir -p $RPM_BUILD_ROOT/usr/man/man1
cp man/man1/mpatrol.1 $RPM_BUILD_ROOT/usr/man/man1
cp man/man1/mprof.1 $RPM_BUILD_ROOT/usr/man/man1
cp man/man1/mleak.1 $RPM_BUILD_ROOT/usr/man/man1
mkdir -p $RPM_BUILD_ROOT/usr/man/man3
cp man/man3/mpatrol.3 $RPM_BUILD_ROOT/usr/man/man3


%files
/usr/bin
/usr/doc
/usr/include
/usr/info
/usr/lib
/usr/man


%post
/sbin/ldconfig
/sbin/install-info /usr/info/mpatrol.info /usr/info/dir


%preun
if [ $1 = 0 ]
then
    /sbin/install-info --delete /usr/info/mpatrol.info /usr/info/dir
fi


%postun
if [ $1 = 0 ]
then
    /sbin/ldconfig
fi


%clean
rm -rf $RPM_BUILD_ROOT
