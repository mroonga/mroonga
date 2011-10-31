AC_INIT([groonga-storage-engine], [1.0.2], [ikdttr@gmail.com])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror foreign tar-pax])

AC_C_BIGENDIAN
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

  ac_mysql_source_dir=
  AC_ARG_WITH([mysql-source],
    [AS_HELP_STRING([--with-mysql-source=PATH], [MySQL source directory PATH])],
    [
      ac_mysql_source_dir="$withval"
      if test -f "$ac_mysql_source_dir/sql/handler.h" ; then
        MYSQL_INC="-I$ac_mysql_source_dir/sql"
        MYSQL_INC="$MYSQL_INC -I$ac_mysql_source_dir/include"
        MYSQL_INC="$MYSQL_INC -I$ac_mysql_source_dir/regex"
        MYSQL_INC="$MYSQL_INC -I$ac_mysql_source_dir"
        AC_SUBST(MYSQL_INC)
        AC_MSG_RESULT([yes])
      else
        AC_MSG_ERROR([invalid MySQL source directory])
      fi
    ],
    [AC_MSG_ERROR([--with-mysql-source=PATH is required for standalone build])]
  )
  MYSQL_SOURCE="$ac_mysql_source_dir"
  AC_SUBST(MYSQL_SOURCE)

  ac_mysql_build_dir=
  AC_ARG_WITH([mysql-build],
    [AS_HELP_STRING([--with-mysql-build=PATH], [MySQL build directory PATH])],
    [ac_mysql_build_dir="$withval"],
    [ac_mysql_build_dir="$ac_mysql_source_dir"]
  )
  MYSQL_BUILD="$ac_mysql_build_dir"
  AC_SUBST(MYSQL_BUILD)

  AC_MSG_CHECKING([mysql_config])
  AC_ARG_WITH([mysql-config],
    [AS_HELP_STRING([--with-mysql-config=PATH],
                    [mysql_config PATH])],
    [ac_mysql_config="$withval"],
    [ac_mysql_config=])
  if test -z "$ac_mysql_config"; then
    AC_PATH_PROG(ac_mysql_config, mysql_config, mysql-config-not-found)
  fi
  if test "$ac_mysql_config" = "mysql-config-not-found"; then
    AC_MSG_ERROR([can't detect mysql_config. Please specify mysql_config path by --with-mysql-config=PATH.])
  fi
  AC_MSG_RESULT([$ac_mysql_config])
  plugindir="$($ac_mysql_config --plugindir)"
  if test $? -ne 0; then
    AC_MSG_ERROR([failed to run "$ac_mysql_config": $plugindir])
  fi
  MYSQL_CFLAGS="$MYSQL_CFLAGS $($ac_mysql_config --cflags)"
  MYSQL_INC="$MYSQL_INC $($ac_mysql_config --include)"
  ac_mysql_major_version="`$ac_mysql_config --version | cut -b 1,3`"
  if test $ac_mysql_major_version -lt 55; then
    MYSQL_VERSION="5.1"
  else
    MYSQL_VERSION="5.5"
  fi
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
  PKG_CHECK_MODULES(GROONGA, groonga >= 1.2.7)
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
if test "$standalone_build" = "yes"; then
  CONFIG_OPTION_MYSQL
else
  MYSQL_INC="-I../../sql -I../../include -I../../regex -I../.."
  plugindir="\$(pkglibdir)/plugin"
fi
AC_SUBST(MYSQL_INC)
AC_SUBST(MYSQL_CFLAGS)
AC_SUBST(MYSQL_VERSION)
AC_SUBST(plugindir)
CONFIG_OPTION_MECAB
CONFIG_OPTION_GROONGA

AC_ARG_WITH(default_parser,
    [AC_HELP_STRING([--with-default-parser=PARSER],
                    [specify the default fulltext parser like
                     --with-default-parser=TokenMecab.
                     (default: TokenBigram)])],
    [default_parser=$withval],
    [default_parser=no])
if test x"$default_parser" != x"no"; then
  AC_DEFINE_UNQUOTED(MRN_PARSER_DEFAULT,
		     "$default_parser",
		     "specified default fulltext parser")
fi

AC_ARG_ENABLE(fast_mutexes,
    [AC_HELP_STRING([--disable-fast-mutexes],
                    [Force disable fast mutex.
                     [default: use mysql_config output]])],
    [enable_fast_mutexes=$enableval],
    [enable_fast_mutexes=auto])
if test "$enable_fast_mutexes" = "no"; then
   AC_DEFINE(FORCE_FAST_MUTEX_DISABLED, [1],
             [Define to 1 if force fast mutext disabled])
fi

# check Cutter with GLib support if available
REQUIRED_MINIMUM_CUTTER_VERSION=1.1.3
m4_ifdef([AC_CHECK_GCUTTER], [
AC_CHECK_GCUTTER(>= $REQUIRED_MINIMUM_CUTTER_VERSION)
],
[ac_cv_use_cutter="no"])
AM_CONDITIONAL([WITH_CUTTER], [test "$ac_cv_use_cutter" != "no"])

# For mroonga.github.com
AC_ARG_WITH(mroonga-github-com-path,
            [AS_HELP_STRING([--with-mroonga-github-com-path=PATH],
            [specify mroonga.github.com path to update mroonga.github.com.])],
            [MROONGA_GITHUB_COM_PATH="$withval"],
            [MROONGA_GITHUB_COM_PATH=""])
AC_SUBST(MROONGA_GITHUB_COM_PATH)

# For Debian package release
AC_ARG_WITH(rsync-path,
            [AS_HELP_STRING([--with-rsync-path=PATH],
            [specify rsync path to upload groonga storage engine Debian packages.])],
            [RSYNC_PATH="$withval"],
            [RSYNC_PATH=""])
AC_SUBST(RSYNC_PATH)

# Document
AC_MSG_CHECKING([whether enable document])
AC_ARG_ENABLE(document,
  [AS_HELP_STRING([--enable-document],
    [enable document generation by Sphinx. [default=auto]])],
  [enable_document="$enableval"],
  [enable_document="auto"])
AC_MSG_RESULT($enable_document)

# check sphinx-build for documentation
document_available=no
if test x"$enable_document" != x"no"; then
  AC_PATH_PROG(HG, hg, hg-not-found)
  if test -f "$srcdir/doc/build-stamp"; then
    document_available=yes
  else
    if test x"$HG" = x"hg-not-found"; then
      if test x"$enable_document" = x"yes"; then
        AC_MSG_ERROR("No hg found")
      fi
    else
      document_available=yes
    fi
  fi
fi
AC_SUBST(HG)

AM_CONDITIONAL([ENABLE_DOCUMENT],
               [test "${document_available}" = "yes"])
AC_MSG_CHECKING([whether document available])
AC_MSG_RESULT($document_available)

AM_CONDITIONAL([ENABLE_DOCUMENT],
               [test "${document_available}" = "yes"])
AC_MSG_CHECKING([whether document available])
AC_MSG_RESULT($document_available)

CFLAGS="$CFLAGS -Werror"
CXXFLAGS="$CXXFLAGS -Werror -fno-implicit-templates -fno-exceptions -fno-rtti -felide-constructors"

AC_CONFIG_FILES([
  Makefile
  test/Makefile
  test/unit/Makefile
  test/sql/Makefile
  test/sql/groonga_include/Makefile
  test/sql/groonga_storage/Makefile
  test/sql/groonga_storage/r/Makefile
  test/sql/groonga_storage/t/Makefile
  test/sql/groonga_wrapper/Makefile
  test/sql/groonga_wrapper/r/Makefile
  test/sql/groonga_wrapper/t/Makefile
  packages/Makefile
  packages/rpm/Makefile
  packages/rpm/centos/Makefile
  packages/rpm/fedora/Makefile
  packages/yum/Makefile
  packages/apt/Makefile
  tools/Makefile
  doc/Makefile
  doc/locale/Makefile
  doc/locale/en/Makefile
  doc/locale/en/LC_MESSAGES/Makefile
  doc/locale/ja/Makefile
  doc/locale/ja/LC_MESSAGES/Makefile
])
AC_OUTPUT([
  packages/rpm/centos/mysql-groonga.spec
  packages/rpm/fedora/mysql-groonga.spec
])
