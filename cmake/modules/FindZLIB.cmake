#
# This module detects if zlib is installed and determines where the
# include files and libraries are.
#
# This code sets the following variables:
# 
# ZLIB_LIBRARIES       = full path to the zlib libraries
# ZLIB_INCLUDE_DIR     = include dir to be used when using the glib2 library
# ZLIB_FOUND           = set to true if zlib was found successfully
#
# ZLIB_LOCATION
#   setting this enables search for zlib libraries / headers in this location

find_package (PkgConfig)
pkg_check_modules(ZLIB_PKG zlib)

if (ZLIB_PKG_FOUND)
    set (ZLIB_LIBRARIES ${ZLIB_PKG_LIBRARIES})
    set (ZLIB_INCLUDE_DIRS ${ZLIB_PKG_INCLUDE_DIRS})
    if (NOT ZLIB_INCLUDE_DIRS)
        set (ZLIB_INCLUDE_DIRS "/usr/include")
    endif (NOT ZLIB_INCLUDE_DIRS)
    set (ZLIB_DEFINITIONS "${ZLIB_PKG_CFLAGS} ${ZLIB_PKG_CFLAGS_OTHER}")
else (ZLIB_PKG_FOUND)

    find_library(ZLIB_LIBRARIES
        NAMES zlib
        HINTS ${ZLIB_LOCATION} 
              ${CMAKE_INSTALL_PREFIX}/zlib/*/${PLATFORM}/
        DOC "The main glib2 library"
    )

    find_path(ZLIB_INCLUDE_DIRS 
        NAMES zlib.h
        HINTS ${ZLIB_LOCATION}/include/*
              ${CMAKE_INSTALL_PREFIX}/zlib/*/${PLATFORM}/
        DOC "The glib2 include directory"
    )

    set (ZLIB_DEFINITIONS "")
endif (ZLIB_PKG_FOUND)

if (ZLIB_LIBRARIES)
    message (STATUS "ZLIB libraries: ${ZLIB_LIBRARIES}")
endif (ZLIB_LIBRARIES)
if (ZLIB_INCLUDE_DIRS)
    message (STATUS "ZLIB include dir: ${ZLIB_INCLUDE_DIRS}")
endif (ZLIB_INCLUDE_DIRS)

# -----------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set ZLIB_FOUND to TRUE if 
# all listed variables are TRUE
# -----------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args (ZLIB DEFAULT_MSG
    ZLIB_LIBRARIES  ZLIB_INCLUDE_DIRS
)
mark_as_advanced(ZLIB_INCLUDE_DIRS ZLIB_LIBRARIES)
