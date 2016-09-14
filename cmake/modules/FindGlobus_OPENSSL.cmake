#
# This module detects if globus-openssl is installed and determines where the
# include files and libraries are.
#
# This code sets the following variables:
# 
# GLOBUS_OPENSSL_LIBRARIES       = full path to the globus-openssl libraries
# GLOBUS_OPENSSL_INCLUDE_DIR     = include dir to be used when using the globus-openssl library
# GLOBUS_OPENSSL_FOUND           = set to true if globus-openssl was found successfully
#
# GLOBUS_OPENSSL_LOCATION
#   setting this enables search for globus-openssl libraries / headers in this location

find_package (PkgConfig)
pkg_check_modules(GLOBUS_OPENSSL_PKG globus-openssl-module)

if (GLOBUS_OPENSSL_PKG_FOUND)
    set (GLOBUS_OPENSSL_LIBRARIES ${GLOBUS_OPENSSL_PKG_LIBRARIES})
    set (GLOBUS_OPENSSL_INCLUDE_DIRS ${GLOBUS_OPENSSL_PKG_INCLUDE_DIRS})
    set (GLOBUS_OPENSSL_DEFINITIONS "${GLOBUS_OPENSSL_PKG_CFLAGS}")
else (GLOBUS_OPENSSL_PKG_FOUND)

    set (CMAKE_FIND_FRAMEWORK NEVER)

    find_library(GLOBUS_OPENSSL_LIBRARIES
        NAMES globus_openssl
        HINTS ${GLOBUS_OPENSSL_LOCATION} 
              ${CMAKE_INSTALL_PREFIX}/globus/*/${PLATFORM}/
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/lib
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/lib64
              ${CMAKE_INSTALL_PREFIX}/opt/globus-toolkit/libexec/lib
              ${GLOBUS_PREFIX}/libexec/lib
        DOC "The main globus-openssl library"
    )

    find_path(GLOBUS_OPENSSL_INCLUDE_DIRS 
        NAMES globus_openssl.h
        HINTS ${GLOBUS_OPENSSL_LOCATION}/include/*
              ${CMAKE_INSTALL_PREFIX}/globus/*/${PLATFORM}/include
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/include
              ${CMAKE_INSTALL_PREFIX}/opt/globus-toolkit/libexec/include
              ${GLOBUS_PREFIX}/libexec/include
        DOC "The globus-openssl include directory"
    )

    set (GLOBUS_OPENSSL_DEFINITIONS "")
endif (GLOBUS_OPENSSL_PKG_FOUND)

if (GLOBUS_OPENSSL_LIBRARIES)
    message (STATUS "GLOBUS_OPENSSL libraries: ${GLOBUS_OPENSSL_LIBRARIES}")
endif (GLOBUS_OPENSSL_LIBRARIES)
if (GLOBUS_OPENSSL_INCLUDE_DIRS)
    message (STATUS "GLOBUS_OPENSSL include dir: ${GLOBUS_OPENSSL_INCLUDE_DIRS}")
endif (GLOBUS_OPENSSL_INCLUDE_DIRS)

# -----------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set GLOBUS_OPENSSL_FOUND to TRUE if 
# all listed variables are TRUE
# -----------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args (GLOBUS_OPENSSL DEFAULT_MSG
    GLOBUS_OPENSSL_LIBRARIES  GLOBUS_OPENSSL_INCLUDE_DIRS
)
mark_as_advanced(GLOBUS_OPENSSL_INCLUDE_DIRS GLOBUS_OPENSSL_LIBRARIES)
