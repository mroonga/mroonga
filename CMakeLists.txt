# Copyright(C) 2012 Kouhei Sutou <kou@clear-code.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(mroonga)

SET(MRN_VERSION_MAJOR 2)
SET(MRN_VERSION_MINOR 0)
SET(MRN_VERSION_MICRO 1)
SET(MRN_VERSION "${MRN_VERSION_MAJOR}.${MRN_VERSION_MINOR}${MRN_VERSION_MICRO}")
# TODO: FIXME
SET(MRN_VERSION_IN_HEX
  "0x0${MRN_VERSION_MAJOR}${MRN_VERSION_MINOR}${MRN_VERSION_MICRO}")
SET(MRN_PACKAGE_STRING "${PROJECT_NAME} ${MRN_VERSION}")

SET(MROONGA_C_SOURCES
  mrn_sys.c
  mrn_sys.h

  mrn_macro.h
  mrn_err.h
  mrn_mysql.h
  mrn_mysql_compat.h
  )
SET(MROONGA_CPP_SOURCES
  ha_mroonga.cc
  ha_mroonga.h
  mrn_table.cc
  mrn_table.h
  lib/mrn_path_mapper.cpp
  lib/mrn_path_mapper.hpp
  )
SET(MROONGA_SOURCES ${MROONGA_C_SOURCES} ${MROONGA_CPP_SOURCES})

# TODO: set VERSIONINFO for Microsoft Visual C++.
#       http://msdn.microsoft.com/en-us/library/aa381058%28VS.85%29.aspx
ADD_LIBRARY(ha_mroonga MODULE ${MROONGA_SOURCES})
SET_TARGET_PROPERTIES(ha_mroonga PROPERTIES
  COMPILE_DEFINITIONS "MYSQL_DYNAMIC_PLUGIN"
  PREFIX ""
  OUTPUT_NAME "ha_mroonga")
SET_SOURCE_FILES_PROPERTIES(${MROONGA_CPP_SOURCES} PROPERTIES
  COMPILE_FLAGS "-Werror -fno-implicit-templates -fno-exceptions -fno-rtti -felide-constructors")

INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(GROONGA REQUIRED "groonga >= 2.0.0")
TARGET_LINK_LIBRARIES(ha_mroonga ${GROONGA_LIBRARIES})

OPTION(WITH_DEBUG "Enable debug options" OFF)
IF(WITH_DEBUG)
  SET_PROPERTY(TARGET ha_mroonga APPEND PROPERTY
    COMPILE_DEFINITIONS "SAFE_MUTEX")
ENDIF()

OPTION(WITH_DEBUG_FULL "Enable full debug options" OFF)
IF(WITH_DEBUG_FULL)
  SET_PROPERTY(TARGET ha_mroonga APPEND PROPERTY
    COMPILE_DEFINITIONS "SAFE_MUTEX" "SAFEMALLOC")
ENDIF()

OPTION(DISABLE_FAST_MUTEXES "Force disabling fast mutex" OFF)
IF(DISABLE_FAST_MUTEXES)
  SET_PROPERTY(TARGET ha_mroonga APPEND PROPERTY
    COMPILE_DEFINITIONS "FORCE_FAST_MUTEX_DISABLED=1")
ENDIF()

SET(MYSQL_SOURCE_DIR "PATH" CACHE PATH "MySQL source directory")
SET(MYSQL_INCLUDE_DIRS
  "${MYSQL_SOURCE_DIR}/sql"
  "${MYSQL_SOURCE_DIR}/include"
  "${MYSQL_SOURCE_DIR}/regex"
  "${MYSQL_SOURCE_DIR}"
  CACHE INTERNAL "MySQL include directories")

SET(MYSQL_BUILD_DIR ${MYSQL_SOURCE_DIR} CACHE PATH "MySQL build directory")

SET(MYSQL_CONFIG "PATH" CACHE PATH "mysql-config command path")
FIND_PATH(MYSQL_CONFIG "${MYSQL_CONFIG}")

MACRO(SET_MYSQL_CONFIG_VALUE OPTION VARIABLE)
  EXECUTE_PROCESS(COMMAND "${MYSQL_CONFIG}" ${OPTION}
    OUTPUT_VARIABLE MYSQL_CONFIG_OUTPUT)
  STRING(STRIP ${MYSQL_CONFIG_OUTPUT} ${VARIABLE})
ENDMACRO()

SET_MYSQL_CONFIG_VALUE("--plugindir" MYSQL_PLUGIN_DIR)
SET_MYSQL_CONFIG_VALUE("--cflags" MYSQL_CFLAGS)
SET_MYSQL_CONFIG_VALUE("--version" MYSQL_VERSION)

SET(DEFAULT_PARSER "TokenBigram" CACHE STRING "The default fulltext parser")
SET_PROPERTY(TARGET ha_mroonga APPEND PROPERTY
  COMPILE_DEFINITIONS "MRN_PARSER_DEFAULT=\"${DEFAULT_PARSER}\"")

INCLUDE_DIRECTORIES(
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}"
  "${PROJECT_SOURCE_DIR}/lib"
  ${MYSQL_INCLUDE_DIRS}
  ${GROONGA_INCLUDE_DIRS}
  )

LINK_DIRECTORIES(
  ${GROONGA_LIBRARY_DIRS}
  )

INSTALL(TARGETS ha_mroonga DESTINATION "${MYSQL_PLUGIN_DIR}")

CONFIGURE_FILE (
  "${PROJECT_SOURCE_DIR}/mrn_version.h.in"
  "${PROJECT_BINARY_DIR}/mrn_version.h"
  )

CONFIGURE_FILE (
  "${PROJECT_SOURCE_DIR}/config.sh.in"
  "${PROJECT_BINARY_DIR}/config.sh"
  )
