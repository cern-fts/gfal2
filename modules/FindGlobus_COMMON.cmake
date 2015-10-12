#
# This module detects if globus-common is installed and determines where the
# include files and libraries are.
#
# This code sets the following variables:
# 
# GLOBUS_COMMON_LIBRARIES       = full path to the globus-common libraries
# GLOBUS_COMMON_INCLUDE_DIR     = include dir to be used when using the globus-common library
# GLOBUS_COMMON_FOUND           = set to true if globus-common was found successfully
#
# GLOBUS_COMMON_LOCATION
#   setting this enables search for globus-common libraries / headers in this location

find_package (PkgConfig)
pkg_check_modules(GLOBUS_COMMON_PKG globus-common)

if (GLOBUS_COMMON_PKG_FOUND)
    set (GLOBUS_COMMON_LIBRARIES ${GLOBUS_COMMON_PKG_LIBRARIES})
    set (GLOBUS_COMMON_INCLUDE_DIRS ${GLOBUS_COMMON_PKG_INCLUDE_DIRS})
    set (GLOBUS_COMMON_DEFINITIONS "${GLOBUS_COMMON_PKG_CFLAGS}")
else (GLOBUS_COMMON_PKG_FOUND)

    set (CMAKE_FIND_FRAMEWORK NEVER)

    find_library(GLOBUS_COMMON_LIBRARIES
        NAMES globus_common
        HINTS ${GLOBUS_COMMON_LOCATION} 
              ${CMAKE_INSTALL_PREFIX}/globus/*/${PLATFORM}/
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/lib
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/lib64
              ${CMAKE_INSTALL_PREFIX}/opt/globus-toolkit/libexec/lib
              ${GLOBUS_PREFIX}/libexec/lib
        DOC "The main globus-common library"
    )

    find_path(GLOBUS_COMMON_INCLUDE_DIRS 
        NAMES globus_common.h
        HINTS ${GLOBUS_COMMON_LOCATION}/include/*
              ${CMAKE_INSTALL_PREFIX}/globus/*/${PLATFORM}/include
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/include
              ${CMAKE_INSTALL_PREFIX}/globus-toolkit/libexec/include
              ${GLOBUS_PREFIX}/libexec/include
        DOC "The globus-common include directory"
    )

    set (GLOBUS_COMMON_DEFINITIONS "")
endif (GLOBUS_COMMON_PKG_FOUND)

if (GLOBUS_COMMON_LIBRARIES)
    message (STATUS "GLOBUS_COMMON libraries: ${GLOBUS_COMMON_LIBRARIES}")
endif (GLOBUS_COMMON_LIBRARIES)
if (GLOBUS_COMMON_INCLUDE_DIRS)
    message (STATUS "GLOBUS_COMMON include dir: ${GLOBUS_COMMON_INCLUDE_DIRS}")
endif (GLOBUS_COMMON_INCLUDE_DIRS)

# -----------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set GLOBUS_COMMON_FOUND to TRUE if 
# all listed variables are TRUE
# -----------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args (GLOBUS_COMMON DEFAULT_MSG
    GLOBUS_COMMON_LIBRARIES  GLOBUS_COMMON_INCLUDE_DIRS
)
mark_as_advanced(GLOBUS_COMMON_INCLUDE_DIRS GLOBUS_COMMON_LIBRARIES)
