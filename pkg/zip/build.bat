@echo off


rem mpatrol
rem A library for controlling and tracing dynamic memory allocations.
rem Copyright (C) 1997-2000 Graeme S. Roy <graeme@epc.co.uk>
rem
rem This library is free software; you can redistribute it and/or
rem modify it under the terms of the GNU Library General Public
rem License as published by the Free Software Foundation; either
rem version 2 of the License, or (at your option) any later version.
rem
rem This library is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
rem Library General Public License for more details.
rem
rem You should have received a copy of the GNU Library General Public
rem License along with this library; if not, write to the Free
rem Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
rem MA 02111-1307, USA.


rem DOS batch file to build a zip distribution file


rem $Id: build.bat,v 1.21 2000-12-18 21:56:14 graeme Exp $


set version=1.3.1


rem Build the mpatrol library.

cd ..\..\build\windows
nmake -f NMakefile clobber
nmake -f NMakefile libmpatrol.lib mpatrol.dll
nmake -f NMakefile libmpatrolmt.lib mpatrolmt.dll
nmake -f NMakefile mpatrol.exe mprof.exe mptrace.exe mleak.exe
cd ..\..\pkg\zip


rem Create the distribution directory structure and copy the necessary
rem files into it.  This assumes that the Texinfo manual, reference card
rem and manual pages have already been processed into the different
rem documentation formats.

rmdir /s /q mpatrol
mkdir mpatrol
copy ..\..\README mpatrol\README.txt
copy ..\..\COPYING mpatrol\COPYING.txt
copy ..\..\COPYING.LIB mpatrol\COPYING_LIB.txt
copy ..\..\NEWS mpatrol\NEWS.txt
copy ..\..\ChangeLog mpatrol\ChangeLog.txt
mkdir mpatrol\bin
copy ..\..\build\windows\mpatrol.exe mpatrol\bin
copy ..\..\build\windows\mprof.exe mpatrol\bin
copy ..\..\build\windows\mptrace.exe mpatrol\bin
copy ..\..\build\windows\mleak.exe mpatrol\bin
copy ..\..\bin\mpsym mpatrol\bin
copy ..\..\bin\mpedit mpatrol\bin
copy ..\..\build\windows\mpatrol.dll mpatrol\bin
copy ..\..\build\windows\mpatrolmt.dll mpatrol\bin
mkdir mpatrol\doc
copy ..\..\doc\README mpatrol\doc\README.txt
copy ..\..\doc\mpatrol.txt mpatrol\doc
copy ..\..\doc\mpatrol.info mpatrol\doc
copy ..\..\doc\mpatrol.guide mpatrol\doc
copy ..\..\doc\mpatrol.html mpatrol\doc
copy ..\..\doc\mpatrol.dvi mpatrol\doc
copy ..\..\doc\mpatrol.ps mpatrol\doc
copy ..\..\doc\mpatrol.pdf mpatrol\doc
copy ..\..\doc\refcard.dvi mpatrol\doc
copy ..\..\doc\refcard.ps mpatrol\doc
copy ..\..\doc\refcard.pdf mpatrol\doc
mkdir mpatrol\doc\images
copy ..\..\doc\images\mpatrol.txt mpatrol\doc\images
copy ..\..\doc\images\mpatrol.jpg mpatrol\doc\images
copy ..\..\doc\images\mpatrol.eps mpatrol\doc\images
copy ..\..\doc\images\mpatrol.pdf mpatrol\doc\images
copy ..\..\doc\images\test.jpg mpatrol\doc\images
copy ..\..\doc\images\test.eps mpatrol\doc\images
copy ..\..\doc\images\test.pdf mpatrol\doc\images
copy ..\..\doc\images\gcc.jpg mpatrol\doc\images
copy ..\..\doc\images\gcc.eps mpatrol\doc\images
copy ..\..\doc\images\gcc.pdf mpatrol\doc\images
copy ..\..\doc\images\cpp.jpg mpatrol\doc\images
copy ..\..\doc\images\cpp.eps mpatrol\doc\images
copy ..\..\doc\images\cpp.pdf mpatrol\doc\images
copy ..\..\doc\images\cc1.jpg mpatrol\doc\images
copy ..\..\doc\images\cc1.eps mpatrol\doc\images
copy ..\..\doc\images\cc1.pdf mpatrol\doc\images
copy ..\..\doc\images\collect2.jpg mpatrol\doc\images
copy ..\..\doc\images\collect2.eps mpatrol\doc\images
copy ..\..\doc\images\collect2.pdf mpatrol\doc\images
copy ..\..\doc\images\trace.jpg mpatrol\doc\images
copy ..\..\doc\images\trace.eps mpatrol\doc\images
copy ..\..\doc\images\trace.pdf mpatrol\doc\images
mkdir mpatrol\doc\man
copy ..\..\man\README mpatrol\doc\man\README.txt
mkdir mpatrol\doc\man\dvi
copy ..\..\man\dvi\mpatrol.dvi mpatrol\doc\man\dvi
copy ..\..\man\dvi\mprof.dvi mpatrol\doc\man\dvi
copy ..\..\man\dvi\mptrace.dvi mpatrol\doc\man\dvi
copy ..\..\man\dvi\mleak.dvi mpatrol\doc\man\dvi
copy ..\..\man\dvi\mpsym.dvi mpatrol\doc\man\dvi
copy ..\..\man\dvi\mpedit.dvi mpatrol\doc\man\dvi
copy ..\..\man\dvi\libmpatrol.dvi mpatrol\doc\man\dvi
mkdir mpatrol\doc\man\ps
copy ..\..\man\ps\mpatrol.ps mpatrol\doc\man\ps
copy ..\..\man\ps\mprof.ps mpatrol\doc\man\ps
copy ..\..\man\ps\mptrace.ps mpatrol\doc\man\ps
copy ..\..\man\ps\mleak.ps mpatrol\doc\man\ps
copy ..\..\man\ps\mpsym.ps mpatrol\doc\man\ps
copy ..\..\man\ps\mpedit.ps mpatrol\doc\man\ps
copy ..\..\man\ps\libmpatrol.ps mpatrol\doc\man\ps
mkdir mpatrol\doc\man\pdf
copy ..\..\man\pdf\mpatrol.pdf mpatrol\doc\man\pdf
copy ..\..\man\pdf\mprof.pdf mpatrol\doc\man\pdf
copy ..\..\man\pdf\mptrace.pdf mpatrol\doc\man\pdf
copy ..\..\man\pdf\mleak.pdf mpatrol\doc\man\pdf
copy ..\..\man\pdf\mpsym.pdf mpatrol\doc\man\pdf
copy ..\..\man\pdf\mpedit.pdf mpatrol\doc\man\pdf
copy ..\..\man\pdf\libmpatrol.pdf mpatrol\doc\man\pdf
mkdir mpatrol\include
copy ..\..\src\mpatrol.h mpatrol\include
mkdir mpatrol\lib
copy ..\..\build\windows\libmpatrol.lib mpatrol\lib
copy ..\..\build\windows\libmpatrolmt.lib mpatrol\lib
copy ..\..\build\windows\mpatrol.lib mpatrol\lib
copy ..\..\build\windows\mpatrol.exp mpatrol\lib
copy ..\..\build\windows\mpatrolmt.lib mpatrol\lib
copy ..\..\build\windows\mpatrolmt.exp mpatrol\lib
mkdir mpatrol\man
mkdir mpatrol\man\cat1
copy ..\..\man\cat1\mpatrol.1 mpatrol\man\cat1
copy ..\..\man\cat1\mprof.1 mpatrol\man\cat1
copy ..\..\man\cat1\mptrace.1 mpatrol\man\cat1
copy ..\..\man\cat1\mleak.1 mpatrol\man\cat1
copy ..\..\man\cat1\mpsym.1 mpatrol\man\cat1
copy ..\..\man\cat1\mpedit.1 mpatrol\man\cat1
mkdir mpatrol\man\man1
copy ..\..\man\man1\mpatrol.1 mpatrol\man\man1
copy ..\..\man\man1\mprof.1 mpatrol\man\man1
copy ..\..\man\man1\mptrace.1 mpatrol\man\man1
copy ..\..\man\man1\mleak.1 mpatrol\man\man1
copy ..\..\man\man1\mpsym.1 mpatrol\man\man1
copy ..\..\man\man1\mpedit.1 mpatrol\man\man1
mkdir mpatrol\man\cat3
copy ..\..\man\cat3\libmpatrol.3 mpatrol\man\cat3
mkdir mpatrol\man\man3
copy ..\..\man\man3\libmpatrol.3 mpatrol\man\man3
mkdir mpatrol\tests
mkdir mpatrol\tests\pass
copy ..\..\tests\pass\test*.c mpatrol\tests\pass
mkdir mpatrol\tests\fail
copy ..\..\tests\fail\test*.c mpatrol\tests\fail
mkdir mpatrol\tests\profile
copy ..\..\tests\profile\test*.c mpatrol\tests\profile
mkdir mpatrol\tests\tutorial
copy ..\..\tests\tutorial\test*.c mpatrol\tests\tutorial


rem Create the ZIP distribution archive.

del /f /q mpatrol_%version%.zip
wzzip -P -r mpatrol_%version%.zip mpatrol
rmdir /s /q mpatrol


rem Clean up the build directory.

cd ..\..\build\windows
nmake -f NMakefile clobber
cd ..\..\pkg\zip
