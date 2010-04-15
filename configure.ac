AC_INIT([mroonga], [0.1], [ikdttr@gmail.com])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_LIBTOOL

standalone_build=

AC_DEFUN([CONFIG_OPTION_PLUGINS],
  AC_MSG_CHECKING([standalone build])
  AC_ARG_WITH([plugins],,
    [
      standalone_build="no"
      AC_MSG_RESULT([no])
    ],
    [
      standalone_build="yes"
      AC_MSG_RESULT([yes])
    ])
)

AC_DEFUN([CONFIG_OPTION_MYSQL],[
  AC_MSG_CHECKING([mysql source])
  AC_ARG_WITH([mysql],
    [AS_HELP_STRING([--with-mysql=PATH],[mysql source directory PATH])],
    [
      if test -f $withval/sql/handler.h ; then
        MYSQL_INC="-I$withval/sql -I$withval/include -I$withval/regex -I$withval"
        AC_SUBST(MYSQL_INC)
        AC_MSG_RESULT([yes])
      else
        AC_MSG_ERROR([invalid mysql source directory])
      fi
     ],
    [AC_MSG_ERROR([--with-mysql=PATH is required for standalone build])]
  )
])

AC_DEFUN([CONFIG_OPTION_MECAB],[
  MECAB_INCLUDES=
  MECAB_LIBS=

  AC_MSG_CHECKING([mecab])
  AC_ARG_WITH([mecab],[AS_HELP_STRING([--with-mecab=PATH],[mecab prefix PATH])],
    [with_mecab=$withval],[with_mecab=no])

  if test "$with_mecab" = "no"; then
    AC_MSG_RESULT([no])
  elif test "$with_mecab" = "yes"; then
    if test -x /usr/bin/mecab-config; then
      MECAB_CONFIG='/usr/bin/mecab-config'
    elif test -x /usr/local/bin/mecab-config; then
      MECAB_CONFIG='/usr/local/bin/mecab-config'
    else
      AC_MSG_ERROR([mecab not found!!])
    fi
    MECAB_INCLUDES="`\"$MECAB_CONFIG\" --cflags`"
    MECAB_LIBS="`\"$MECAB_CONFIG\" --libs`"
  elif test -d $with_mecab; then
    MECAB_CONFIG="$with_mecab/bin/mecab-config"
    MECAB_INCLUDES="`\"$MECAB_CONFIG\" --cflags`"
    MECAB_LIBS="`\"$MECAB_CONFIG\" --libs`"
  else
    AC_MSG_ERROR([argument doesn't exists: --with-mecab=$with_mecab])
  fi

  _CPPFLAGS="${CPPFLAGS}"
  _LIBS="${LIBS}"
  CPPFLAGS="${CPPFLAGS} ${MECAB_INCLUDES}"
  LIBS="${LIBS} ${MECAB_LIBS}"

  AC_CHECK_FUNC(mecab_new, [
    AC_DEFINE([HAVE_MECAB], [1], [Define to 1 if libmecab is available])
    HAVE_MECAB=1
  ], [
    HAVE_MECAB=
    MECAB_INCLUDES=
    MECAB_LIBS=
  ])

  CPPFLAGS="${_CPPFLAGS}"
  LIBS="${_LIBS}"

  AC_SUBST(MECAB_INCLUDES)
  AC_SUBST(MECAB_LIBS)
])

AC_DEFUN([CONFIG_OPTION_GROONGA],[
  GROONGA_INCLUDES=
  GROONGA_LIBS=
  AC_MSG_CHECKING([groonga])
  AC_ARG_WITH([groonga],
    [AS_HELP_STRING([--with-groonga=PATH],[groonga prefix PATH])],
    [with_groonga=$withval],[with_groonga=yes])

  if test "$with_groonga" = "no";then
    AC_MSG_ERROR([groonga must not be disabled]);
  elif test "$with_groonga" = "yes"; then
    GROONGA_INCLUDES="`pkg-config --cflags groonga`"
    GROONGA_LIBS="`pkg-config --libs groonga`"
  elif test -d $with_groonga; then
    PKG_CONFIG_PATH="$with_groonga/lib/pkgconfig"
    GROONGA_INCLUDES="`PKG_CONFIG_PATH=$PKG_CONFIG_PATH pkg-config --cflags groonga`"
    GROONGA_LIBS="`PKG_CONFIG_PATH=$PKG_CONFIG_PATH pkg-config --libs groonga`"
  else
   AC_MSG_ERROR([argument doesn't exists: --with-groonga=$with_groonga])
  fi
  AC_MSG_RESULT([yes])

  _CPPFLAGS="${CPPFLAGS}"
  _LIBS="${LIBS}"
  CPPFLAGS="${CPPFLAGS} ${GROONGA_INCLUDES}"
  LIBS="${LIBS} ${GROONGA_LIBS}"

  AC_CHECK_FUNC(grn_init, [AC_DEFINE([ENABLE_GROONGA], [1],
    [Define to 1 if Groonga is enabled])],
    [AC_MSG_ERROR([Could not find libgroonga. Check your Groonga installation.])
    ])

  CPPFLAGS="${_CPPFLAGS}"
  LIBS="${_LIBS}"

  AC_SUBST([GROONGA_INCLUDES])
  AC_SUBST([GROONGA_LIBS])
])

AC_ARG_WITH(debug,
    [  --with-debug            Add debug code
  --with-debug=full       Add debug code (adds memory checker, very slow)],
    [with_debug=$withval],
    [with_debug=no])
if test "$with_debug" = "yes"
then
  # Medium debug.
  AC_DEFINE([DBUG_ON], [1], [Use libdbug])
  CFLAGS="$DEBUG_CFLAGS $DEBUG_OPTIMIZE_CC -DSAFE_MUTEX $CFLAGS"
  CXXFLAGS="$DEBUG_CXXFLAGS $DEBUG_OPTIMIZE_CXX -DSAFE_MUTEX $CXXFLAGS"
elif test "$with_debug" = "full"
then
  # Full debug. Very slow in some cases
  AC_DEFINE([DBUG_ON], [1], [Use libdbug])
  CFLAGS="$DEBUG_CFLAGS -DSAFE_MUTEX -DSAFEMALLOC $CFLAGS"
  CXXFLAGS="$DEBUG_CXXFLAGS -DSAFE_MUTEX -DSAFEMALLOC $CXXFLAGS"
else
  # Optimized version. No debug
  AC_DEFINE([DBUG_OFF], [1], [Don't use libdbug])
  CFLAGS="$OPTIMIZE_CFLAGS $CFLAGS"
  CXXFLAGS="$OPTIMIZE_CXXFLAGS $CXXFLAGS"
fi

CONFIG_OPTION_PLUGINS
if test "$standalone_build" = "yes";then
  CONFIG_OPTION_MYSQL
else
  MYSQL_INC="-I../../sql -I../../include -I../../regex -I../.."
  AC_SUBST(MYSQL_INC)
fi
CONFIG_OPTION_MECAB
CONFIG_OPTION_GROONGA

AC_CHECK_CUTTER
AC_CHECK_GCUTTER

CFLAGS="$CFLAGS -Werror"
CXXFLAGS="$CXXFLAGS -Werror -fno-implicit-templates -fno-exceptions -fno-rtti -felide-constructors"

AC_CONFIG_FILES([
  Makefile
  test/Makefile
  test/unit/Makefile
])
AC_OUTPUT
