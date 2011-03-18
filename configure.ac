
# According to http://www.gnu.org/software/libtool/manual/libtool.html#Updating-version-info :
#
# 1) when bugs are fixed or internal code is changed: increase MICRO
# 2) if API is added, increase MINOR and set MICRO to 0
# 3) if API or ABI is broken (this case should (must) never happen as it's very bad for a library), or a new designed library, increase MAJOR and set MINOR and MICRO to 0

m4_define([v_maj], [1])
m4_define([v_min], [5])
m4_define([v_mic], [1])

m4_define([lt_cur], m4_eval(v_maj + v_min))
m4_define([lt_rev], v_mic)
m4_define([lt_age], v_min)

AC_INIT([mpatrol], [v_maj.v_min.v_mic], [])
AC_CONFIG_HEADERS([configure.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([1.6 dist-bzip2])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_LIBTOOL_WIN32_DLL
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
AC_PROG_LIBTOOL

VMAJ=v_maj
VMIN=v_min
VMIC=v_mic
mp_version="${VMAJ}${VMIN}${VMIC}"
AC_SUBST(mp_version)

lt_version=lt_cur:lt_rev:lt_age
AC_SUBST(lt_version)


### Needed informations

have_windows="no"
case "$host_os" in
   mingw32ce* | cegcc*)
      AC_MSG_ERROR([Windows CE not supported])
      ;;
   mingw*)
      have_windows="yes"
      ;;
esac

AM_CONDITIONAL([HAVE_WINDOWS], [test "x${have_windows}" = "xyes"])

MPATROL_CPPFLAGS=""
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

# On Windows (MinGW) it seems that there is a conflict with
# mpatrol and msvcrt DLLs. So we statically link mpatrol to
# the program which uses it on that platform.
case "$host_os" in
   mingw*)
# Because of autoconf cache feature, I prefer using the
# linker directly.
      LIBS_save="$LIBS"
      LIBS="$LIBS -lbfd -limagehlp -liberty"
      AC_LINK_IFELSE(
         [AC_LANG_PROGRAM(
             [[
#include <bfd.h>
             ]],
             [[
bfd *b;
b = bfd_openr("", "");
             ]])],
         [
          have_bfd="yes"
          bfd_libs="-lbfd"
         ],
         [have_bfd="no"])
      LIBS="$LIBS_save"
      AC_MSG_CHECKING([whether bfd_openr is in -lbfd])
      AC_MSG_RESULT([${have_bfd}])

      if test "x$have_bfd" = "xno" ; then
         LIBS_save="$LIBS"
         LIBS="$LIBS -lbfd -lintl -limagehlp -liberty"
         AC_LINK_IFELSE(
            [AC_LANG_PROGRAM(
                [[
#include <bfd.h>
                ]],
                [[
bfd *b;
b = bfd_openr("", "");
                ]])],
            [
             have_bfd="yes"
             bfd_libs="-lbfd -lintl"
            ],
            [have_bfd="no"])
         LIBS="$LIBS_save"
         AC_MSG_CHECKING([whether bfd_openr is in -lbfd -lintl])
         AC_MSG_RESULT([${have_bfd}])
      fi
      if test "x$have_bfd" = "xno" ; then
         AC_MSG_ERROR([the symbol bfd_openr can not be found in libbfd.a, or libbfd.a or libintl are missing])
      fi
      MPATROL_LIBS="${bfd_libs} -limagehlp -liberty"
      ;;
   *)
      AC_CHECK_LIB([bfd], [bfd_openr],
         [dumy="yes"],
         [AC_MSG_ERROR([the symbol bfd_openr can not be found in libbfd.a or libbfd.a is missing])])
      MPATROL_LIBS="-lbfd"
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
   mingw*)
      ;;
   *)
      AC_CHECK_HEADERS([bfd.h],
         [dummy="yes"],
         [AC_MSG_ERROR([the header file bfd.h is missing])])
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
   mingw*)
      MPATROL_CFLAGS="${MPATROL_CFLAGS} -mno-cygwin"
      AC_DEFINE([X_DISPLAY_MISSING], [1], [Define to 1 if you want to build the GUI.])
      ;;
esac

# On x86_64, compilation prolem. Need to be fixed in mpatrol.
# Ugly hack: we avoid initialisation support on that platform
case "$host" in
   *x86_64*)
      MPATROL_CPPFLAGS="-DMP_INIT_SUPPORT=0"
      ;;
esac


### Checks for linker characteristics

case "$host_os" in
   mingw*)
      MPATROL_LDFLAGS="${MPATROL_LDFLAGS} -Wl,--enable-auto-import -Wl,--allow-multiple-definition"
      ;;
esac


### Checks for library functions

AC_FUNC_MMAP
AC_FUNC_SETVBUF_REVERSED
AC_CHECK_FUNCS(memalign mincore)


### Unit tests, coverage and benchmarking

MP_CHECK_TESTS([enable_tests="yes"], [enable_tests="no"])


AC_SUBST(MPATROL_CPPFLAGS)
AC_SUBST(MPATROL_CFLAGS)
AC_SUBST(MPATROL_LIBS)
AC_SUBST(MPATROL_LDFLAGS)

AC_CONFIG_FILES([
Makefile
mpatrol.pc
mpatrol-mt.pc
mpalloc.pc
src/Makefile
src/bin/Makefile
src/lib/Makefile
src/lib/mpatrol.h
src/lib/version.h
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
echo "Compilation:"
echo
echo "  CPPFLAGS.............: ${CPPFLAGS}"
echo "  CFLAGS...............: ${CFLAGS}"
echo "  LDFLAGS..............: ${LDFLAGS}"
echo
echo "Installation:"
echo
echo "  prefix...............: ${prefix}"
echo
