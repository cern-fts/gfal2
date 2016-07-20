#
# This module detects if globus-ftp-control is installed and determines where the
# include files and libraries are.
#
# This code sets the following variables:
# 
# GLOBUS_FTP_CONTROL_LIBRARIES       = full path to the globus-ftp-control libraries
# GLOBUS_FTP_CONTROL_INCLUDE_DIR     = include dir to be used when using the globus-ftp-control library
# GLOBUS_FTP_CONTROL_FOUND           = set to true if globus-ftp-control was found successfully
#
# GLOBUS_FTP_CONTROL_LOCATION
#   setting this enables search for globus-ftp-control libraries / headers in this location

find_package (PkgConfig)
pkg_check_modules(GLOBUS_FTP_CONTROL_PKG globus-ftp-control)

if (GLOBUS_FTP_CONTROL_PKG_FOUND)
    set (GLOBUS_FTP_CONTROL_LIBRARIES ${GLOBUS_FTP_CONTROL_PKG_LIBRARIES})
    set (GLOBUS_FTP_CONTROL_INCLUDE_DIRS ${GLOBUS_FTP_CONTROL_PKG_INCLUDE_DIRS})
    set (GLOBUS_FTP_CONTROL_DEFINITIONS "${GLOBUS_FTP_CONTROL_PKG_CFLAGS}")
else (GLOBUS_FTP_CONTROL_PKG_FOUND)

    set (CMAKE_FIND_FRAMEWORK NEVER)

    find_library(GLOBUS_FTP_CONTROL_LIBRARIES
        NAMES globus_ftp_control
        HINTS ${GLOBUS_FTP_CONTROL_LOCATION} 
              ${CMAKE_INSTALL_PREFIX}/globus/*/${PLATFORM}/
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/lib
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/lib64
              ${CMAKE_INSTALL_PREFIX}/opt/globus-toolkit/libexec/lib
              ${GLOBUS_PREFIX}/libexec/lib
        DOC "The main globus-ftp-control library"
    )

    find_path(GLOBUS_FTP_CONTROL_INCLUDE_DIRS 
        NAMES globus_ftp_control.h
        HINTS ${GLOBUS_FTP_CONTROL_LOCATION}/include/*
              ${CMAKE_INSTALL_PREFIX}/globus/*/${PLATFORM}/include
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/include
              ${CMAKE_INSTALL_PREFIX}/opt/globus-toolkit/libexec/include
              ${GLOBUS_PREFIX}/libexec/include
        DOC "The globus-ftp-control include directory"
    )

    set (GLOBUS_FTP_CONTROL_DEFINITIONS "")
endif (GLOBUS_FTP_CONTROL_PKG_FOUND)

if (GLOBUS_FTP_CONTROL_LIBRARIES)
    message (STATUS "GLOBUS_FTP_CONTROL libraries: ${GLOBUS_FTP_CONTROL_LIBRARIES}")
endif (GLOBUS_FTP_CONTROL_LIBRARIES)
if (GLOBUS_FTP_CONTROL_INCLUDE_DIRS)
    message (STATUS "GLOBUS_FTP_CONTROL include dir: ${GLOBUS_FTP_CONTROL_INCLUDE_DIRS}")
endif (GLOBUS_FTP_CONTROL_INCLUDE_DIRS)

# -----------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set GLOBUS_FTP_CONTROL_FOUND to TRUE if 
# all listed variables are TRUE
# -----------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args (GLOBUS_FTP_CONTROL DEFAULT_MSG
    GLOBUS_FTP_CONTROL_LIBRARIES  GLOBUS_FTP_CONTROL_INCLUDE_DIRS
)
mark_as_advanced(GLOBUS_FTP_CONTROL_INCLUDE_DIRS GLOBUS_FTP_CONTROL_LIBRARIES)
