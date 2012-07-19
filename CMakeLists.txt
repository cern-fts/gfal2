## cmake build script for srm-ifce
project (gfal2)
cmake_minimum_required (VERSION 2.6)
message("cmake source dir : ${CMAKE_SOURCE_DIR}")
# build type
set(CMAKE_BUILD_TYPE RelWithDebInfo
	CACHE STRING "type of build")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")
# load module
include(DefineInstallationPaths REQUIRED)
include(MacroCopyFile REQUIRED)
include(ReleaseDebugAutoFlags REQUIRED)
include(CMakeGeneratePkgConfig REQUIRED)
include(MacroAddDoxygen REQUIRED)

find_package(PkgConfig REQUIRED)

find_package(OpenMP REQUIRED)

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

set(VERSION_MAJOR 2)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})




# libs checks
SET(CGREEN_LIBRARIES "cgreen")

pkg_check_modules(GLIB2_PKG REQUIRED glib-2.0)
pkg_check_modules(GTHREAD2_PKG REQUIRED gthread-2.0)

pkg_check_modules(UUID_PKG REQUIRED uuid)

pkg_search_module(GLIBMM_PKG REQUIRED glibmm glibmm-2.6 glibmm-2.4)


# general parameters for configuration
add_definitions( -DGFAL_PLUGIN_DIR_DEFAULT="${PLUGIN_INSTALL_DIR}" )
add_definitions( -DGFAL_CONFIG_DIR_DEFAULT="${SYSCONF_INSTALL_DIR}" )


#install doc license
install(FILES "LICENSE" "README" "RELEASE-NOTES" "VERSION" "DESCRIPTION"
			DESTINATION ${DOC_INSTALL_DIR})
			
# install configuration files
FILE(GLOB lst_conf_files "${CMAKE_SOURCE_DIR}/dist/etc/gfal2.d/*")
install(FILES ${lst_conf_files}
			DESTINATION ${SYSCONF_INSTALL_DIR}/gfal2.d/)

add_subdirectory (doc)
add_subdirectory (src)
add_subdirectory (test)
add_subdirectory (testing)
