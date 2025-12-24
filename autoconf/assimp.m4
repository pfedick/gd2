dnl Configure path for Assimp
dnl PKG_CHECK_MODULES will be used if available

AC_DEFUN([AM_PATH_ASSIMP],
[dnl
dnl Get the cflags and libraries
dnl
AC_ARG_WITH(assimp,[  --with-assimp=PFX   Prefix where Assimp is installed (optional)], assimp_prefix="$withval", assimp_prefix="")
AC_ARG_ENABLE(assimptest, [  --disable-assimptest       Do not try to compile and run a test Assimp program],, enable_assimptest=yes)

  if test x$assimp_prefix != x ; then
    assimp_args="$assimp_args --prefix=$assimp_prefix"
    ASSIMP_CFLAGS="-I$assimp_prefix/include"
    ASSIMP_LIBS="-L$assimp_prefix/lib"
  fi

  PKG_CHECK_MODULES([ASSIMP], [assimp >= 5.0.0], [
    AC_DEFINE(HAVE_ASSIMP, 1, [Define if you have Assimp library])
    $1
  ], [
    dnl Fallback to manual checking
    AC_MSG_CHECKING(for Assimp)
    
    save_CPPFLAGS="$CPPFLAGS"
    save_LIBS="$LIBS"
    CPPFLAGS="$CPPFLAGS $ASSIMP_CFLAGS"
    LIBS="$LIBS $ASSIMP_LIBS -lassimp"
    
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[
      #include <assimp/cimport.h>
      #include <assimp/scene.h>
      #include <assimp/postprocess.h>
    ]], [[
      const struct aiScene* scene = aiImportFile("test.lwo", 0);
      aiReleaseImport(scene);
    ]])],[
      AC_MSG_RESULT(yes)
      ASSIMP_LIBS="$ASSIMP_LIBS -lassimp"
      AC_DEFINE(HAVE_ASSIMP, 1, [Define if you have Assimp library])
      $1
    ],[
      AC_MSG_RESULT(no)
      CPPFLAGS="$save_CPPFLAGS"
      LIBS="$save_LIBS"
      $2
    ])
  ])

  AC_SUBST(ASSIMP_CFLAGS)
  AC_SUBST(ASSIMP_LIBS)
])
