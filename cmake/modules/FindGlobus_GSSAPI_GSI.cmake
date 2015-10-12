#
# This module detects if globus-gssapi-gsi is installed and determines where the
# include files and libraries are.
#
# This code sets the following variables:
# 
# GLOBUS_GSSAPI_GSI_LIBRARIES       = full path to the globus-gssapi-gsi libraries
# GLOBUS_GSSAPI_GSI_INCLUDE_DIR     = include dir to be used when using the globus-gssapi-gsi library
# GLOBUS_GSSAPI_GSI_FOUND           = set to true if globus-gssapi-gsi was found successfully
#
# GLOBUS_GSSAPI_GSI_LOCATION
#   setting this enables search for globus-gssapi-gsi libraries / headers in this location

find_package (PkgConfig)
pkg_check_modules(GLOBUS_GSSAPI_GSI_PKG globus-gssapi-gsi)

if (GLOBUS_GSSAPI_GSI_PKG_FOUND)
    set (GLOBUS_GSSAPI_GSI_LIBRARIES ${GLOBUS_GSSAPI_GSI_PKG_LIBRARIES})
    set (GLOBUS_GSSAPI_GSI_INCLUDE_DIRS ${GLOBUS_GSSAPI_GSI_PKG_INCLUDE_DIRS})
    set (GLOBUS_GSSAPI_GSI_DEFINITIONS "${GLOBUS_GSSAPI_GSI_PKG_CFLAGS}")
else (GLOBUS_GSSAPI_GSI_PKG_FOUND)

    set (CMAKE_FIND_FRAMEWORK NEVER)

    find_library(GLOBUS_GSSAPI_GSI_LIBRARIES
        NAMES globus_gssapi_gsi
        HINTS ${GLOBUS_GSSAPI_GSI_LOCATION} 
              ${CMAKE_INSTALL_PREFIX}/globus/*/${PLATFORM}/
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/lib
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/lib64
              ${CMAKE_INSTALL_PREFIX}/opt/globus-toolkit/libexec/lib
              ${GLOBUS_PREFIX}/libexec/lib
        DOC "The main globus-gssapi-gsi library"
    )

    find_path(GLOBUS_GSSAPI_GSI_INCLUDE_DIRS 
        NAMES gssapi.h
        HINTS ${GLOBUS_GSSAPI_GSI_LOCATION}/include/*
              ${CMAKE_INSTALL_PREFIX}/globus/*/${PLATFORM}/include
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/include
              ${CMAKE_INSTALL_PREFIX}/opt/globus-toolkit/libexec/include
              ${GLOBUS_PREFIX}/libexec/include
        DOC "The globus-gssapi-gsi include directory"
    )

    set (GLOBUS_GSSAPI_GSI_DEFINITIONS "")
endif (GLOBUS_GSSAPI_GSI_PKG_FOUND)

if (GLOBUS_GSSAPI_GSI_LIBRARIES)
    message (STATUS "GLOBUS_GSSAPI_GSI libraries: ${GLOBUS_GSSAPI_GSI_LIBRARIES}")
endif (GLOBUS_GSSAPI_GSI_LIBRARIES)
if (GLOBUS_GSSAPI_GSI_INCLUDE_DIRS)
    message (STATUS "GLOBUS_GSSAPI_GSI include dir: ${GLOBUS_GSSAPI_GSI_INCLUDE_DIRS}")
endif (GLOBUS_GSSAPI_GSI_INCLUDE_DIRS)

# -----------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set GLOBUS_GSSAPI_GSI_FOUND to TRUE if 
# all listed variables are TRUE
# -----------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args (GLOBUS_GSSAPI_GSI DEFAULT_MSG
    GLOBUS_GSSAPI_GSI_LIBRARIES  GLOBUS_GSSAPI_GSI_INCLUDE_DIRS
)
mark_as_advanced(GLOBUS_GSSAPI_GSI_INCLUDE_DIRS GLOBUS_GSSAPI_GSI_LIBRARIES)
