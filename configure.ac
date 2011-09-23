AC_INIT([groonga-storage-engine], [1.0], [ikdttr@gmail.com])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

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
  MYSQL_INC="$MYSQL_INC $($ac_mysql_config --include)"
  AC_SUBST(plugindir)
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
  PKG_CHECK_MODULES(GROONGA, groonga >= 1.0.4)
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
AC_SUBST(plugindir)
CONFIG_OPTION_MECAB
CONFIG_OPTION_GROONGA

# check Cutter with GLib support if available
REQUIRED_MINIMUM_CUTTER_VERSION=1.1.3
m4_ifdef([AC_CHECK_GCUTTER], [
AC_CHECK_GCUTTER(>= $REQUIRED_MINIMUM_CUTTER_VERSION)
],
[ac_cv_use_cutter="no"])
AM_CONDITIONAL([WITH_CUTTER], [test "$ac_cv_use_cutter" != "no"])

# For Debian package release
AC_ARG_WITH(rsync-path,
            [AS_HELP_STRING([--with-rsync-path=PATH],
            [specify rsync path to upload groonga storage engine Debian packages.])],
            [RSYNC_PATH="$withval"],
            [RSYNC_PATH=""])
AC_SUBST(RSYNC_PATH)

# check sphinx-build for documentation
ac_sphinx_available="no"
sphinx_required_version="1.0.1"
sphinx_required_version_major=$(echo $sphinx_required_version | cut -d. -f1)
sphinx_required_version_minor=$(echo $sphinx_required_version | cut -d. -f2)
sphinx_required_version_micro=$(echo $sphinx_required_version | cut -d. -f3)
AC_ARG_WITH([sphinx_build],
            AS_HELP_STRING([--with-sphinx-build=PATH],
                           [sphinx-build path (default: auto-detect)]),
            [SPHINX_BUILD="$withval"])

if test "$SPHINX_BUILD" = "no"; then
  : # ignore
elif test "$SPHINX_BUILD" = ""; then
  AC_PATH_PROG(SPHINX_BUILD, sphinx-build, none)
  if test "$SPHINX_BUILD" != "none"; then
    ac_sphinx_available="yes"
  fi
else
  AC_CHECK_FILE([$SPHINX_BUILD],
                [ac_sphinx_available="yes"],
                [AC_MSG_WARN([$SPHINX_BUILD is not found. Disable document gerataion.])])
fi
if test "$ac_sphinx_available" = "yes"; then
  sphinx_build_version=$($SPHINX_BUILD 2>&1 | head -1 | $SED -e 's/^Sphinx v//')
  sphinx_build_version_major=$(echo $sphinx_build_version | cut -d. -f1)
  sphinx_build_version_minor=$(echo $sphinx_build_version | cut -d. -f2)
  sphinx_build_version_micro=$(echo $sphinx_build_version | cut -d. -f3)
  if test \
       \( "$sphinx_build_version_major" -gt \
             "$sphinx_required_version_major" \) -o \
       \( "$sphinx_build_version_major" -eq \
             "$sphinx_required_version_major" -a \
          "$sphinx_build_version_minor" -gt \
             "$sphinx_required_version_minor" \) -o \
       \( "$sphinx_build_version_major" -eq \
             "$sphinx_required_version_major" -a \
          "$sphinx_build_version_minor" -eq \
             "$sphinx_required_version_minor" -a \
          "$sphinx_build_version_micro" -ge \
             "$sphinx_required_version_micro" \); then
    :
  else
    ac_sphinx_available="no"
  fi
fi
AC_SUBST(SPHINX_BUILD)
AM_CONDITIONAL([ENABLE_DOCUMENT],
               [test "$ac_sphinx_available" = "yes" -o \
                     -f "$srcdir/doc/ja/html-build-stamp"])
AC_MSG_CHECKING([for sphinx availablity])
AC_MSG_RESULT($ac_sphinx_available (sphinx-build=$SPHINX_BUILD version=$sphinx_build_version required=$sphinx_required_version))

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
  apt/Makefile
  rpm/Makefile
  rpm/centos/Makefile
  rpm/fedora/Makefile
  yum/Makefile
  doc/Makefile
  doc/ja/Makefile
])
AC_OUTPUT([
  rpm/centos/mysql-groonga.spec
  rpm/fedora/mysql-groonga.spec
])
