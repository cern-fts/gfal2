#
# This module detects if gthread2 is installed and determines where the
# include files and libraries are.
#
# This code sets the following variables:
# 
# GTHREAD2_LIBRARIES       = full path to the glib2 libraries
# GTHREAD2_INCLUDE_DIR     = include dir to be used when using the glib2 library
# GTHREAD2_FOUND           = set to true if glib2 was found successfully
#
# GTHREAD2_LOCATION
#   setting this enables search for gthread2 libraries / headers in this location

find_package (PkgConfig)
pkg_check_modules(GTHREAD2_PKG gthread-2.0)

if (GTHREAD2_PKG_FOUND)
    set (GTHREAD2_LIBRARIES ${GTHREAD2_PKG_LIBRARIES})
    set (GTHREAD2_INCLUDE_DIRS ${GTHREAD2_PKG_INCLUDE_DIRS})
    set (GTHREAD2_DEFINITIONS "${GTHREAD2_PKG_CFLAGS} ${GTHREAD2_PKG_CFLAGS_OTHER}")
else (GTHREAD2_PKG_FOUND)

    find_library(GTHREAD2_LIBRARIES
        NAMES libgthread-2.0.so.0
        HINTS ${GTHREAD2_LOCATION} 
              ${CMAKE_INSTALL_PREFIX}/glib2/*/${PLATFORM}/
        DOC "The main gthread2 library"
    )

    find_path(GTHREAD2_INCLUDE_DIRS 
        NAMES gthread.h
        HINTS ${GTHREAD2_LOCATION}/include/*
              ${CMAKE_INSTALL_PREFIX}/glib2/*/${PLATFORM}/
        DOC "The gthread2 include directory"
    )

    set (GTHREAD2_DEFINITIONS "")
endif (GTHREAD2_PKG_FOUND)

if (GTHREAD2_LIBRARIES)
    message (STATUS "GTHREAD2 libraries: ${GTHREAD2_LIBRARIES}")
endif (GTHREAD2_LIBRARIES)
if (GTHREAD2_INCLUDE_DIRS)
    message (STATUS "GTHREAD2 include dir: ${GTHREAD2_INCLUDE_DIRS}")
endif (GTHREAD2_INCLUDE_DIRS)

# -----------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set GTHREAD2_FOUND to TRUE if 
# all listed variables are TRUE
# -----------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args (GTHREAD2 DEFAULT_MSG
    GTHREAD2_LIBRARIES  GTHREAD2_INCLUDE_DIRS
)
mark_as_advanced(GTHREAD2_INCLUDE_DIRS GTHREAD2_LIBRARIES)
