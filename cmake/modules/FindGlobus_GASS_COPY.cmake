#
# This module detects if globus-gass-copy is installed and determines where the
# include files and libraries are.
#
# This code sets the following variables:
# 
# GLOBUS_GASS_COPY_LIBRARIES       = full path to the globus-gass-copy libraries
# GLOBUS_GASS_COPY_INCLUDE_DIR     = include dir to be used when using the globus-gass-copy library
# GLOBUS_GASS_COPY_FOUND           = set to true if globus-gass-copy was found successfully
#
# GLOBUS_GASS_COPY_LOCATION
#   setting this enables search for globus-gass-copy libraries / headers in this location

find_package (PkgConfig)
pkg_check_modules(GLOBUS_GASS_COPY_PKG globus-gass-copy)

if (GLOBUS_GASS_COPY_PKG_FOUND)
    set (GLOBUS_GASS_COPY_LIBRARIES ${GLOBUS_GASS_COPY_PKG_LIBRARIES})
    set (GLOBUS_GASS_COPY_INCLUDE_DIRS ${GLOBUS_GASS_COPY_PKG_INCLUDE_DIRS})
    set (GLOBUS_GASS_COPY_DEFINITIONS "${GLOBUS_GASS_COPY_PKG_CFLAGS}")
else (GLOBUS_GASS_COPY_PKG_FOUND)

    find_library(GLOBUS_GASS_COPY_LIBRARIES
        NAMES globus_gass_copy
        HINTS ${GLOBUS_GASS_COPY_LOCATION} 
              ${CMAKE_INSTALL_PREFIX}/globus/*/${PLATFORM}/
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/lib
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/lib64
              ${CMAKE_INSTALL_PREFIX}/opt/globus-toolkit/libexec/lib
              ${GLOBUS_PREFIX}/libexec/lib
        DOC "The main globus-gass-copy library"
    )

    find_path(GLOBUS_GASS_COPY_INCLUDE_DIRS 
        NAMES globus_gass_copy.h
        HINTS ${GLOBUS_GASS_COPY_LOCATION}/include/*
              ${CMAKE_INSTALL_PREFIX}/globus/*/${PLATFORM}/
              ${CMAKE_INSTALL_PREFIX}/Grid/epel/*/${PLATFORM}/
              ${CMAKE_INSTALL_PREFIX}/opt/globus-toolkit/libexec/include
              ${GLOBUS_PREFIX}/libexec/include
        DOC "The globus-gass-copy include directory"
    )

    set (GLOBUS_GASS_COPY_DEFINITIONS "")
endif (GLOBUS_GASS_COPY_PKG_FOUND)

if (GLOBUS_GASS_COPY_LIBRARIES)
    message (STATUS "GLOBUS_GSSAPI_GSI libraries: ${GLOBUS_GASS_COPY_LIBRARIES}")
endif (GLOBUS_GASS_COPY_LIBRARIES)
if (GLOBUS_GASS_COPY_INCLUDE_DIRS)
    message (STATUS "GLOBUS_GSSAPI_GSI include dir: ${GLOBUS_GASS_COPY_INCLUDE_DIRS}")
endif (GLOBUS_GASS_COPY_INCLUDE_DIRS)

# -----------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set GLOBUS_GASS_COPY_FOUND to TRUE if 
# all listed variables are TRUE
# -----------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args (GLOBUS_GSSAPI_GSI DEFAULT_MSG
    GLOBUS_GASS_COPY_LIBRARIES  GLOBUS_GASS_COPY_INCLUDE_DIRS
)
mark_as_advanced(GLOBUS_GASS_COPY_INCLUDE_DIRS GLOBUS_GASS_COPY_LIBRARIES)
