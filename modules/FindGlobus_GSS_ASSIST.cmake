#
# This module detects if globus-gssapi-gsi is installed and determines where the
# include files and libraries are.
#
# This code sets the following variables:
# 
# GLOBUS_GSS_ASSIST_LIBRARIES       = full path to the globus-gssapi-gsi libraries
# GLOBUS_GSS_ASSIST_INCLUDE_DIR     = include dir to be used when using the globus-gssapi-gsi library
# GLOBUS_GSS_ASSIST_FOUND           = set to true if globus-gssapi-gsi was found successfully
#
# GLOBUS_GSS_ASSIST_LOCATION
#   setting this enables search for globus-gssapi-gsi libraries / headers in this location

find_package (PkgConfig)
pkg_check_modules(GLOBUS_GSS_ASSIST_PKG globus-gss-assist)

if (GLOBUS_GSS_ASSIST_PKG_FOUND)
    set (GLOBUS_GSS_ASSIST_LIBRARIES ${GLOBUS_GSS_ASSIST_PKG_LIBRARIES})
    set (GLOBUS_GSS_ASSIST_INCLUDE_DIRS ${GLOBUS_GSS_ASSIST_PKG_INCLUDE_DIRS})
    set (GLOBUS_GSS_ASSIST_DEFINITIONS "${GLOBUS_GSS_ASSIST_PKG_CFLAGS}")
else (GLOBUS_GSS_ASSIST_PKG_FOUND)

    find_library(GLOBUS_GSS_ASSIST_LIBRARIES
        NAMES globus_gss_assist
        HINTS ${GLOBUS_GSS_ASSIST_LOCATION} 
              ${CMAKE_INSTALL_PREFIX}/globus/*/${PLATFORM}/
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/lib
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/lib64
              ${CMAKE_INSTALL_PREFIX}/opt/globus-toolkit/libexec/lib
              ${GLOBUS_PREFIX}/libexec/lib
        DOC "The main globus-gssapi-gsi library"
    )

    find_path(GLOBUS_GSS_ASSIST_INCLUDE_DIRS 
        NAMES globus_gss_assist.h
        HINTS ${GLOBUS_GSS_ASSIST_LOCATION}/include/*
              ${CMAKE_INSTALL_PREFIX}/globus/*/${PLATFORM}/include
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/include
              ${CMAKE_INSTALL_PREFIX}/opt/globus-toolkit/libexec/include
              ${GLOBUS_PREFIX}/libexec/include
        DOC "The globus-gss-assist include directory"
    )

    set (GLOBUS_GSS_ASSIST_DEFINITIONS "")
endif (GLOBUS_GSS_ASSIST_PKG_FOUND)

if (GLOBUS_GSS_ASSIST_LIBRARIES)
    message (STATUS "GLOBUS GSS ASSIST libraries: ${GLOBUS_GSS_ASSIST_LIBRARIES}")
endif (GLOBUS_GSS_ASSIST_LIBRARIES)
if (GLOBUS_GSS_ASSIST_INCLUDE_DIRS)
    message (STATUS "GLOBUS GSS ASSIST include dir: ${GLOBUS_GSS_ASSIST_INCLUDE_DIRS}")
endif (GLOBUS_GSS_ASSIST_INCLUDE_DIRS)

# -----------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set GLOBUS_GSS_ASSIST_FOUND to TRUE if 
# all listed variables are TRUE
# -----------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args (GLOBUS_ASSIST DEFAULT_MSG
    GLOBUS_GSS_ASSIST_LIBRARIES  GLOBUS_GSS_ASSIST_INCLUDE_DIRS
)
mark_as_advanced(GLOBUS_GSS_ASSIST_INCLUDE_DIRS GLOBUS_GSS_ASSIST_LIBRARIES)
