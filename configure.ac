m4_define([mrn_version_major],  m4_include(version_major))
m4_define([mrn_version_minor],  m4_include(version_minor))
m4_define([mrn_version_micro],  m4_include(version_micro))
m4_define([mrn_version],        m4_include(version))
m4_define([mrn_version_in_hex], m4_include(version_in_hex))
m4_define([mrn_plugin_version], m4_include(plugin_version))

AC_INIT([mroonga], [mrn_version], [ikdttr@gmail.com])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror foreign tar-pax])

MRN_VERSION=mrn_version
MRN_VERSION_MAJOR=mrn_version_major
MRN_VERSION_MINOR=mrn_version_minor
MRN_VERSION_MICRO=mrn_version_micro
MRN_VERSION_IN_HEX=mrn_version_in_hex
MRN_PLUGIN_VERSION=mrn_plugin_version
AC_SUBST([MRN_VERSION])
AC_SUBST([MRN_VERSION_MAJOR])
AC_SUBST([MRN_VERSION_MINOR])
AC_SUBST([MRN_VERSION_MICRO])
AC_SUBST([MRN_VERSION_IN_HEX])
AC_SUBST([MRN_PLUGIN_VERSION])

MRN_PACKAGE_STRING="$PACKAGE_STRING"
AC_SUBST([MRN_PACKAGE_STRING])

AC_C_BIGENDIAN
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_LIBTOOL

AC_DEFUN([CHECK_CFLAG], [
  AC_MSG_CHECKING([if gcc supports $1])
  old_CFLAGS=$CFLAGS
  flag=`echo '$1' | sed -e 's,^-Wno-,-W,'`
  CFLAGS="$CFLAGS $flag -Werror"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
    [check_cflag=yes],
    [check_cflag=no])
  CFLAGS="$old_CFLAGS"
  if test "x$check_cflag" = "xyes"; then
    CFLAGS="$CFLAGS $1"
  fi
  AC_MSG_RESULT([$check_cflag])
])

AC_DEFUN([CHECK_CXXFLAG], [
  AC_MSG_CHECKING([if g++ supports $1])
  old_CXXFLAGS=$CXXFLAGS
  flag=`echo '$1' | sed -e 's,^-Wno-,-W,'`
  CXXFLAGS="$CXXFLAGS $flag -Werror"
  AC_LANG_PUSH([C++])
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
    [check_cxxflag=yes],
    [check_cxxflag=no])
  AC_LANG_POP([C++])
  CXXFLAGS="$old_CXXFLAGS"
  if test "x$check_cxxflag" = "xyes"; then
    CXXFLAGS="$CXXFLAGS $1"
  fi
  AC_MSG_RESULT([$check_cxxflag])
])

AC_DEFUN([CHECK_BUILD_FLAG], [
  CHECK_CFLAG([$1])
  CHECK_CXXFLAG([$1])
])

if test "$GCC" = "yes"; then
  CHECK_BUILD_FLAG([-Wall])
  CHECK_BUILD_FLAG([-Wextra])
  CHECK_BUILD_FLAG([-Wno-unused-parameter])
  CHECK_BUILD_FLAG([-Wno-strict-aliasing])
  # REMOVEME: workaround for MySQL/MariaDB 5.5.22 :<
  # They use deprecated MYSQL::generate_name style in class definition.
  CHECK_BUILD_FLAG([-Wno-deprecated])
fi

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
      if test -f "$ac_mysql_source_dir/sql/handler.h"; then
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
  MYSQL_SOURCE_DIR="$ac_mysql_source_dir"
  AC_SUBST(MYSQL_SOURCE_DIR)

  ac_mysql_build_dir=
  AC_ARG_WITH([mysql-build],
    [AS_HELP_STRING([--with-mysql-build=PATH], [MySQL build directory PATH])],
    [ac_mysql_build_dir="$withval"],
    [ac_mysql_build_dir="$ac_mysql_source_dir"]
  )
  MYSQL_BUILD_DIR="$ac_mysql_build_dir"
  AC_SUBST(MYSQL_BUILD_DIR)

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
  MYSQL_VERSION="$($ac_mysql_config --version)"

  AC_MSG_CHECKING([for InnoDB FTS])
  mrn_mysql_innodb_fts_p=no
  if test x"$MYSQL_VERSION" == x"5.6.3-labs-innodb-fts"; then
    mrn_mysql_innodb_fts_p=yes
    AC_DEFINE([MRN_MYSQL_INNODB_FTS_P], [1],
              [Define to 1 if the target MySQL has InnoDB FTS])
  fi
  AC_MSG_RESULT($mrn_mysql_innodb_fts_p)
])

AC_DEFUN([CONFIG_OPTION_GROONGA],[
  PKG_CHECK_MODULES(GROONGA, groonga >= 2.0.2)
  _PKG_CONFIG(GROONGA_VERSION, variable=groonga_version, groonga)
  GROONGA_VERSION=$pkg_cv_GROONGA_VERSION
  AC_SUBST(GROONGA_VERSION)
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

AC_ARG_WITH(valgrind,
  [AS_HELP_STRING([--with-valgrind], [Use valgrind. [default=no]])],
  [with_valgrind="$withval"],
  [with_valgrind="no"])
if test "$with_valgrind" != "no"; then
  CFLAGS="-DHAVE_valgrind $CFLAGS"
  CXXFLAGS="-DHAVE_valgrind $CXXFLAGS"
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
  MRN_DEFAULT_PARSER=$default_parser
else
  MRN_DEFAULT_PARSER=TokenBigram
fi
AC_SUBST(MRN_DEFAULT_PARSER)

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

# check Cutter with C++ support if available
REQUIRED_MINIMUM_CUTTER_VERSION=1.1.3
m4_ifdef([AC_CHECK_CPPCUTTER], [
AC_CHECK_CPPCUTTER(>= $REQUIRED_MINIMUM_CUTTER_VERSION)
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
            [specify rsync path to upload mroonga Debian packages.])],
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

CFLAGS="$CFLAGS -Werror"
CXXFLAGS="$CXXFLAGS -Werror -fno-implicit-templates -fno-exceptions -fno-rtti -felide-constructors"

AC_CONFIG_FILES([
  Makefile
  build/Makefile
  build/cmake_modules/Makefile
  lib/Makefile
  test/Makefile
  test/unit/Makefile
  test/sql/Makefile
  test/sql/include/Makefile
  test/sql/suite/mroonga_storage/Makefile
  test/sql/suite/mroonga_storage/r/Makefile
  test/sql/suite/mroonga_storage/t/Makefile
  test/sql/suite/mroonga_wrapper/Makefile
  test/sql/suite/mroonga_wrapper/r/Makefile
  test/sql/suite/mroonga_wrapper/t/Makefile
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
  config.sh
  mrn_version.h
  test/sql/suite/mroonga_storage/r/information_schema.result
  test/sql/suite/mroonga_storage/r/variables.result
  packages/rpm/centos/mysql-mroonga.spec
  packages/rpm/fedora/mysql-mroonga.spec
])
