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


rem $Id: build.bat,v 1.6 2000-07-18 22:59:11 graeme Exp $


set version=1.3.0


rem Build the mpatrol library.

cd ..\..\build\windows
nmake -f NMakefile clobber
nmake -f NMakefile libmpatrol.lib mpatrol.dll
nmake -f NMakefile libmpatrolmt.lib mpatrolmt.dll
nmake -f NMakefile mpatrol.exe mprof.exe mleak.exe
cd ..\..\pkg\zip


rem Create the distribution directory structure and copy the necessary
rem files into it.  This assumes that the Texinfo manual has already been
rem processed into the different documentation formats.

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
copy ..\..\build\windows\mleak.exe mpatrol\bin
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
mkdir mpatrol\include
copy ..\..\src\mpatrol.h mpatrol\include
mkdir mpatrol\lib
copy ..\..\build\windows\libmpatrol.lib mpatrol\lib
copy ..\..\build\windows\libmpatrolmt.lib mpatrol\lib
copy ..\..\build\windows\mpatrol.lib mpatrol\lib
copy ..\..\build\windows\mpatrol.exp mpatrol\lib
copy ..\..\build\windows\mpatrolmt.lib mpatrol\lib
copy ..\..\build\windows\mpatrolmt.exp mpatrol\lib
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
pkzip -add -dir=current mpatrol_%version%.zip mpatrol\*
rmdir /s /q mpatrol


rem Clean up the build directory.

cd ..\..\build\windows
nmake -f NMakefile clobber
cd ..\..\pkg\zip
