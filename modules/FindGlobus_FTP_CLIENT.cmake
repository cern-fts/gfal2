#
# This module detects if globus-ftp-client is installed and determines where the
# include files and libraries are.
#
# This code sets the following variables:
# 
# GLOBUS_FTP_CLIENT_LIBRARIES       = full path to the globus-ftp-client libraries
# GLOBUS_FTP_CLIENT_INCLUDE_DIR     = include dir to be used when using the globus-ftp-client library
# GLOBUS_FTP_CLIENT_FOUND           = set to true if globus-ftp-client was found successfully
#
# GLOBUS_FTP_CLIENT_LOCATION
#   setting this enables search for globus-ftp-client libraries / headers in this location

find_package (PkgConfig)
pkg_check_modules(GLOBUS_FTP_CLIENT_PKG globus-ftp-client)

if (GLOBUS_FTP_CLIENT_PKG_FOUND)
    set (GLOBUS_FTP_CLIENT_LIBRARIES ${GLOBUS_FTP_CLIENT_PKG_LIBRARIES})
    set (GLOBUS_FTP_CLIENT_INCLUDE_DIRS ${GLOBUS_FTP_CLIENT_PKG_INCLUDE_DIRS})
    set (GLOBUS_FTP_CLIENT_DEFINITIONS "${GLOBUS_FTP_CLIENT_PKG_CFLAGS}")
else (GLOBUS_FTP_CLIENT_PKG_FOUND)

    set (CMAKE_FIND_FRAMEWORK NEVER)

    find_library(GLOBUS_FTP_CLIENT_LIBRARIES
        NAMES globus_ftp_client
        HINTS ${GLOBUS_FTP_CLIENT_LOCATION} 
              ${CMAKE_INSTALL_PREFIX}/globus/*/${PLATFORM}/
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/lib
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/lib64
              ${CMAKE_INSTALL_PREFIX}/opt/globus-toolkit/libexec/lib
              ${GLOBUS_PREFIX}/libexec/lib
        DOC "The main globus-ftp-client library"
    )

    find_path(GLOBUS_FTP_CLIENT_INCLUDE_DIRS 
        NAMES globus_ftp_client.h
        HINTS ${GLOBUS_FTP_CLIENT_LOCATION}/include/*
              ${CMAKE_INSTALL_PREFIX}/globus/*/${PLATFORM}/include
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/include
              ${CMAKE_INSTALL_PREFIX}/opt/globus-toolkit/libexec/include
              ${GLOBUS_PREFIX}/libexec/include
        DOC "The globus-ftp-client include directory"
    )

    set (GLOBUS_FTP_CLIENT_DEFINITIONS "")
endif (GLOBUS_FTP_CLIENT_PKG_FOUND)

if (GLOBUS_FTP_CLIENT_LIBRARIES)
    message (STATUS "GLOBUS_FTP_CLIENT libraries: ${GLOBUS_FTP_CLIENT_LIBRARIES}")
endif (GLOBUS_FTP_CLIENT_LIBRARIES)
if (GLOBUS_FTP_CLIENT_INCLUDE_DIRS)
    message (STATUS "GLOBUS_FTP_CLIENT include dir: ${GLOBUS_FTP_CLIENT_INCLUDE_DIRS}")
endif (GLOBUS_FTP_CLIENT_INCLUDE_DIRS)

# -----------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set GLOBUS_FTP_CLIENT_FOUND to TRUE if 
# all listed variables are TRUE
# -----------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args (GLOBUS_FTP_CLIENT DEFAULT_MSG
    GLOBUS_FTP_CLIENT_LIBRARIES  GLOBUS_FTP_CLIENT_INCLUDE_DIRS
)
mark_as_advanced(GLOBUS_FTP_CLIENT_INCLUDE_DIRS GLOBUS_FTP_CLIENT_LIBRARIES)
