dnl Copyright (C) 2008 Vincent Torri <vtorri at univ-evry dot fr>
dnl That code is public domain and can be freely used or copied.

dnl Macro that check if tests programs are wanted and if yes, if
dnl the Check library is available.

dnl Usage: MP_CHECK_TESTS([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Define the automake conditionnal MP_ENABLE_TESTS

AC_DEFUN([MP_CHECK_TESTS],
[

dnl configure option

AC_ARG_ENABLE([tests],
   [AC_HELP_STRING([--enable-tests], [enable tests @<:@default=no@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       _mp_enable_tests="yes"
    else
       _mp_enable_tests="no"
    fi
   ],
   [_mp_enable_tests="no"]
)
AC_MSG_CHECKING([whether tests are built])
AC_MSG_RESULT([${_mp_enable_tests}])

if test "x${_mp_enable_tests}" = "xyes" ; then
   PKG_CHECK_MODULES([CHECK], [check >= 0.9.5], [dummy="yes"], [_mp_enable_tests="no"])
fi

AM_CONDITIONAL(MP_ENABLE_TESTS, test "x${_mp_enable_tests}" = "xyes")

if test "x${_mp_enable_tests}" = "xyes" ; then
   ifelse([$1], , :, [$1])
else
   ifelse([$2], , :, [$2])
fi
])
