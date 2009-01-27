AC_INIT([groonga-mysql], [0.1], [te.ikeda@jpta.scs.co.jp])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_PROG_CXX
AC_PROG_CPP
AC_PROG_LIBTOOL

AC_MSG_CHECKING([mysql source])
AC_ARG_WITH([mysql],
  [AS_HELP_STRING([--with-mysql=PATH],[mysql source directory PATH])],
  [
    if test -f $withval/include/mysql/plugin.h ; then
      MYSQL_INC="-I$withval/sql -I$withval/include -I$withval/regex -I$withval"
      AC_SUBST(MYSQL_INC)
      AC_MSG_RESULT([yes])
    else
      AC_MSG_ERROR([invalid mysql source directory])
    fi
   ],
  [AC_MSG_ERROR([--with-mysql=PATH is required])])

CXXFLAGS="$CXXFLAGS -fno-implicit-templates -fno-exceptions -fno-rtti"

AC_CONFIG_FILES([
  Makefile
  src/Makefile
])
AC_OUTPUT
