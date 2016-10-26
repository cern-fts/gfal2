#
# This module detects if libssh2 is installed and determines where the
# include files and libraries are.
#
# This code sets the following variables:
#
# LIBSSH2_LIBRARIES       = full path to the libssh2 libraries
# LIBSSH2_INCLUDE_DIR     = include dir to be used when using the libssh2 library
# LIBSSH2_FOUND           = set to true if libssh2 was found successfully
#
# LIBSSH2_LOCATION
#   setting this enables search for libssh2 libraries / headers in this location

find_package (PkgConfig)
pkg_check_modules(LIBSSH2_PKG libssh2)

if (LIBSSH2_PKG_FOUND)
    set (LIBSSH2_LIBRARIES ${LIBSSH2_PKG_LIBRARIES})
    set (LIBSSH2_INCLUDE_DIRS ${LIBSSH2_PKG_INCLUDE_DIRS})
    set (LIBSSH2_DEFINITIONS "${LIBSSH2_PKG_CFLAGS}")
else (LIBSSH2_PKG_FOUND)
    set (CMAKE_FIND_FRAMEWORK NEVER)

    find_library(LIBSSH2_LIBRARIES
        NAMES ssh2
        HINTS ${LIBSSH2_LOCATION}
        ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/lib
        ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/lib64
        DOC "The main libssh2 library"
    )

    find_path(LIBSSH2_INCLUDE_DIRS
        NAMES libssh2_sftp.h
        HINTS ${LIBSSH2_LOCATION}/include/*
        ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/include
        DOC "The libssh2 include directory"
    )

    set (LIBSSH2_DEFINITIONS "")
endif (LIBSSH2_PKG_FOUND)

if (LIBSSH2_LIBRARIES)
    message (STATUS "LIBSSH2 libraries: ${LIBSSH2_LIBRARIES}")
endif (LIBSSH2_LIBRARIES)
if (LIBSSH2_INCLUDE_DIRS)
    message (STATUS "LIBSSH2 include dir: ${LIBSSH2_INCLUDE_DIRS}")
endif (LIBSSH2_INCLUDE_DIRS)

# -----------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set LIBSSH2_FOUND to TRUE if
# all listed variables are TRUE
# -----------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args (SSH2 DEFAULT_MSG
    LIBSSH2_LIBRARIES  LIBSSH2_INCLUDE_DIRS
)
mark_as_advanced(LIBSSH2_INCLUDE_DIRS LIBSSH2_LIBRARIES)
