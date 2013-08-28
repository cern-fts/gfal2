## cmake build script for srm-ifce
project (gfal2)
cmake_minimum_required (VERSION 2.6)
message("cmake source dir : ${CMAKE_SOURCE_DIR}")

# default set of options
SET(MAIN_CORE TRUE
        CACHE STRING "enable compilation of the main library")
SET(MAIN_TRANSFER TRUE
        CACHE STRING "enable compilation of the transfer library")

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

set(VERSION_MAJOR  2)
set(VERSION_MINOR  4)
set(VERSION_PATCH  4)
set(VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})


# out of project test compilation
SET(ONLY_TESTS FALSE
	CACHE STRING "enable or disable out of project dir")

# libs checks
pkg_check_modules(GLIB2_PKG REQUIRED glib-2.0)
pkg_check_modules(GTHREAD2_PKG REQUIRED gthread-2.0)

# include directories
include_directories(${GLIB2_PKG_INCLUDE_DIRS})
include_directories(${GTHREAD2_PKG_INCLUDE_DIRS})

# Core and plugins
IF(NOT ONLY_TESTS)
    pkg_search_module(GLIBMM_PKG REQUIRED glibmm glibmm-2.6 glibmm-2.4)
    pkg_check_modules(UUID_PKG REQUIRED uuid)

    include_directories(${GLIBMM_PKG_INCLUDE_DIRS})

   # general parameters for configuration
    add_definitions( -DGFAL_PLUGIN_DIR_DEFAULT="${PLUGIN_INSTALL_DIR}" )
    add_definitions( -DGFAL_CONFIG_DIR_DEFAULT="${SYSCONF_INSTALL_DIR}" )
    # add file offset 64 for 32 bits
    add_definitions( -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_GNU_SOURCE)


    include_directories(${CMAKE_SOURCE_DIR}/src/core)
    include_directories(${CMAKE_SOURCE_DIR}/src/utils)

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

