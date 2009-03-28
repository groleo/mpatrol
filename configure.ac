AC_INIT([mpatrol], [1.5.1], [])
AM_CONFIG_HEADER([configure.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE(1.6 dist-bzip2)

AC_LIBTOOL_WIN32_DLL
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
AC_PROG_LIBTOOL

### Libtool versioning
# 1. Start with version information of ‘0:0:0’ for each libtool library.
# 2. Update the version information only immediately before a public release
#    of your software. More frequent updates are unnecessary, and only
#    guarantee that the current interface number gets larger faster.
# 3. If the library source code has changed at all since the last update,
#    then increment revision (‘c:r:a’ becomes ‘c:r+1:a’).
# 4. If any interfaces have been added, removed, or changed since the last
#    update, increment current, and set revision to 0.
# 5. If any interfaces have been added since the last public release, then
#    increment age.
# 6. If any interfaces have been removed since the last public release, then
#    set age to 0.

version_info="6:1:5"
AC_SUBST(version_info)


### Needed informations

have_windows="no"
case "$host_os" in
   mingw* | cegcc*)
      have_windows="yes"
      ;;
esac

AM_CONDITIONAL([HAVE_WINDOWS], [test "x${have_windows}" = "xyes"])

MPATROL_CFLAGS=""
MPATROL_LIBS=""
MPATROL_LDFLAGS=""

### Additional options to configure


### Checks for programs
AC_PROG_CXX
AC_LANG(C)
AC_PROG_CPP
AC_PROG_CC


### Checks for libraries

case "$host_os" in
   mingw* | cegcc*)
      MPALLOC_PC_LIBS="\${libdir}/libmpalloc.a"
      MPATROL_PC_LIBS="\${libdir}/libmpatrol.a"
      MPATROL_MT_PC_LIBS="\${libdir}/libmpatrolmt.a"
      MPATROL_LIBS="${MPATROL_LIBS} -lbfd -limagehlp -liberty"
      ;;
   *)
      MPALLOC_PC_LIBS="-L\${libdir} -lmpalloc"
      MPATROL_PC_LIBS="-L\${libdir} -lmpatrol"
      MPATROL_MT_PC_LIBS="-L\${libdir} -lmpatrolmt"
      MPATROL_LIBS="${MPATROL_LIBS} -lbfd"
      ;;
esac

AC_PATH_X
AC_PATH_XTRA

have_x="yes"
if test "x${no_x}" = "xyes" ; then
   have_x="no"
fi

if test "x${have_x}" = "xyes" ; then
   AC_CHECK_LIB([Xt], [XtVaAppInitialize], [have_x="yes"], [have_x="no"])
fi


### Checks for header files

case "$host_os" in
   mingw* | cegcc*)
      ;;
   *)
      AC_CHECK_HEADERS([bfd.h])
      ;;
esac

save_CPPFLAGS=${CPPFLAGS}
CPPFLAGS="${CPPFLAGS} ${X_CFLAGS}"
if test "x${have_x}" = "xyes" ; then
   AC_CHECK_HEADERS([Xm/DrawingA.h Xm/ScrolledW.h], [have_x="yes"], [have_x="no"])
fi
CPPFLAGS="${save_CPPFLAGS}"

if test "x${have_x}" = "xyes" ; then
   AC_DEFINE([MP_GUI_SUPPORT], [1], [Define to 1 if you built mptrace with X toolkit])
fi


### Checks for types

AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_CHECK_TYPE([ptrdiff_t], [long])
AC_CHECK_SIZEOF([void *])


### Checks for structures


### Checks for compiler characteristics

MPATROL_CFLAGS="-O3 -fno-inline-functions -Wmissing-prototypes -Wall -Wshadow -Wpointer-arith -Wstrict-prototypes -Wmissing-declarations -Wdeclaration-after-statement"
case "$host_os" in
   mingw* | cegcc*)
      MPATROL_CFLAGS="${MPATROL_CFLAGS} -mno-cygwin"
      AC_DEFINE([X_DISPLAY_MISSING], [1], [Define to 1 if you want to build the GUI.])
      ;;
esac


### Checks for linker characteristics

case "$host_os" in
   mingw* | cegcc*)
      MPATROL_LDFLAGS="${MPATROL_LDFLAGS} -Wl,--enable-auto-import -Wl,--allow-multiple-definition"
      ;;
esac


### Checks for library functions

AC_FUNC_MMAP
AC_FUNC_SETVBUF_REVERSED
AC_CHECK_FUNCS(memalign mincore)


### Unit tests, coverage and benchmarking

MP_CHECK_TESTS([enable_tests="yes"], [enable_tests="no"])


AC_SUBST(MPATROL_CFLAGS)
AC_SUBST(MPATROL_LIBS)
AC_SUBST(MPALLOC_PC_LIBS)
AC_SUBST(MPATROL_PC_LIBS)
AC_SUBST(MPATROL_MT_PC_LIBS)
AC_SUBST(MPATROL_LDFLAGS)

AC_CONFIG_FILES([
Makefile
mpatrol.pc
mpatrol-mt.pc
mpalloc.pc
src/Makefile
src/bin/Makefile
src/lib/Makefile
src/tests/Makefile
])

AC_OUTPUT

echo
echo
echo "------------------------------------------------------------------------"
echo "$PACKAGE_NAME $PACKAGE_VERSION"
echo "------------------------------------------------------------------------"
echo
echo "Configuration Options Summary:"
echo
echo "  OS...................: ${host_os}"
echo "  GUI..................: ${have_x}"
echo "  Tests................: ${enable_tests}"
echo
echo "Installation:"
echo
echo "  prefix...............: ${prefix}"
echo
