# ===========================================================================
# AX_CHECK_GTEST([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
#
# Checks for Google Test and defines
#   GTEST_CFLAGS
#   GTEST_LIBS
#
# It also defines HAVE_GTEST if it's found and sets the conditional HAVE_GTEST
# ===========================================================================
AC_DEFUN([AX_CHECK_GTEST],[
  AC_MSG_CHECKING([for Google Test])
  AC_ARG_WITH([gtest],
    AS_HELP_STRING([--without-gtest], [disable Google Test checks]),
    [],
    [with_gtest=yes])

  HAVE_GTEST="no"
  AS_IF([test "x$with_gtest" != "xno"], [
    # Use pkg-config to find gtest and gtest_main
    # gtest_main provides a main() function, which is convenient
    PKG_CHECK_MODULES([GTEST], [gtest gtest_main], [
      AC_DEFINE([HAVE_GTEST], [1], [Define if you have Google Test])
      HAVE_GTEST="yes"
    ], [
      # Fallback if gtest_main is not found (older versions)
      PKG_CHECK_MODULES([GTEST], [gtest], [
        AC_DEFINE([HAVE_GTEST], [1], [Define if you have Google Test])
        HAVE_GTEST="yes"
      ])
    ])
  ])

  AS_IF([test "x$HAVE_GTEST" = "xyes"], [
    AC_MSG_RESULT([yes])
    m4_default([$1], [:])
  ], [
    AC_MSG_RESULT([no])
    m4_default([$2], [:])
  ])

  AM_CONDITIONAL([HAVE_GTEST], [test "x$HAVE_GTEST" = "xyes"])
])
