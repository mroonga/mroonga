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

cmake_minimum_required(VERSION 2.6)
project(mroonga)

if(WITH_MROONGA_STORAGE_ENGINE)
  set(MRN_SOURCE_DIR ${CMAKE_SOURCE_DIR}/storage/mroonga)
else()
  set(MRN_SOURCE_DIR ${CMAKE_SOURCE_DIR})
endif()

file(READ ${MRN_SOURCE_DIR}/version        MRN_VERSION)
file(READ ${MRN_SOURCE_DIR}/version_major  MRN_VERSION_MAJOR)
file(READ ${MRN_SOURCE_DIR}/version_minor  MRN_VERSION_MINOR)
file(READ ${MRN_SOURCE_DIR}/version_micro  MRN_VERSION_MICRO)
file(READ ${MRN_SOURCE_DIR}/version_in_hex MRN_VERSION_IN_HEX)
file(READ ${MRN_SOURCE_DIR}/plugin_version MRN_PLUGIN_VERSION)

file(READ ${MRN_SOURCE_DIR}/required_groonga_version REQUIRED_GROONGA_VERSION)
string(STRIP "${REQUIRED_GROONGA_VERSION}" REQUIRED_GROONGA_VERSION)

set(MRN_PACKAGE_STRING "${PROJECT_NAME} ${MRN_VERSION}")

include(${MRN_SOURCE_DIR}/build/cmake_modules/ReadFileList.cmake)

if(WITH_MROONGA_STORAGE_ENGINE)
  set(LIBMRN_DIR_PREFIX "${MRN_SOURCE_DIR}/")
else()
  set(LIBMRN_DIR_PREFIX "")
endif()

read_file_list(${CMAKE_CURRENT_SOURCE_DIR}/sources.am MROONGA_SOURCES)
read_file_list(${CMAKE_CURRENT_SOURCE_DIR}/lib/libmrn_no_mysql_sources.am
  LIBMRN_NO_MYSQL_SOURCES)
string(REGEX REPLACE "([^;]+)" "${LIBMRN_DIR_PREFIX}lib/\\1"
  LIBMRN_NO_MYSQL_SOURCES "${LIBMRN_NO_MYSQL_SOURCES}")
read_file_list(${CMAKE_CURRENT_SOURCE_DIR}/lib/libmrn_need_mysql_sources.am
  LIBMRN_NEED_MYSQL_SOURCES)
string(REGEX REPLACE "([^;]+)" "${LIBMRN_DIR_PREFIX}lib/\\1"
  LIBMRN_NEED_MYSQL_SOURCES "${LIBMRN_NEED_MYSQL_SOURCES}")

if(WITH_MROONGA_STORAGE_ENGINE)
  set(MYSQL_SOURCE_DIR ${CMAKE_SOURCE_DIR} CACHE PATH "MySQL source directory")
else()
  set(MYSQL_SOURCE_DIR "PATH" CACHE PATH "MySQL source directory")
endif()

set(MYSQL_INCLUDE_DIRS
  "${MYSQL_BUILD_DIR}/include"
  "${MYSQL_SOURCE_DIR}/sql"
  "${MYSQL_SOURCE_DIR}/include"
  "${MYSQL_SOURCE_DIR}/regex"
  "${MYSQL_SOURCE_DIR}"
  CACHE INTERNAL "MySQL include directories")

set(MYSQL_BUILD_DIR ${MYSQL_SOURCE_DIR} CACHE PATH "MySQL build directory")

if(WITH_MROONGA_STORAGE_ENGINE)
  set(MYSQL_CONFIG ${CMAKE_SOURCE_DIR}/scripts/mysql_config
    CACHE PATH "mysql-config command path")
else()
  set(MYSQL_CONFIG "PATH" CACHE PATH "mysql-config command path")
endif()
find_path(MYSQL_CONFIG "${MYSQL_CONFIG}")

macro(SET_MYSQL_CONFIG_VALUE OPTION VARIABLE)
  if(NOT ${VARIABLE})
    execute_process(COMMAND "${MYSQL_CONFIG}" ${OPTION}
      OUTPUT_VARIABLE MYSQL_CONFIG_OUTPUT)
    string(STRIP ${MYSQL_CONFIG_OUTPUT} ${VARIABLE})
  endif()
endmacro()

set_mysql_config_value("--plugindir" MYSQL_PLUGIN_DIR)
set_mysql_config_value("--cflags" MYSQL_CFLAGS)
set_mysql_config_value("--version" MYSQL_VERSION)

include(FindPkgConfig)
pkg_check_modules(GROONGA REQUIRED "groonga >= ${REQUIRED_GROONGA_VERSION}")

include_directories(
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}"
  "${PROJECT_SOURCE_DIR}/lib"
  ${MYSQL_INCLUDE_DIRS}
  ${GROONGA_INCLUDE_DIRS}
  )

if(WIN32)
  set(MYSQL_LIBRARY_DIRS
    "${MYSQL_BUILD_DIR}/lib"
    "${MYSQL_BUILD_DIR}/libmysqld"
    )
endif()
link_directories(
  ${GROONGA_LIBRARY_DIRS}
  ${MYSQL_LIBRARY_DIRS}
  )

# TODO: set VERSIONINFO for Microsoft Visual C++.
#       http://msdn.microsoft.com/en-us/library/aa381058%28VS.85%29.aspx
add_library(ha_mroonga MODULE
  ${MROONGA_SOURCES}
  ${LIBMRN_NO_MYSQL_SOURCES}
  ${LIBMRN_NEED_MYSQL_SOURCES})
if(NOT WIN32)
  set(MYSQL_CFLAGS "${MYSQL_CFLAGS} -Werror -fno-implicit-templates -fno-exceptions -fno-rtti -felide-constructors")
endif()
set_source_files_properties(${MROONGA_SOURCES} PROPERTIES
  COMPILE_FLAGS "${MYSQL_CFLAGS}")
set_target_properties(ha_mroonga PROPERTIES
  COMPILE_DEFINITIONS "MYSQL_DYNAMIC_PLUGIN"
  PREFIX ""
  OUTPUT_NAME "ha_mroonga")

set(DEFAULT_PARSER "TokenBigram" CACHE STRING "The default fulltext parser")
set_property(TARGET ha_mroonga APPEND PROPERTY
  COMPILE_DEFINITIONS "MRN_PARSER_DEFAULT=\"${DEFAULT_PARSER}\"")
if(WIN32)
# TODO: set DBUG_OFF to link with MySQL release/debug import library.
#       it is better that you can customize this definition.
# TODO: support security enhancements introduced in the CRT.
#       http://msdn.microsoft.com/en-us/library/8ef0s5kh%28v=vs.90%29.aspx
  set_property(TARGET ha_mroonga APPEND PROPERTY COMPILE_DEFINITIONS
    "__WIN__"
    "DBUG_OFF"
    "_CRT_SECURE_NO_WARNINGS"
    "_WIN32_WINNT=0x0501"
    "WIN32_LEAN_AND_MEAN"
  )
  
endif()

if(WITH_MROONGA_STORAGE_ENGINE)
  MYSQL_ADD_PLUGIN(mroonga
    "${MROONGA_SOURCES};${LIBMRN_NEED_MYSQL_SOURCES};${LIBMRN_NO_MYSQL_SOURCES}"
    STORAGE_ENGINE MANDATORY
    LINK_LIBRARIES ${GROONGA_LIBRARIES}
    RECOMPILE_FOR_EMBEDDED)
endif()

if(WIN32)
# NOTE: ignore incompatible runtime libraries explicitly.
#       http://msdn.microsoft.com/en-us/library/6wtdswk0%28v=vs.80%29.aspx 
  set_target_properties(ha_mroonga PROPERTIES
    LINK_FLAGS "/NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:msvcrtd.lib")
  set_target_properties(ha_mroonga PROPERTIES
    LINK_FLAGS_DEBUG "/NODEFAULTLIB:libcmt.lib")
  set_target_properties(ha_mroonga PROPERTIES
    LINK_FLAGS_RELEASE "/NODEFAULTLIB:libcmtd.lib")
  set(MYSQL_IMPORT_LIBRARIES
    "libmysqld${IMPORT_SUFFIX};mysqlserver${IMPORT_SUFFIX}")
  target_link_libraries(ha_mroonga ${GROONGA_LIBRARIES} ${MYSQL_IMPORT_LIBRARIES})
else()
  target_link_libraries(ha_mroonga ${GROONGA_LIBRARIES})
endif()

option(WITH_DEBUG "Enable debug options" OFF)
if(WITH_DEBUG)
  set_property(TARGET ha_mroonga APPEND PROPERTY
    COMPILE_DEFINITIONS "SAFE_MUTEX")
endif()

option(WITH_DEBUG_FULL "Enable full debug options" OFF)
if(WITH_DEBUG_FULL)
  set_property(TARGET ha_mroonga APPEND PROPERTY
    COMPILE_DEFINITIONS "SAFE_MUTEX" "SAFEMALLOC")
endif()

option(DISABLE_FAST_MUTEXES "Force disabling fast mutex" OFF)
if(DISABLE_FAST_MUTEXES)
  set_property(TARGET ha_mroonga APPEND PROPERTY
    COMPILE_DEFINITIONS "FORCE_FAST_MUTEX_DISABLED=1")
endif()

install(TARGETS ha_mroonga DESTINATION "${MYSQL_PLUGIN_DIR}")

configure_file(
  "${PROJECT_SOURCE_DIR}/mrn_version.h.in"
  "${PROJECT_BINARY_DIR}/mrn_version.h"
  )

configure_file(
  "${PROJECT_SOURCE_DIR}/config.sh.in"
  "${PROJECT_BINARY_DIR}/config.sh"
  )

configure_file(
  "${PROJECT_SOURCE_DIR}/test/sql/suite/mroonga/storage/information_schema/r/plugins.result.in"
  "${PROJECT_SOURCE_DIR}/test/sql/suite/mroonga/storage/information_schema/r/plugins.result"
  )
