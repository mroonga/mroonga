# -*- indent-tabs-mode: nil -*-
#
# Copyright(C) 2012-2024  Sutou Kouhei <kou@clear-code.com>
# Copyright(C) 2013  Kentoku SHIBA
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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  set(MRN_BUNDLED FALSE)
else()
  set(MRN_BUNDLED TRUE)
endif()

if(MRN_BUNDLED)
  if(WITHOUT_MROONGA OR
      WITHOUT_MROONGA_STORAGE_ENGINE OR
      "${PLUGIN_MROONGA}" STREQUAL "NO")
    return()
  endif()
endif()

# Ubuntu 22.04: 3.22
cmake_minimum_required(VERSION 3.22)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/version_full   MRN_VERSION_FULL)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/version_major  MRN_VERSION_MAJOR)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/version_minor  MRN_VERSION_MINOR)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/version_micro  MRN_VERSION_MICRO)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/version_in_hex MRN_VERSION_IN_HEX)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/plugin_version MRN_PLUGIN_VERSION)

project(mroonga
  VERSION "${VERSION}"
  DESCRIPTION "CJK-ready fulltext search, column store"
  LANGUAGES C CXX)

if(NOT DEFINED CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 17)
  set(CMAKE_CXX_STANDARD 17)
endif()

include(TestBigEndian)
test_big_endian(BIG_ENDIAN)
if(BIG_ENDIAN)
  set(MRN_BIG_ENDIAN_NOT_SUPPORTED_MESSAGE
    "Mroonga doesn't support on big-endian")
  if(MRN_BUNDLED)
    message(STATUS ${MRN_BIG_ENDIAN_NOT_SUPPORTED_MESSAGE})
    return()
  else()
    message(FATAL_ERROR ${MRN_BIG_ENDIAN_NOT_SUPPORTED_MESSAGE})
  endif()
endif()

if(MSVC)
  if(MSVC_VERSION LESS 1800)
    set(MRN_OLD_MSVC_MESSAGE "Mroonga supports only MSVC 2013 or later")
    if(MRN_BUNDLED)
      message(STATUS ${MRN_OLD_MSVC_MESSAGE})
      return()
    else()
      message(FATAL_ERROR ${MRN_OLD_MSVC_MESSAGE})
    endif()
  endif()
endif()

set(MRN_BUNDLED_GROONGA_RELATIVE_DIR "../../extra/groonga")
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${MRN_BUNDLED_GROONGA_RELATIVE_DIR}")
  set(MRN_BUNDLED_GROONGA_RELATIVE_DIR "vendor/groonga")
endif()
set(MRN_BUNDLED_GROONGA_DIR
  "${CMAKE_CURRENT_SOURCE_DIR}/${MRN_BUNDLED_GROONGA_RELATIVE_DIR}")
if(EXISTS "${MRN_BUNDLED_GROONGA_DIR}")
  set(MRN_GROONGA_BUNDLED TRUE)
else()
  set(MRN_GROONGA_BUNDLED FALSE)
endif()

set(MRN_PLUGIN_SUFFIX ${CMAKE_SHARED_MODULE_SUFFIX})

# Reset include directories defined by MariaDB.
get_directory_property(DEFAULT_INCLUDE_DIRECTORIES INCLUDE_DIRECTORIES)
set_directory_properties(PROPERTIES INCLUDE_DIRECTORIES "")

if(MRN_BUNDLED)
  set(MYSQL_SOURCE_DIR ${CMAKE_SOURCE_DIR})
  set(MYSQL_BUILD_DIR ${CMAKE_BINARY_DIR})
  set(MYSQL_PLUGIN_DIR "${INSTALL_PLUGINDIR}")
  set(MYSQL_CFLAGS "${CMAKE_C_FLAGS}")
  set(MYSQL_VERSION "${MYSQL_BASE_VERSION}")
else()
  set(MYSQL_SOURCE_DIR "/PATH/TO/MYSQL/SOURCE/DIRECTORY/"
    CACHE PATH "MySQL source directory")
  if(NOT EXISTS ${MYSQL_SOURCE_DIR})
    message(FATAL_ERROR
      "MySQL source directory (MYSQL_SOURCE_DIR) doesn't exist: <${MYSQL_SOURCE_DIR}>")
  endif()
  set(MYSQL_BUILD_DIR ${MYSQL_SOURCE_DIR} CACHE PATH "MySQL build directory")
  set(MYSQL_CONFIG "mysql_config" CACHE PATH "mysql-config command path")
  find_program(MYSQL_CONFIG "${MYSQL_CONFIG}")

  macro(SET_MYSQL_CONFIG_VALUE OPTION VARIABLE)
    if(NOT ${VARIABLE})
      execute_process(COMMAND "${MYSQL_CONFIG}" ${OPTION}
        OUTPUT_VARIABLE MYSQL_CONFIG_OUTPUT)
      string(STRIP ${MYSQL_CONFIG_OUTPUT} ${VARIABLE})
    endif()
  endmacro()

  set_mysql_config_value("--plugindir" MYSQL_PLUGIN_DIR)
  set_mysql_config_value("--variable=pkglibdir" MYSQL_PKG_LIB_DIR)
  set(MYSQL_SERVICES_LIB_STATIC
    "${MYSQL_BUILD_DIR}/archive_output_directory/libmysqlservices.a")
  if(NOT EXISTS "${MYSQL_SERVICES_LIB_STATIC}")
    set(MYSQL_SERVICES_LIB_STATIC
      "${MYSQL_BUILD_DIR}/libservices/libmysqlservices.a")
  endif()
  if(NOT EXISTS "${MYSQL_SERVICES_LIB_STATIC}")
    set(MYSQL_SERVICES_LIB_STATIC "${MYSQL_PKG_LIB_DIR}/libmysqlservices.a")
  endif()
  set(PERFSCHEMA_LIB_STATIC
    "${MYSQL_BUILD_DIR}/storage/perfschema/libperfschema.a")
  if(NOT EXISTS "${PERFSCHEMA_LIB_STATIC}")
    set(PERFSCHEMA_LIB_STATIC "${MYSQL_PKG_LIB_DIR}/libperfschema.a")
  endif()
  set_mysql_config_value("--cflags" MYSQL_CFLAGS)
  set_mysql_config_value("--version" MYSQL_VERSION)
endif()

if(MRN_GROONGA_BUNDLED)
  if(WITH_ZLIB STREQUAL "bundled")
    target_include_directories(zlib SYSTEM INTERFACE ${ZLIB_INCLUDE_DIR})
  endif()

  option(MRN_GROONGA_EMBED
    "Embed libgroonga"
    ON)
  if(MRN_GROONGA_EMBED)
    set(GRN_EMBED ON)
  endif()

  set(MRN_BUNDLED_GROONGA_NORMALIZER_MYSQL_RELATIVE_DIR
    "../../extra/groonga-normalizer-mysql")
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${MRN_BUNDLED_GROONGA_NORMALIZER_MYSQL_RELATIVE_DIR}")
    set(MRN_BUNDLED_GROONGA_NORMALIZER_MYSQL_DIR
      "${CMAKE_CURRENT_SOURCE_DIR}/${MRN_BUNDLED_GROONGA_NORMALIZER_MYSQL_RELATIVE_DIR}")
  else()
    set(MRN_BUNDLED_GROONGA_NORMALIZER_MYSQL_RELATIVE_DIR)
    set(MRN_BUNDLED_GROONGA_NORMALIZER_MYSQL_DIR
      "${MRN_BUNDLED_GROONGA_DIR}/vendor/plugins/groonga-normalizer-mysql")
  endif()
  option(MRN_GROONGA_NORMALIZER_MYSQL_EMBED
    "Embed groonga-normalizer-mysql Groonga plugin"
    ON)
  if(EXISTS ${MRN_BUNDLED_GROONGA_NORMALIZER_MYSQL_DIR})
    set(GROONGA_NORMALIZER_MYSQL_FOUND ON)
  else()
    set(GROONGA_NORMALIZER_MYSQL_FOUND OFF)
    set(MRN_GROONGA_NORMALIZER_MYSQL_EMBED OFF)
  endif()
  if(MRN_GROONGA_NORMALIZER_MYSQL_EMBED)
    set(GROONGA_NORMALIZER_MYSQL_EMBED ON)
  endif()

  function(mrn_build_groonga)
    if(MSVC)
      # Disable warning as errors defined in MariaDB
      string(APPEND CMAKE_C_FLAGS " /wd4700 /wd4311 /wd4477 /wd4302 /wd4090")
      string(APPEND CMAKE_CXX_FLAGS " /wd4099 /wd4700 /wd4311 /wd4477 /wd4302 /wd4090")
    endif()
    add_subdirectory("${MRN_BUNDLED_GROONGA_RELATIVE_DIR}" groonga)
    if(GROONGA_NORMALIZER_MYSQL_FOUND AND
        MRN_BUNDLED_GROONGA_NORMALIZER_MYSQL_RELATIVE_DIR)
      add_subdirectory("${MRN_BUNDLED_GROONGA_NORMALIZER_MYSQL_RELATIVE_DIR}"
        groonga-normalizer-mysql)
    endif()
  endfunction()
  mrn_build_groonga()
else()
  set(MRN_GROONGA_EMBED OFF)

  file(READ ${CMAKE_CURRENT_SOURCE_DIR}/required_groonga_version REQUIRED_GROONGA_VERSION)
  string(STRIP "${REQUIRED_GROONGA_VERSION}" REQUIRED_GROONGA_VERSION)

  file(READ
    ${CMAKE_CURRENT_SOURCE_DIR}/required_groonga_normalizer_mysql_version
    REQUIRED_GROONGA_NORMALIZER_MYSQL_VERSION)
  string(STRIP
    "${REQUIRED_GROONGA_NORMALIZER_MYSQL_VERSION}"
    REQUIRED_GROONGA_NORMALIZER_MYSQL_VERSION)
endif()

set(MRN_PACKAGE_STRING "${PROJECT_NAME} ${MRN_VERSION_FULL}")

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(${CMAKE_CURRENT_SOURCE_DIR}/build/cmake_modules/ReadFileList.cmake)

set(MRN_C_COMPILE_FLAGS "")
set(MRN_CXX_COMPILE_FLAGS "")

macro(mrn_check_cflag flag)
  string(REGEX REPLACE "[-=]" "_" temporary_variable_name ${flag})
  string(TOUPPER "${temporary_variable_name}" temporary_variable_name)
  set(temporary_variable_name "CFLAG${temporary_variable_name}")
  check_c_compiler_flag(${flag} ${temporary_variable_name})
  if(${temporary_variable_name})
    set(MRN_C_COMPILE_FLAGS "${MRN_C_COMPILE_FLAGS} ${flag}")
  endif()
endmacro()

macro(mrn_check_cxxflag flag)
  string(REGEX REPLACE "[-=]" "_" temporary_variable_name ${flag})
  string(TOUPPER "${temporary_variable_name}" temporary_variable_name)
  set(temporary_variable_name "CXXFLAG${temporary_variable_name}")
  check_cxx_compiler_flag(${flag} ${temporary_variable_name})
  if(${temporary_variable_name})
    set(MRN_CXX_COMPILE_FLAGS "${MRN_CXX_COMPILE_FLAGS} ${flag}")
  endif()
endmacro()

macro(mrn_build_flag flag)
  mrn_check_cflag(${flag})
  mrn_check_cxxflag(${flag})
endmacro()

read_file_list(${CMAKE_CURRENT_SOURCE_DIR}/sources.am MRN_SOURCES)
read_file_list(${CMAKE_CURRENT_SOURCE_DIR}/lib/libmrn_no_mysql_sources.am
  LIBMRN_NO_MYSQL_SOURCES)
string(REGEX REPLACE "([^;]+)" "${CMAKE_CURRENT_SOURCE_DIR}/lib/\\1"
  LIBMRN_NO_MYSQL_SOURCES "${LIBMRN_NO_MYSQL_SOURCES}")
read_file_list(${CMAKE_CURRENT_SOURCE_DIR}/lib/libmrn_need_mysql_sources.am
  LIBMRN_NEED_MYSQL_SOURCES)
string(REGEX REPLACE "([^;]+)" "${CMAKE_CURRENT_SOURCE_DIR}/lib/\\1"
  LIBMRN_NEED_MYSQL_SOURCES "${LIBMRN_NEED_MYSQL_SOURCES}")
read_file_list(${CMAKE_CURRENT_SOURCE_DIR}/udf/sources.am MRN_UDF_SOURCES)
string(REGEX REPLACE "([^;]+)" "${CMAKE_CURRENT_SOURCE_DIR}/udf/\\1"
  MRN_UDF_SOURCES "${MRN_UDF_SOURCES}")

if(EXISTS "${MYSQL_SOURCE_DIR}/storage/maria")
  set(MYSQL_VARIANT "MariaDB")
else()
  set(MYSQL_VARIANT "MySQL")
endif()

if(${MYSQL_VARIANT} STREQUAL "MariaDB")
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
else()
  if(${MYSQL_VERSION} VERSION_LESS "8.4")
    set(CMAKE_CXX_STANDARD 17)
  else()
    set(CMAKE_CXX_STANDARD 20)
  endif()
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

set(MRN_ALL_SOURCES
  ${MRN_SOURCES}
  ${MRN_UDF_SOURCES}
  ${LIBMRN_NO_MYSQL_SOURCES}
  ${LIBMRN_NEED_MYSQL_SOURCES})

if(MRN_BUNDLED)
  mysql_add_plugin(mroonga
    ${MRN_ALL_SOURCES}
    STORAGE_ENGINE MODULE_ONLY
    LINK_LIBRARIES ${MRN_LIBRARIES})
  if(NOT TARGET mroonga)
    return()
  endif()
else()
  add_library(mroonga MODULE ${MRN_ALL_SOURCES})

  if(NOT WIN32)
    set(MRN_LIBRARIES ${MRN_LIBRARIES} ${MYSQL_SERVICES_LIB_STATIC})
    if(MYSQL_VARIANT STREQUAL "MariaDB" AND EXISTS "${PERFSCHEMA_LIB_STATIC}")
      set(MRN_LIBRARIES ${MRN_LIBRARIES} ${PERFSCHEMA_LIB_STATIC})
    endif()
  endif()
  if(APPLE)
    # Need to use -bundle_loader to resolve symbols by mysqld executable.
    set(MYSQLD "mysqld" CACHE PATH "Path to mysqld executable")
    find_program(MYSQLD_ABSOLUTE ${MYSQLD} REQUIRED)
    add_executable(mysqld IMPORTED)
    set_property(TARGET mysqld PROPERTY IMPORTED_LOCATION ${MYSQLD_ABSOLUTE})
    set_property(TARGET mysqld PROPERTY ENABLE_EXPORTS TRUE)
    list(APPEND MRN_LIBRARIES mysqld)
  endif()
  target_link_libraries(mroonga PRIVATE ${MRN_LIBRARIES})

  option(WITH_DEBUG "Enable debug options" OFF)
  if(WITH_DEBUG)
    set_property(TARGET mroonga APPEND PROPERTY
      COMPILE_DEFINITIONS "SAFE_MUTEX")
    if(${MYSQL_VARIANT} STREQUAL "MariaDB")
      set_property(TARGET mroonga APPEND PROPERTY
        COMPILE_DEFINITIONS "DBUG_TRACE")
    endif()
    if(CMAKE_COMPILER_IS_GNUCXX)
      set(MRN_C_COMPILE_FLAGS "${MRN_C_COMPILE_FLAGS} -g3 -O0")
      set(MRN_CXX_COMPILE_FLAGS "${MRN_CXX_COMPILE_FLAGS} -g3 -O0")
    endif()
  else()
    set_property(TARGET mroonga APPEND PROPERTY
      COMPILE_DEFINITIONS "NDEBUG" "DBUG_OFF")
  endif()

  option(WITH_DEBUG_FULL "Enable full debug options" OFF)
  if(WITH_DEBUG_FULL)
    set_property(TARGET mroonga APPEND PROPERTY
      COMPILE_DEFINITIONS "SAFE_MUTEX" "SAFEMALLOC")
  endif()

  option(DISABLE_FAST_MUTEXES "Force disabling fast mutex" OFF)
  if(DISABLE_FAST_MUTEXES)
    set_property(TARGET mroonga APPEND PROPERTY
      COMPILE_DEFINITIONS "FORCE_FAST_MUTEX_DISABLED=1")
  endif()

  option(WITH_FAST_MUTEXES "Enable fast mutex" OFF)
  if(WITH_FAST_MUTEXES)
    set_property(TARGET mroonga APPEND PROPERTY
      COMPILE_DEFINITIONS "MY_PTHREAD_FASTMUTEX")
  endif()

  if(CMAKE_COMPILER_IS_GNUCXX)
    mrn_build_flag("-Wall")
    mrn_build_flag("-Wextra")
    mrn_build_flag("-Wno-unused-parameter")
    mrn_build_flag("-Wno-strict-aliasing")
    mrn_build_flag("-Wno-deprecated")
    mrn_check_cxxflag("-felide-constructors")
  endif()
  set_source_files_properties(${MRN_SOURCES} PROPERTIES
    COMPILE_FLAGS "${MYSQL_CFLAGS} ${MRN_CXX_COMPILE_FLAGS}")
  set_source_files_properties(${LIBMRN_NEED_MYSQL_SOURCES} PROPERTIES
    COMPILE_FLAGS "${MYSQL_CFLAGS} ${MRN_CXX_COMPILE_FLAGS}")
  set_source_files_properties(${MRN_UDF_SOURCES} PROPERTIES
    COMPILE_FLAGS "${MRN_CXX_COMPILE_FLAGS}")
  set_source_files_properties(${LIBMRN_NO_MYSQL_SOURCES} PROPERTIES
    COMPILE_FLAGS "${MRN_CXX_COMPILE_FLAGS}")
  set_property(TARGET mroonga APPEND PROPERTY
    COMPILE_DEFINITIONS "MYSQL_DYNAMIC_PLUGIN")
  set_target_properties(mroonga PROPERTIES
    PREFIX ""
    OUTPUT_NAME "ha_mroonga")

  install(TARGETS mroonga DESTINATION "${MYSQL_PLUGIN_DIR}")
endif()

if(MRN_GROONGA_BUNDLED)
  set(GROONGA_INCLUDE_DIRS
    "${MRN_BUNDLED_GROONGA_DIR}/include"
    "${CMAKE_CURRENT_BINARY_DIR}/groonga/include")
  target_link_libraries(mroonga libgroonga)
  if(TARGET mysql_normalizer AND MRN_GROONGA_NORMALIZER_MYSQL_EMBED)
    target_link_libraries(mroonga mysql_normalizer)
  endif()
else()
  include(FindPkgConfig)
  pkg_check_modules(GROONGA REQUIRED IMPORTED_TARGET
    "groonga >= ${REQUIRED_GROONGA_VERSION}")
  target_link_libraries(mroonga PRIVATE PkgConfig::GROONGA)
  pkg_check_modules(GROONGA_NORMALIZER_MYSQL
    "groonga-normalizer-mysql >= ${REQUIRED_GROONGA_NORMALIZER_MYSQL_VERSION}")
endif()

target_include_directories(mroonga BEFORE PRIVATE
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}"
  "${PROJECT_SOURCE_DIR}/lib")
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.20")
  set(TARGET_INCLUDE_DIRECTORIES_AFTER "AFTER")
else()
  set(TARGET_INCLUDE_DIRECTORIES_AFTER "")
endif()
target_include_directories(mroonga ${TARGET_INCLUDE_DIRECTORIES_AFTER} PRIVATE
  ${DEFAULT_INCLUDE_DIRECTORIES})
if(NOT MRN_BUNDLED)
  target_include_directories(mroonga ${TARGET_INCLUDE_DIRECTORIES_AFTER} PRIVATE
    "${MYSQL_BUILD_DIR}/include"
    "${MYSQL_SOURCE_DIR}/sql"
    "${MYSQL_SOURCE_DIR}/libs"
    "${MYSQL_SOURCE_DIR}/include")
  if(${MYSQL_VARIANT} STREQUAL "MySQL")
    target_include_directories(mroonga ${TARGET_INCLUDE_DIRECTORIES_AFTER} PRIVATE "${MYSQL_SOURCE_DIR}")
  endif()

  if(EXISTS "${MYSQL_SOURCE_DIR}/wsrep-lib")
    set(MYSQL_WSREP_INCLUDE_DIR "${MYSQL_SOURCE_DIR}/wsrep-lib/include")
    file(GLOB
      MYSQL_WSREP_API_H
      "${MYSQL_SOURCE_DIR}/wsrep-lib/wsrep-API/*/wsrep_api.h")
    get_filename_component(MYSQL_WSREP_API_INCLUDE_DIR
      "${MYSQL_WSREP_API_H}"
      DIRECTORY)
    target_include_directories(mroonga ${TARGET_INCLUDE_DIRECTORIES_AFTER} PRIVATE
      "${MYSQL_WSREP_INCLUDE_DIR}"
      "${MYSQL_WSREP_API_INCLUDE_DIR}")
  endif()

  if(EXISTS "${MYSQL_SOURCE_DIR}/pcre")
    set(MYSQL_REGEX_INCLUDE_DIR "${MYSQL_SOURCE_DIR}/pcre")
  else()
    set(MYSQL_REGEX_INCLUDE_DIR "${MYSQL_SOURCE_DIR}/regex")
  endif()
  target_include_directories(mroonga ${TARGET_INCLUDE_DIRECTORIES_AFTER} PRIVATE
    "${MYSQL_REGEX_INCLUDE_DIR}")

  if(EXISTS "${MYSQL_SOURCE_DIR}/extra/rapidjson")
    target_include_directories(mroonga ${TARGET_INCLUDE_DIRECTORIES_AFTER} PRIVATE
      "${MYSQL_SOURCE_DIR}/extra/rapidjson/include")
  endif()

  if(EXISTS "${MYSQL_SOURCE_DIR}/libbinlogevents")
    target_include_directories(mroonga ${TARGET_INCLUDE_DIRECTORIES_AFTER} PRIVATE
      "${MYSQL_SOURCE_DIR}/libbinlogevents/export"
      "${MYSQL_BUILD_DIR}/libbinlogevents/include"
      "${MYSQL_SOURCE_DIR}/libbinlogevents/include")
  endif()

  if(EXISTS "${MYSQL_BUILD_DIR}/extra/protobuf/protobuf-3.11.4")
    target_include_directories(mroonga ${TARGET_INCLUDE_DIRECTORIES_AFTER} PRIVATE
      "${MYSQL_SOURCE_DIR}/extra/protobuf/protobuf-3.11.4/src")
  endif()

  if(EXISTS "${MYSQL_BUILD_DIR}/extra/zlib/zlib-1.2.13")
    target_include_directories(mroonga ${TARGET_INCLUDE_DIRECTORIES_AFTER} PRIVATE
      "${MYSQL_BUILD_DIR}/extra/zlib/zlib-1.2.13"
      "${MYSQL_SOURCE_DIR}/extra/zlib/zlib-1.2.13")
  elseif(EXISTS "${MYSQL_BUILD_DIR}/extra/zlib/zlib-1.2.12")
    target_include_directories(mroonga ${TARGET_INCLUDE_DIRECTORIES_AFTER} PRIVATE
      "${MYSQL_BUILD_DIR}/extra/zlib/zlib-1.2.12"
      "${MYSQL_SOURCE_DIR}/extra/zlib/zlib-1.2.12")
  endif()

  if(EXISTS "${MYSQL_SOURCE_DIR}/extra/unordered_dense/unordered_dense-4.4.0")
    target_include_directories(mroonga ${TARGET_INCLUDE_DIRECTORIES_AFTER} PRIVATE
      "${MYSQL_SOURCE_DIR}/extra/unordered_dense/unordered_dense-4.4.0/include")
  endif()

  if(APPLE)
    find_program(BREW "brew")
    if(BREW)
      # This assumes that MySQL is installed by Homebrew.
      execute_process(COMMAND ${BREW} --prefix "openssl@1.1"
        OUTPUT_VARIABLE BREW_PREFIX_OPENSSL11
        OUTPUT_STRIP_TRAILING_WHITESPACE)
      if(BREW_PREFIX_OPENSSL11)
        target_include_directories(mroonga ${TARGET_INCLUDE_DIRECTORIES_AFTER} PRIVATE
          "${BREW_PREFIX_OPENSSL11}")
      endif()
    endif()
  endif()
endif()

if(WIN32)
  target_link_directories(mroonga PRIVATE
    "${MYSQL_BUILD_DIR}/lib"
    "${MYSQL_BUILD_DIR}/libmysqld")
endif()


option(MRN_BUILD_FOR_EMBEDDED_SERVER
  "Whether to build Mroonga for embedded server or not. You can't use Mroonga built for embedded server with non embedded server."
  OFF)
if(MRN_BUILD_FOR_EMBEDDED_SERVER)
  set_property(TARGET mroonga APPEND PROPERTY
    COMPILE_DEFINITIONS "EMBEDDED_LIBRARY")
endif()

if(GROONGA_NORMALIZER_MYSQL_FOUND)
  set_property(TARGET mroonga APPEND PROPERTY
    COMPILE_DEFINITIONS "WITH_GROONGA_NORMALIZER_MYSQL=1")
  if(MRN_GROONGA_NORMALIZER_MYSQL_EMBED)
    set_property(TARGET mroonga APPEND PROPERTY
      COMPILE_DEFINITIONS "MRN_GROONGA_NORMALIZER_MYSQL_EMBEDDED")
  else()
    set_property(TARGET mroonga APPEND PROPERTY
      COMPILE_DEFINITIONS "GROONGA_NORMALIZER_MYSQL_PLUGIN_NAME=\"normalizers/mysql\"")
  endif()
endif()

if(MRN_GROONGA_EMBED)
  set_property(TARGET mroonga APPEND PROPERTY
    COMPILE_DEFINITIONS "MRN_GROONGA_EMBEDDED")
endif()

set(MRN_DEFAULT_PARSER "" CACHE STRING
  "The default fulltext parser (Deprecated. Use MRN_DEFAULT_TOKENIZER instead.)")
set(MRN_DEFAULT_TOKENIZER "" CACHE STRING
  "The default tokenizer for fulltext index")
if(NOT ${MRN_DEFAULT_TOKENIZER} STREQUAL "")
  set_property(TARGET mroonga APPEND PROPERTY
    COMPILE_DEFINITIONS "MRN_DEFAULT_TOKENIZER=\"${MRN_DEFAULT_TOKENIZER}\"")
elseif(NOT ${MRN_DEFAULT_PARSER} STREQUAL "")
  set_property(TARGET mroonga APPEND PROPERTY
    COMPILE_DEFINITIONS "MRN_DEFAULT_TOKENIZER=\"${MRN_DEFAULT_PARSER}\"")
endif()

configure_file(
  "${PROJECT_SOURCE_DIR}/mrn_version.h.in"
  "${PROJECT_BINARY_DIR}/mrn_version.h")

configure_file(
  "${PROJECT_SOURCE_DIR}/config.sh.in"
  "${PROJECT_BINARY_DIR}/config.sh")

set(MRN_TEST_SUITE_DIR "${CMAKE_SOURCE_DIR}/mysql-test/suite/mroonga")
if(NOT EXISTS "${MRN_TEST_SUITE_DIR}")
  set(MRN_TEST_SUITE_DIR "${PROJECT_SOURCE_DIR}/mysql-test/mroonga")
endif()
configure_file(
  "${MRN_TEST_SUITE_DIR}/storage/information_schema/r/plugins.result.in"
  "${MRN_TEST_SUITE_DIR}/storage/information_schema/r/plugins.result"
  NEWLINE_STYLE LF)
configure_file(
  "${MRN_TEST_SUITE_DIR}/storage/variable/r/version.result.in"
  "${MRN_TEST_SUITE_DIR}/storage/variable/r/version.result"
  NEWLINE_STYLE LF)

include(GNUInstallDirs)
if(MRN_BUNDLED)
  set(MRN_DATA_DIR "${INSTALL_MYSQLSHAREDIR}/${PROJECT_NAME}")
else()
  set(MRN_DATA_DIR "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}")
endif()
if(WIN32 OR IS_ABSOLUTE "${MRN_DATA_DIR}")
  set(MRN_FULL_DATA_DIR "${MRN_DATA_DIR}")
else()
  set(MRN_FULL_DATA_DIR "${CMAKE_INSTALL_PREFIX}/${MRN_DATA_DIR}")
endif()
configure_file(
  "${PROJECT_SOURCE_DIR}/data/install.sql.in"
  "${PROJECT_BINARY_DIR}/data/install.sql"
  @ONLY)
configure_file(
  "${PROJECT_SOURCE_DIR}/data/update.sql.in"
  "${PROJECT_BINARY_DIR}/data/update.sql"
  @ONLY)

install(FILES
  "${PROJECT_SOURCE_DIR}/AUTHORS"
  "${PROJECT_SOURCE_DIR}/COPYING"
  "${PROJECT_BINARY_DIR}/data/install.sql"
  "${PROJECT_SOURCE_DIR}/data/uninstall.sql"
  "${PROJECT_BINARY_DIR}/data/update.sql"
  DESTINATION "${MRN_DATA_DIR}/")
install(PROGRAMS
  "${PROJECT_SOURCE_DIR}/data/deb/postinst.sh"
  "${PROJECT_SOURCE_DIR}/data/deb/prerm.sh"
  DESTINATION "${MRN_DATA_DIR}/deb/")
install(PROGRAMS
  "${PROJECT_SOURCE_DIR}/data/rpm/post.sh"
  "${PROJECT_SOURCE_DIR}/data/rpm/preun.sh"
  DESTINATION "${MRN_DATA_DIR}/rpm/")
if(MSVC)
  install(FILES $<TARGET_PDB_FILE:mroonga>
    DESTINATION "${MYSQL_PLUGIN_DIR}"
    OPTIONAL)
endif()

if(NOT MRN_BUNDLED)
  option(MRN_WITH_DOC "build document" OFF)
  if(MRN_WITH_DOC)
    add_subdirectory(doc)
  endif()
endif()
