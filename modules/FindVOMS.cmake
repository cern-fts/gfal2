#
# This module detects if voms is installed and determines where the
# include files and libraries are.
#
# This code sets the following variables:
# 
# VOMS_LIBRARIES       = full path to the voms libraries
# VOMS_INCLUDE_DIR     = include dir to be used when using the voms library
# VOMS_DEFINITIONS     = compiler flags
# VOMS_FOUND           = set to true if voms was found successfully
#
# VOMS_LOCATION
#   setting this enables search for voms libraries / headers in this location

find_package (PkgConfig)
pkg_search_module(VOMS_PKG voms-2.0 voms)

if (VOMS_PKG_FOUND)
    set (VOMS_LIBRARIES ${VOMS_PKG_LIBRARIES})
    set (VOMS_INCLUDE_DIRS ${VOMS_PKG_INCLUDE_DIRS})
    if (NOT VOMS_INCLUDE_DIRS)
        set (VOMS_INCLUDE_DIRS "/usr/include")
    endif (NOT VOMS_INCLUDE_DIRS)
    set (VOMS_DEFINITIONS "${VOMS_PKG_CFLAGS} ${VOMS_PKG_CFLAGS_OTHER}")
else (VOMS_PKG_FOUND)

    message("SEARCH FOR ${CMAKE_INSTALL_PREFIX}/Grid/voms/*/${PLATFORM}/lib64")

    find_library(VOMS_LIBRARIES
        NAMES vomsapi
        HINTS ${VOMS_LOCATION} 
              ${CMAKE_INSTALL_PREFIX}/Grid/voms/*/${PLATFORM}/lib
              ${CMAKE_INSTALL_PREFIX}/Grid/voms/*/${PLATFORM}/lib64
        DOC "The voms library"
    )

    find_path(VOMS_INCLUDE_DIRS 
        NAMES voms/voms_api.h
        HINTS ${VOMS_LOCATION}/include/*
              ${CMAKE_INSTALL_PREFIX}/Grid/voms/*/${PLATFORM}/include
        DOC "The voms include directory"
    )

    set (VOMS_CFLAGS "")
endif (VOMS_PKG_FOUND)

if (VOMS_LIBRARIES)
    message (STATUS "VOMS libraries: ${VOMS_LIBRARIES}")
endif (VOMS_LIBRARIES)
if (VOMS_INCLUDE_DIRS)
    message (STATUS "VOMS include dir: ${VOMS_INCLUDE_DIRS}")
endif (VOMS_INCLUDE_DIRS)

# -----------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set VOMS_FOUND to TRUE if 
# all listed variables are TRUE
# -----------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args (VOMS DEFAULT_MSG
    VOMS_LIBRARIES  VOMS_INCLUDE_DIRS
)
mark_as_advanced(VOMS_LIBRARIES VOMS_INCLUDE_DIRS VOMS_DEFINITIONS)
