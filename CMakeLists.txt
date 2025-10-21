## cmake build script for srm-ifce
project (gfal2)
cmake_minimum_required (VERSION 2.6)
message ("cmake source dir : ${CMAKE_SOURCE_DIR}")

# default set of options
set (MAIN_CORE      TRUE CACHE STRING "enable compilation of the main library")
set (MAIN_TRANSFER  TRUE CACHE STRING "enable compilation of the transfer library")
set (PLUGIN_SFTP    TRUE CACHE STRING "enable compilation of the sftp plugin")
set (PLUGIN_SRM     TRUE CACHE STRING "enable compilation of the srm plugin")
set (PLUGIN_DCAP    TRUE CACHE STRING "enable compilation of the DCAP plugin")
set (PLUGIN_FILE    TRUE CACHE STRING "enable compilation of the local File plugin")
set (PLUGIN_GRIDFTP TRUE CACHE STRING "enable compilation of the GRIDFTP plugin")
set (PLUGIN_HTTP    TRUE CACHE STRING "enable compilation of the HTTP plugin")
set (PLUGIN_XROOTD  TRUE CACHE STRING "enable compilation of the XROOTD plugin")
set (PLUGIN_MOCK    FALSE CACHE STRING "enable compilation of the MOCK plugin")
# Deprecated / unsupported protocols after CC7
set (PLUGIN_LFC     FALSE CACHE STRING "enable compilation of the lfc plugin")
set (PLUGIN_RFIO    FALSE CACHE STRING "enable compilation of the RFIO plugin")

# build type
set (CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "type of build")

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

# load modules
include(DefineInstallationPaths REQUIRED)
include(MacroCopyFile REQUIRED)
include(ReleaseDebugAutoFlags REQUIRED)
include(CMakeGeneratePkgConfig REQUIRED)
include(MacroAddDoxygen REQUIRED)
include(CMakeCXX11Support REQUIRED)

# Enable C++11 support
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_FLAG_ENABLE}")

# enable testing
include (CTest)

#define PROJECT vars
set (PROJECT_NAME_MAIN "gfal2")
set (OUTPUT_NAME_MAIN "gfal2")

set (PROJECT_NAME_TRANSFER "gfal_transfer")
set (OUTPUT_NAME_TRANSFER "gfal_transfer")

set (VERSION_MAJOR  2)
set (VERSION_MINOR  23)
set (VERSION_PATCH  5)
set (VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

add_definitions(-DVERSION="${VERSION_STRING}")

# out of project test compilation
set (ONLY_TESTS FALSE CACHE STRING "build only the tests")
set (SKIP_TESTS FALSE CACHE STRING "skip the tests")

# libs checks
find_package (GLIB2 REQUIRED)
find_package (GTHREAD2 REQUIRED)
find_package (JSONC REQUIRED)

# include directories
include_directories (${GLIB2_INCLUDE_DIRS})
include_directories (${GTHREAD2_INCLUDE_DIRS})
include_directories (${CMAKE_SOURCE_DIR}/src/core)
include_directories (${CMAKE_SOURCE_DIR}/src/utils)

# Core and plugins
if (NOT ONLY_TESTS)

    # general parameters for configuration
    add_definitions ( -DGFAL_PLUGIN_DIR_DEFAULT="${PLUGIN_INSTALL_DIR}" )
    add_definitions ( -DGFAL_CONFIG_DIR_DEFAULT="${SYSCONF_INSTALL_DIR}" )
    # add file offset 64 for 32 bits
    add_definitions ( -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_GNU_SOURCE)

    #install doc license
    install (FILES "LICENSE" "README.md" "RELEASE-NOTES" "DESCRIPTION" "readme.html"
			 DESTINATION ${DOC_INSTALL_DIR})

    # install global configuration files
    list (APPEND lst_conf_files
        "${CMAKE_SOURCE_DIR}/dist/etc/gfal2.d/gfal2_core.conf"
        "${CMAKE_SOURCE_DIR}/dist/etc/gfal2.d/bdii.conf"
        "${CMAKE_SOURCE_DIR}/dist/etc/gfal2.d/x509.conf")
    install (FILES ${lst_conf_files}
             DESTINATION ${SYSCONF_INSTALL_DIR}/gfal2.d/)

    add_subdirectory (doc)
    add_subdirectory (src)

endif (NOT ONLY_TESTS)

if (NOT SKIP_TESTS)
    add_subdirectory (test)
endif (NOT SKIP_TESTS)

