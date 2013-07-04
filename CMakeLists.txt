## cmake build script for srm-ifce
project (gfal2)
cmake_minimum_required (VERSION 2.6)
message("cmake source dir : ${CMAKE_SOURCE_DIR}")
# default set of options
SET(MAIN_CORE TRUE
        CACHE STRING "enable compilation of the main library")
SET(PLUGIN_LFC TRUE
        CACHE STRING "enable compilation of the lfc plugin")
SET(PLUGIN_SRM TRUE
        CACHE STRING "enable compilation of the srm plugin")
SET(PLUGIN_RFIO TRUE
        CACHE STRING "enable compilation of the RFIO plugin")
SET(PLUGIN_DCAP TRUE
        CACHE STRING "enable compilation of the DCAP plugin")
SET(PLUGIN_FILE TRUE
        CACHE STRING "enable compilation of the local File plugin")
SET(PLUGIN_GRIDFTP TRUE
        CACHE STRING "enable compilation of the GRIDFTP plugin")
SET(PLUGIN_HTTP TRUE
        CACHE STRING "enable compilation of the HTTP plugin")

SET(MAIN_TRANSFER TRUE
        CACHE STRING "enable compilation of the transfer library")

# build type
set(CMAKE_BUILD_TYPE "RelWithDebInfo"
	CACHE STRING "type of build")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")
# load module
include(DefineInstallationPaths REQUIRED)
include(MacroCopyFile REQUIRED)
include(ReleaseDebugAutoFlags REQUIRED)
include(CMakeGeneratePkgConfig REQUIRED)
include(MacroAddDoxygen REQUIRED)

find_package(PkgConfig REQUIRED)

#enable testing
INCLUDE(CTest)

#define PROJECT vars
set(PROJECT_NAME_MAIN "gfal2")
set(OUTPUT_NAME_MAIN "gfal2")

set(PROJECT_NAME_TRANSFER "gfal_transfer")
set(OUTPUT_NAME_TRANSFER "gfal_transfer")

set(PROJECT_NAME_LFC "plugin_lfc")
set(OUTPUT_NAME_LFC "gfal_plugin_lfc")

set(PROJECT_NAME_SRM "plugin_srm")
set(OUTPUT_NAME_SRM "gfal_plugin_srm")

set(PROJECT_NAME_RFIO "plugin_rfio")
set(OUTPUT_NAME_RFIO "gfal_plugin_rfio")

set(PROJECT_NAME_DCAP "plugin_dcap")
set(OUTPUT_NAME_DCAP "gfal_plugin_dcap")

set(PROJECT_NAME_GRIDFTP "plugin_gridftp")
set(OUTPUT_NAME_GRIDFTP "gfal_plugin_gridftp")

set(PROJECT_NAME_FILE "plugin_file")
set(OUTPUT_NAME_FILE "gfal_plugin_file")

set(PROJECT_NAME_HTTP "plugin_http")
set(OUTPUT_NAME_HTTP "gfal_plugin_http")

set(VERSION_MAJOR  2)
set(VERSION_MINOR  3)
set(VERSION_PATCH 0)
set(VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})


# out of project test compilation
SET(ONLY_TESTS FALSE
	CACHE STRING "enable or disable out of project dir")


IF(NOT ONLY_TESTS)

# libs checks
pkg_check_modules(GLIB2_PKG REQUIRED glib-2.0)
pkg_check_modules(GTHREAD2_PKG REQUIRED gthread-2.0)
pkg_check_modules(UUID_PKG REQUIRED uuid)
pkg_search_module(GLIBMM_PKG REQUIRED glibmm glibmm-2.6 glibmm-2.4)

IF(PLUGIN_SRM)
pkg_check_modules(SRM_IFCE_PKG REQUIRED srm-ifce>=1.15.0)
ENDIF(PLUGIN_SRM)

IF(PLUGIN_HTTP)
pkg_check_modules(GSOAP_SSL_PKG REQUIRED "gsoapssl++")
pkg_check_modules(DAVIX_PKG REQUIRED "davix>=0.0.20")
find_package(gSOAP REQUIRED)
ENDIF(PLUGIN_HTTP)

IF(PLUGIN_GRIDFTP)
pkg_check_modules(GLOBUS_GASS_COPY_PKG REQUIRED globus-gass-copy)
ENDIF(PLUGIN_GRIDFTP)


# general parameters for configuration
add_definitions( -DGFAL_PLUGIN_DIR_DEFAULT="${PLUGIN_INSTALL_DIR}" )
add_definitions( -DGFAL_CONFIG_DIR_DEFAULT="${SYSCONF_INSTALL_DIR}" )
# add file offset 64 for 32 bits
add_definitions( -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_GNU_SOURCE)

include_directories( ${GLIB2_PKG_INCLUDE_DIRS} ${GTHREAD2_PKG_INCLUDE_DIRS})


#install doc license
install(FILES "LICENSE" "README" "RELEASE-NOTES" "DESCRIPTION"
			DESTINATION ${DOC_INSTALL_DIR})
			
# install configuration files
FILE(GLOB lst_conf_files "${CMAKE_SOURCE_DIR}/dist/etc/gfal2.d/*.conf")
install(FILES ${lst_conf_files}
			DESTINATION ${SYSCONF_INSTALL_DIR}/gfal2.d/)


add_subdirectory (doc)
add_subdirectory (src)
ENDIF(NOT ONLY_TESTS)


add_subdirectory (test)

