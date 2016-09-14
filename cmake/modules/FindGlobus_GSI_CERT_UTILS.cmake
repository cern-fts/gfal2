#
# This module detects if globus-gsi-cert-utils is installed and determines where the
# include files and libraries are.
#
# This code sets the following variables:
# 
# GLOBUS_GSI_CERT_UTILS_LIBRARIES       = full path to the globus-gsi-cert-utils libraries
# GLOBUS_GSI_CERT_UTILS_INCLUDE_DIR     = include dir to be used when using the globus-gsi-cert-utils library
# GLOBUS_GSI_CERT_UTILS_FOUND           = set to true if globus-gsi-cert-utils was found successfully
#
# GLOBUS_GSI_CERT_UTILS_LOCATION
#   setting this enables search for globus-gsi-cert-utils libraries / headers in this location

find_package (PkgConfig)
pkg_check_modules(GLOBUS_GSI_CERT_UTILS_PKG globus-gsi-cert-utils)

if (GLOBUS_GSI_CERT_UTILS_PKG_FOUND)
    set (GLOBUS_GSI_CERT_UTILS_LIBRARIES ${GLOBUS_GSI_CERT_UTILS_PKG_LIBRARIES})
    set (GLOBUS_GSI_CERT_UTILS_INCLUDE_DIRS ${GLOBUS_GSI_CERT_UTILS_PKG_INCLUDE_DIRS})
    set (GLOBUS_GSI_CERT_UTILS_DEFINITIONS "${GLOBUS_GSI_CERT_UTILS_PKG_CFLAGS}")
else (GLOBUS_GSI_CERT_UTILS_PKG_FOUND)

    set (CMAKE_FIND_FRAMEWORK NEVER)

    find_library(GLOBUS_GSI_CERT_UTILS_LIBRARIES
        NAMES globus_gsi_cert_utils
        HINTS ${GLOBUS_GSI_CERT_UTILS_LOCATION} 
              ${CMAKE_INSTALL_PREFIX}/globus/*/${PLATFORM}/
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/lib
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/lib64
              ${CMAKE_INSTALL_PREFIX}/opt/globus-toolkit/libexec/lib
              ${GLOBUS_PREFIX}/libexec/lib
        DOC "The main globus-gsi-cert-utils library"
    )

    find_path(GLOBUS_GSI_CERT_UTILS_INCLUDE_DIRS 
        NAMES globus_gsi_cert_utils.h
        HINTS ${GLOBUS_GSI_CERT_UTILS_LOCATION}/include/*
              ${CMAKE_INSTALL_PREFIX}/globus/*/${PLATFORM}/include
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/include
              ${CMAKE_INSTALL_PREFIX}/opt/globus-toolkit/libexec/include
              ${GLOBUS_PREFIX}/libexec/include
        DOC "The globus-gsi-cert-utils include directory"
    )

    set (GLOBUS_GSI_CERT_UTILS_DEFINITIONS "")
endif (GLOBUS_GSI_CERT_UTILS_PKG_FOUND)

if (GLOBUS_GSI_CERT_UTILS_LIBRARIES)
    message (STATUS "GLOBUS_GSI_CERT_UTILS libraries: ${GLOBUS_GSI_CERT_UTILS_LIBRARIES}")
endif (GLOBUS_GSI_CERT_UTILS_LIBRARIES)
if (GLOBUS_GSI_CERT_UTILS_INCLUDE_DIRS)
    message (STATUS "GLOBUS_GSI_CERT_UTILS include dir: ${GLOBUS_GSI_CERT_UTILS_INCLUDE_DIRS}")
endif (GLOBUS_GSI_CERT_UTILS_INCLUDE_DIRS)

# -----------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set GLOBUS_GSI_CERT_UTILS_FOUND to TRUE if 
# all listed variables are TRUE
# -----------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args (GLOBUS_GSI_CERT_UTILS DEFAULT_MSG
    GLOBUS_GSI_CERT_UTILS_LIBRARIES  GLOBUS_GSI_CERT_UTILS_INCLUDE_DIRS
)
mark_as_advanced(GLOBUS_GSI_CERT_UTILS_INCLUDE_DIRS GLOBUS_GSI_CERT_UTILS_LIBRARIES)
