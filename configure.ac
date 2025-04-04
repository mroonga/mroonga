AC_PREREQ(2.59)

m4_define([mrn_version_major],  m4_include(version_major))
m4_define([mrn_version_minor],  m4_include(version_minor))
m4_define([mrn_version_micro],  m4_include(version_micro))
m4_define([mrn_version_full],   m4_include(version_full))
m4_define([mrn_version_in_hex], m4_include(version_in_hex))
m4_define([mrn_plugin_version], m4_include(plugin_version))

AC_INIT([mroonga], [mrn_version_full], [groonga-talk@lists.sourceforge.net])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([tar-pax foreign subdir-objects])

MRN_VERSION_FULL=mrn_version_full
MRN_VERSION_MAJOR=mrn_version_major
MRN_VERSION_MINOR=mrn_version_minor
MRN_VERSION_MICRO=mrn_version_micro
MRN_VERSION_IN_HEX=mrn_version_in_hex
MRN_PLUGIN_VERSION=mrn_plugin_version
AC_SUBST([MRN_VERSION_FULL])
AC_SUBST([MRN_VERSION_MAJOR])
AC_SUBST([MRN_VERSION_MINOR])
AC_SUBST([MRN_VERSION_MICRO])
AC_SUBST([MRN_VERSION_IN_HEX])
AC_SUBST([MRN_PLUGIN_VERSION])

MRN_PACKAGE_STRING="$PACKAGE_STRING"
AC_SUBST([MRN_PACKAGE_STRING])

MRN_BUNDLED=FALSE
AC_SUBST([MRN_BUNDLED])

AC_C_BIGENDIAN
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_LIBTOOL
m4_ifdef([LT_OUTPUT], [LT_OUTPUT])

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
  CHECK_CXXFLAG([-Werror=overloaded-virtual])
fi

AC_MSG_CHECKING(for the suffix of plugin shared libraries)
shrext_cmds=$(./libtool --config | grep '^shrext_cmds=')
eval $shrext_cmds
module=yes eval MRN_PLUGIN_SUFFIX="$shrext_cmds"
AC_MSG_RESULT($MRN_PLUGIN_SUFFIX)
if test -z "$MRN_PLUGIN_SUFFIX"; then
  AC_MSG_ERROR([can't detect plugin suffix])
fi
AC_SUBST(MRN_PLUGIN_SUFFIX)

AC_DEFUN([CONFIG_OPTION_MYSQL],[
  AC_MSG_CHECKING([mysql source])

  ac_mysql_source_dir=
  AC_ARG_WITH([mysql-source],
    [AS_HELP_STRING([--with-mysql-source=PATH], [MySQL source directory PATH])],
    [
      ac_mysql_source_dir="$withval"
      if test -f "$ac_mysql_source_dir/sql/handler.h"; then
        case "$ac_mysql_source_dir" in
        /*)
            :
            ;;
        *)
            ac_mysql_source_dir="$ac_pwd/$ac_mysql_source_dir"
            ;;
        esac
        AC_MSG_RESULT([yes])
      else
        AC_MSG_ERROR([invalid MySQL source directory])
      fi
    ],
    [AC_MSG_ERROR([--with-mysql-source=PATH is required])]
  )
  MYSQL_SOURCE_DIR="$ac_mysql_source_dir"
  AC_SUBST(MYSQL_SOURCE_DIR)

  ac_mysql_build_dir=
  AC_ARG_WITH([mysql-build],
    [AS_HELP_STRING([--with-mysql-build=PATH], [MySQL build directory PATH])],
    [ac_mysql_build_dir="$withval"],
    [ac_mysql_build_dir="$ac_mysql_source_dir"]
  )
  case "$ac_mysql_build_dir" in
  /*)
      :
      ;;
  *)
      ac_mysql_build_dir="$ac_pwd/$ac_mysql_build_dir"
      ;;
  esac
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
  AC_SUBST(plugindir)

  MYSQL_VERSION="$($ac_mysql_config --version)"
  AC_SUBST(MYSQL_VERSION)

  MYSQL_MAJOR_MINOR_VERSION=["$(echo $MYSQL_VERSION | sed -e 's/\([0-9]*\)\.\([0-9]*\)\.\([0-9]*[a-z]*\)[.0-9a-z-]*$/\1.\2/')"]

  case "$MYSQL_MAJOR_MINOR_VERSION" in
  10.*)
    MYSQL_VARIANT=mariadb
    ;;
  *)
    MYSQL_VARIANT=mysql
    ;;
  esac

  MYSQL_CFLAGS="$MYSQL_CFLAGS $($ac_mysql_config --cflags)"
  AC_SUBST(MYSQL_CFLAGS)

  MYSQL_INCLUDES=""
  MYSQL_INCLUDES="$MYSQL_INCLUDES -I$ac_mysql_build_dir/include"
  MYSQL_INCLUDES="$MYSQL_INCLUDES -I$ac_mysql_source_dir/sql"
  if test -d "$ac_mysql_source_dir/sql/auth"; then
    MYSQL_INCLUDES="$MYSQL_INCLUDES -I$ac_mysql_source_dir/sql/auth"
  fi
  MYSQL_INCLUDES="$MYSQL_INCLUDES -I$ac_mysql_source_dir/include"
  if test -d "$ac_mysql_source_dir/wsrep-lib"; then
    mysql_wsrep_include_dir="$ac_mysql_source_dir/wsrep-lib/include"
    MYSQL_INCLUDES="$MYSQL_INCLUDES -I$mysql_wsrep_include_dir"
    mysql_wsrep_api_include_dir=$(dirname ${ac_mysql_source_dir}/wsrep-lib/wsrep-API/*/wsrep_api.h)
    MYSQL_INCLUDES="$MYSQL_INCLUDES -I$mysql_wsrep_api_include_dir"
  fi
  if test -d "$ac_mysql_source_dir/extra/regex"; then
    mysql_regex_include_dir="$ac_mysql_source_dir/extra/regex"
    MYSQL_INCLUDES="$MYSQL_INCLUDES -I$mysql_regex_include_dir"
  else
    if test -d "$ac_mysql_source_dir/pcre"; then
      mysql_regex_include_dir="$ac_mysql_source_dir/pcre"
    else
      mysql_regex_include_dir="$ac_mysql_source_dir/regex"
    fi
    MYSQL_INCLUDES="$MYSQL_INCLUDES -I$mysql_regex_include_dir"
  fi
  if test -d "$ac_mysql_source_dir/extra/rapidjson"; then
    mysql_rapidjson_include_dir="$ac_mysql_source_dir/extra/rapidjson/include"
    MYSQL_INCLUDES="$MYSQL_INCLUDES -I$mysql_rapidjson_include_dir"
  fi
  if test -d "$ac_mysql_source_dir/libbinlogevents"; then
    MYSQL_INCLUDES="$MYSQL_INCLUDES -I$ac_mysql_build_dir/libbinlogevents/include"
    MYSQL_INCLUDES="$MYSQL_INCLUDES -I$ac_mysql_source_dir/libbinlogevents/export"
    MYSQL_INCLUDES="$MYSQL_INCLUDES -I$ac_mysql_source_dir/libbinlogevents/include"
  fi
  if test -d "$ac_mysql_build_dir/extra/protobuf/protobuf-3.11.4"; then
    mysql_protobuf_include_dir="$ac_mysql_source_dir/extra/protobuf/protobuf-3.11.4/src"
    MYSQL_INCLUDES="$MYSQL_INCLUDES -I$mysql_protobuf_include_dir"
  fi
  if test -d "$ac_mysql_build_dir/extra/zlib/zlib-1.2.13"; then
    mysql_zlib_include_dir="$ac_mysql_source_dir/extra/zlib/zlib-1.2.13"
    MYSQL_INCLUDES="$MYSQL_INCLUDES -I$mysql_zlib_include_dir"
    mysql_zlib_config_include_dir="$ac_mysql_build_dir/extra/zlib/zlib-1.2.13"
    MYSQL_INCLUDES="$MYSQL_INCLUDES -I$mysql_zlib_config_include_dir"
  elif test -d "$ac_mysql_build_dir/extra/zlib/zlib-1.2.12"; then
    mysql_zlib_include_dir="$ac_mysql_source_dir/extra/zlib/zlib-1.2.12"
    MYSQL_INCLUDES="$MYSQL_INCLUDES -I$mysql_zlib_include_dir"
    mysql_zlib_config_include_dir="$ac_mysql_build_dir/extra/zlib/zlib-1.2.12"
    MYSQL_INCLUDES="$MYSQL_INCLUDES -I$mysql_zlib_config_include_dir"
  elif test -d "$ac_mysql_build_dir/extra/zlib"; then
    mysql_zlib_include_dir="$ac_mysql_source_dir/extra/zlib"
    MYSQL_INCLUDES="$MYSQL_INCLUDES -I$mysql_zlib_include_dir"
    mysql_zlib_config_include_dir="$ac_mysql_build_dir/extra/zlib"
    MYSQL_INCLUDES="$MYSQL_INCLUDES -I$mysql_zlib_config_include_dir"
  fi
  if test -d "$ac_mysql_source_dir/extra/unordered_dense/unordered_dense-4.4.0"; then
    mysql_unordered_dense_include_dir="$ac_mysql_source_dir/extra/unordered_dense/unordered_dense-4.4.0/include"
    MYSQL_INCLUDES="$MYSQL_INCLUDES -I$mysql_unordered_dense_include_dir"
  fi
  if test "${MYSQL_VARIANT}" = "mysql"; then
    MYSQL_INCLUDES="$MYSQL_INCLUDES -I$ac_mysql_source_dir"
  fi
  MYSQL_INCLUDES="$MYSQL_INCLUDES $($ac_mysql_config --include)"
  AC_SUBST(MYSQL_INCLUDES)

  MYSQL_CXXFLAGS="-felide-constructors"
  case "$MYSQL_MAJOR_MINOR_VERSION" in
  5.7)
    m4_ifdef([AX_CXX_COMPILE_STDCXX_11],
             [AX_CXX_COMPILE_STDCXX_11([ext], [mandatory])],
             [CHECK_CXXFLAG([-std=gnu++11])])
    ;;
  8.*)
    m4_ifdef([AX_CXX_COMPILE_STDCXX_17],
             [AX_CXX_COMPILE_STDCXX_17([ext], [mandatory])],
             [CHECK_CXXFLAG([-std=gnu++17])])
    ;;
  10.*)
    m4_ifdef([AX_CXX_COMPILE_STDCXX_11],
             [AX_CXX_COMPILE_STDCXX_11([ext], [mandatory])],
             [CHECK_CXXFLAG([-std=gnu++11])])
    ;;
  *)
    :
    ;;
  esac

  AC_MSG_CHECKING([for libmysqlservices.a directory])
  pkglibdir="$($ac_mysql_config --variable=pkglibdir)"
  mysql_build_archive_output_dir="${MYSQL_BUILD_DIR}/archive_output_directory"
  mysql_build_libservices_dir="${MYSQL_BUILD_DIR}/libservices"
  if test -f "${mysql_build_archive_output_dir}/libmysqlservices.a"; then
    mysql_services_lib_dir="${mysql_build_archive_output_dir}"
  elif test -f "${mysql_build_libservices_dir}/libmysqlservices.a"; then
    mysql_services_lib_dir="${mysql_build_libservices_dir}"
  else
    if test -f "${pkglibdir}/libmysqlservices.a"; then
      mysql_services_lib_dir="${pkglibdir}"
    elif test -f "${pkglibdir}/mysql/libmysqlservices.a"; then
      mysql_services_lib_dir="${pkglibdir}/mysql"
    else
      AC_MSG_ERROR([libmysqlservices.a is not found in <${pkglibdir}/> and <${pkglibdir}/mysql/>])
    fi
  fi
  AC_MSG_RESULT([$mysql_services_lib_dir])
  MYSQL_LIBS="$MYSQL_LIBS \"$mysql_services_lib_dir/libmysqlservices.a\""

  case "$MYSQL_MAJOR_MINOR_VERSION" in
  10.4)
    :
    ;;
  10.*)
    AC_MSG_CHECKING([for libperfschema.a directory])
    pkglibdir="$($ac_mysql_config --variable=pkglibdir)"
    mysql_build_perfschema_dir="${MYSQL_BUILD_DIR}/storage/perfschema"
    if test -f "${mysql_build_perfschema_dir}/libperfschema.a"; then
      perfschema_lib_dir="${mysql_build_perfschema_dir}"
    else
      if test -f "${pkglibdir}/libperfschema.a"; then
	perfschema_lib_dir="${pkglibdir}"
      elif test -f "${pkglibdir}/mysql/libperfschema.a"; then
	mysql_services_lib_dir="${pkglibdir}/mysql"
      else
	AC_MSG_ERROR([libperfschema.a is not found in <${pkglibdir}/> and <${pkglibdir}/mysql/>])
      fi
    fi
    AC_MSG_RESULT([$perfschema_lib_dir])
    MYSQL_LIBS="$MYSQL_LIBS \"$perfschema_lib_dir/libperfschema.a\""
    ;;
  *)
    :
    ;;
  esac

  AC_SUBST(MYSQL_LIBS)
])

m4_define([mrn_required_groonga_version], m4_include(required_groonga_version))
REQUIRED_GROONGA_VERSION=mrn_required_groonga_version
AC_SUBST(REQUIRED_GROONGA_VERSION)
AC_DEFUN([CONFIG_OPTION_GROONGA],[
  PKG_CHECK_MODULES(GROONGA, groonga >= ${REQUIRED_GROONGA_VERSION})
  _PKG_CONFIG(GROONGA_VERSION, variable=groonga_version, groonga)
  GROONGA_VERSION=$pkg_cv_GROONGA_VERSION
  AC_SUBST(GROONGA_VERSION)
])

m4_define([mrn_required_groonga_normalizer_mysql_version],
          m4_include(required_groonga_normalizer_mysql_version))
REQUIRED_GROONGA_NORMALIZER_MYSQL_VERSION=mrn_required_groonga_normalizer_mysql_version
AC_DEFUN([CONFIG_OPTION_GROONGA_NORMALIZER_MYSQL], [
  AC_MSG_CHECKING([for groonga-normalizer-mysql])
  PKG_CHECK_EXISTS([groonga-normalizer-mysql >= ${REQUIRED_GROONGA_NORMALIZER_MYSQL_VERSION}],
                   [WITH_GROONGA_NORMALIZER_MYSQL=yes],
                   [WITH_GROONGA_NORMALIZER_MYSQL=no])
  AC_MSG_RESULT($WITH_GROONGA_NORMALIZER_MYSQL)
  if test "$WITH_GROONGA_NORMALIZER_MYSQL" = "yes"; then
    AC_DEFINE([WITH_GROONGA_NORMALIZER_MYSQL],
              [1],
              [Use MySQL normalizer plugin for groonga])
    _PKG_CONFIG(plugin_name, variable=plugin_name, groonga-normalizer-mysql)
    GROONGA_NORMALIZER_MYSQL_PLUGIN_NAME=$pkg_cv_plugin_name
    AC_DEFINE_UNQUOTED([GROONGA_NORMALIZER_MYSQL_PLUGIN_NAME],
                       "${GROONGA_NORMALIZER_MYSQL_PLUGIN_NAME}",
                       [Name of MySQL normalizer plugin for groonga])
  fi
])

m4_define([mrn_package_groonga_version], m4_include(package_groonga_version))
PACKAGE_GROONGA_VERSION=mrn_package_groonga_version
AC_SUBST(PACKAGE_GROONGA_VERSION)
m4_define([mrn_package_mariadb_version], m4_include(package_mariadb_version))
PACKAGE_MARIADB_VERSION=mrn_package_mariadb_version
AC_SUBST(PACKAGE_MARIADB_VERSION)
m4_define([mrn_package_groonga_normalizer_mysql_version], m4_include(package_groonga_normalizer_mysql_version))
PACKAGE_GROONGA_NORMALIZER_MYSQL_VERSION=mrn_package_groonga_normalizer_mysql_version
AC_SUBST(PACKAGE_GROONGA_NORMALIZER_MYSQL_VERSION)

CONFIG_OPTION_MYSQL
CONFIG_OPTION_GROONGA
CONFIG_OPTION_GROONGA_NORMALIZER_MYSQL

AC_ARG_WITH(debug,
    [dnl
AS_HELP_STRING([--with-debug],
               [Add debug code])
AS_HELP_STRING([--with-debug=full],
               [Add debug code (adds memory checker, very slow)])dnl
    ],
    [with_debug=$withval],
    [with_debug=no])
if test "$with_debug" = "yes"
then
  # Medium debug.
  AC_DEFINE([DBUG_ON], [1], [Enable debug])
  if test "${MYSQL_VARIANT}" = "mariadb"; then
    AC_DEFINE([DBUG_TRACE], [1], [Enable debug trace])
  fi
  CFLAGS="$DEBUG_CFLAGS $DEBUG_OPTIMIZE_CC -DSAFE_MUTEX $CFLAGS -O0 -g3"
  CXXFLAGS="$DEBUG_CXXFLAGS $DEBUG_OPTIMIZE_CXX -DSAFE_MUTEX $CXXFLAGS -O0 -g3"
elif test "$with_debug" = "full"
then
  # Full debug. Very slow in some cases
  AC_DEFINE([DBUG_ON], [1], [Enable debug])
  if test "${MYSQL_VARIANT}" = "mariadb"; then
    AC_DEFINE([DBUG_TRACE], [1], [Enable debug trace])
  fi
  CFLAGS="$DEBUG_CFLAGS -DSAFE_MUTEX -DSAFEMALLOC $CFLAGS -O0 -g3"
  CXXFLAGS="$DEBUG_CXXFLAGS -DSAFE_MUTEX -DSAFEMALLOC $CXXFLAGS -O0 -g3"
else
  # Optimized version. No debug
  AC_DEFINE([NDEBUG], [1], [Disable debug])
  AC_DEFINE([DBUG_OFF], [1], [Disable debug])
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

AC_ARG_WITH(default_parser,
    [AS_HELP_STRING([--with-default-parser=PARSER],
                    [Deprecated. Use --with-default-tokenizer=TOKENIZER instead.
		     specify the default fulltext parser like
                     --with-default-parser=TokenMecab.
                     (default: TokenBigram)])],
    [default_parser=$withval],
    [default_parser=no])
if test x"$default_parser" != x"no"; then
  AC_DEFINE_UNQUOTED(MRN_TOKENIZER_DEFAULT,
		     "$default_parser",
		     "specified the default tokenizer for fulltext index")
fi

AC_ARG_WITH(default_tokenizer,
    [AS_HELP_STRING([--with-default-tokenizer=TOKENIZER],
                    [specify the default tokenizer for fulltext index like
                     --with-default-tokenizer=TokenMecab.
                     (default: TokenBigram)])],
    [default_tokenizer=$withval],
    [default_tokenizer=no])
if test x"$default_tokenizer" != x"no"; then
  AC_DEFINE_UNQUOTED(MRN_DEFAULT_TOKENIZER,
		     "$default_tokenizer",
		     "specified the default tokenizer for fulltext index")
fi

AC_ARG_ENABLE(fast_mutexes,
    [AS_HELP_STRING([--disable-fast-mutexes],
                    [Force disable fast mutex.
                     [default: use mysql_config output]])],
    [enable_fast_mutexes=$enableval],
    [enable_fast_mutexes=auto])
if test "$enable_fast_mutexes" = "no"; then
   AC_DEFINE(FORCE_FAST_MUTEX_DISABLED, [1],
             [Define to 1 if force fast mutexes disabled])
elif test "$enable_fast_mutexes" = "yes"; then
   AC_DEFINE(MY_PTHREAD_FASTMUTEX, [1],
             [Define to 1 if fast mutexes enabled])
fi

AC_ARG_ENABLE(dtrace,
    [AS_HELP_STRING([--enable-dtrace],
                    [Enable DTrace. [default: no]])],
    [enable_dtrace=$enableval],
    [enable_dtrace=no])
if test "$enable_dtrace" = "no"; then
   AC_DEFINE(DISABLE_DTRACE, [1], [Define to 1 if DTrace is disabled])
fi

# For mroonga.github.com
AC_ARG_WITH(mroonga-github-com-path,
            [AS_HELP_STRING([--with-mroonga-github-com-path=PATH],
            [specify mroonga.github.com path to update mroonga.github.com.])],
            [MROONGA_GITHUB_COM_PATH="$withval"],
            [MROONGA_GITHUB_COM_PATH=""])
AC_SUBST(MROONGA_GITHUB_COM_PATH)

# For package
AC_ARG_WITH(rsync-path,
            [AS_HELP_STRING([--with-rsync-path=PATH],
            [specify rsync path to upload mroonga packages.])],
            [RSYNC_PATH="$withval"],
            [RSYNC_PATH="packages@packages.groonga.org:public"])
AC_SUBST(RSYNC_PATH)

AC_ARG_WITH(launchpad-ppa,
            [AS_HELP_STRING([--with-launchpad-ppa=PPA],
            [specify Launchpad Personal Package Archive. [default=groonga-ppa]])],
            [LAUNCHPAD_PPA="$withval"],
            [LAUNCHPAD_PPA="groonga-ppa"])
AC_SUBST(LAUNCHPAD_PPA)

AC_ARG_WITH(launchpad-uploader-pgp-key,
            [AS_HELP_STRING([--with-launchpad-uploader-pgp-key=KEY],
            [specify PGP key UID to upload Groonga packages to Launchpad.])],
            [LAUNCHPAD_UPLOADER_PGP_KEY="$withval"],
            [LAUNCHPAD_UPLOADER_PGP_KEY=""])
AC_SUBST(LAUNCHPAD_UPLOADER_PGP_KEY)

GPG_UID=m4_include(gpg_uid)
AC_SUBST(GPG_UID)

# For updating apt/yum repositories
AC_ARG_WITH(groonga-source-path,
            [AS_HELP_STRING([--with-groonga-source-path=PATH],
            [specify Groonga source path for Mroonga's release manager.])],
            [GROONGA_SOURCE_PATH="$withval"])
case "$GROONGA_SOURCE_PATH" in
  ""|/*)
    : # do nothing
    ;;
  *)
    GROONGA_SOURCE_PATH="\$(top_builddir)/${GROONGA_SOURCE_PATH}"
    ;;
esac
AC_SUBST(GROONGA_SOURCE_PATH)

# Document
AC_MSG_CHECKING([whether enable document])
AC_ARG_ENABLE(document,
  [AS_HELP_STRING([--enable-document],
    [enable document generation by Sphinx. [default=auto]])],
  [enable_document="$enableval"],
  [enable_document="auto"])
AC_MSG_RESULT($enable_document)

document_available=no
document_buildable=no
have_built_document=no
if test x"$enable_document" != x"no"; then
  if test -f "$srcdir/doc/build-stamp"; then
    document_available=yes
    have_built_document=yes
  fi

  if test x"$enable_document" = x"yes"; then
    AC_PATH_PROG(SPHINX_BUILD, sphinx-build, [])
    if test -n "$SPHINX_BUILD"; then
      document_available=yes
      document_buildable=yes
    else
      AC_MSG_ERROR([
No sphinx-build found.
Install it and try again.

How to install sphinx-build:

For Debian GNU/Linux based system like Ubuntu:
  % sudo apt-get install -y python-pip
  % sudo pip install sphinx

For Red Hat based system like CentOS:
  % sudo yum install -y python-pip
  % sudo pip install sphinx])
    fi
    AC_SUBST(SPHINX_BUILD)
  fi
fi

AM_CONDITIONAL([DOCUMENT_AVAILABLE],
               [test "${document_available}" = "yes"])
AC_MSG_CHECKING([whether document available])
AC_MSG_RESULT($document_available)

AM_CONDITIONAL([DOCUMENT_BUILDABLE],
               [test "${document_buildable}" = "yes"])
AC_MSG_CHECKING([whether document buildable])
AC_MSG_RESULT($document_buildable)

AM_CONDITIONAL([HAVE_BUILT_DOCUMENT],
               [test "${have_built_document}" = "yes"])
AC_MSG_CHECKING([whether having built document])
AC_MSG_RESULT($have_built_document)

DOCUMENT_VERSION=mrn_version
DOCUMENT_VERSION_FULL="$DOCUMENT_VERSION"
AC_SUBST(DOCUMENT_VERSION)
AC_SUBST(DOCUMENT_VERSION_FULL)

CFLAGS="$CFLAGS"
CXXFLAGS="$CXXFLAGS $MYSQL_CXXFLAGS"

AC_CONFIG_FILES([
  Makefile
  build/Makefile
  build/cmake_modules/Makefile
  lib/Makefile
  udf/Makefile
  test/Makefile
  mysql-test/Makefile
  packages/Makefile
  packages/source/Makefile
  tools/Makefile
  doc/Makefile
  doc/locale/Makefile
  doc/locale/en/Makefile
  doc/locale/en/LC_MESSAGES/Makefile
  doc/locale/ja/Makefile
  doc/locale/ja/LC_MESSAGES/Makefile
  data/Makefile
  data/deb/Makefile
  data/rpm/Makefile
])
AC_OUTPUT([
  config.sh
  mrn_version.h
  mysql-test/mroonga/storage/information_schema/r/plugins.result
  mysql-test/mroonga/storage/variable/r/version.result
  data/update.sql
])
