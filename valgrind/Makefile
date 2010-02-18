TARGET = i586-mingw32msvc-

all:
	$(TARGET)gcc -g -Wall -ansi -pedantic -o valgrind.exe valgrind.c
	$(TARGET)gcc -g -Wall -DPSAPI_VERSION=1 -c -o valgrind_dll.o valgrind_dll.c
	$(TARGET)gcc -shared -Wl,-soname,valgrind_dll.dll -Wl,--out-implib,libvalgrind_dll.dll.a -o valgrind_dll.dll valgrind_dll.o -lpsapi -limagehlp
	$(TARGET)gcc -g -Wall -ansi -pedantic -o valgrind_test.exe valgrind_test.c

clean:
	rm -f *.o *.a *.exe *.dll

dist:
	mkdir valgrind/
	mkdir valgrind/win32/
	rm -f valgrind.tar.bz2
	cp valgrind.c valgrind_dll.c valgrind_test.c Makefile valgrind/
	cp win32/valgrind.sln win32/valgrind.vcproj win32/valgrind_dll.vcproj win32/valgrind_test.vcproj valgrind/win32
	tar cvf valgrind.tar valgrind/
	bzip2 -9 valgrind.tar
	rm -rf valgrind/
