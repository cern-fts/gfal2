#
# This module detects if SRM-IFCE is installed and determines where the
# include files and libraries are.
#
# This code sets the following variables:
# 
# SRM_IFCE_LIBRARIES   = full path to the SRM-IFCE libraries
# SRM_IFCE_INCLUDE_DIR = include dir to be used when using the SRM-IFCE library
# SRM_IFCE_FOUND       = set to true if SRM-IFCE was found successfully
#
# SRM_IFCE_LOCATION
#   setting this enables search for SRM-IFCE libraries / headers in this location

# ----------------------------------------------------- 
# Try with pkgconfig first
# -----------------------------------------------------

find_package(PkgConfig)
pkg_check_modules(SRM_IFCE_PKG REQUIRED srm-ifce>=1.15.0)

if (SRM_IFCE_PKG_FOUND)
    set (SRM_IFCE_LIBRARIES "${SRM_IFCE_PKG_LIBRARIES}")
    set (SRM_IFCE_CFLAGS "${SRM_IFCE_PKG_CFLAGS}")
    if (SRM_IFCE_PKG_INCLUDE_DIRS)
        set (SRM_IFCE_INCLUDE_DIR "${SRM_IFCE_PKG_INCLUDE_DIRS}")
    else ()
        set (SRM_IFCE_INCLUDE_DIR "/usr/include")
    endif ()
else ()
    # SRM-IFCE Libraries
    find_library(SRM_IFCE_LIBRARIES
        NAMES gfal_srm_ifce
        HINTS
            ${SRM_IFCE_LOCATION}/lib ${SRM_IFCE_LOCATION}/lib64 ${SRM_IFCE_LOCATION}/lib32
            ${STAGE_DIR}/lib ${STAGE_DIR}/lib64
            ${CMAKE_INSTALL_PREFIX}/Grid/srm-ifce/*/${PLATFORM}/lib
            ${CMAKE_INSTALL_PREFIX}/Grid/srm-ifce/*/${PLATFORM}/lib64
        DOC "The main srm-ifce library"
    )
    
    # SRM-IFCE Include Directories
    find_path(SRM_IFCE_INCLUDE_DIR 
        NAMES gfal_srm_ifce.h
        HINTS
            ${SRM_IFCE_LOCATION} ${SRM_IFCE_LOCATION}/include ${SRM_IFCE_LOCATION}/include/*
            ${STAGE_DIR}/include ${STAGE_DIR}/include
            ${CMAKE_INSTALL_PREFIX}/Grid/srm-ifce/*/${PLATFORM}/include
        DOC "The srm-ifce include directory"
    )
    set (SRM_IFCE_CFLAGS "")
endif()

if(SRM_IFCE_INCLUDE_DIR)
    message(STATUS "SRM-IFCE includes found in ${SRM_IFCE_INCLUDE_DIR}")
endif()

# -----------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set SRM_IFCE_FOUND to TRUE if 
# all listed variables are TRUE
# -----------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SRM_IFCE DEFAULT_MSG SRM_IFCE_LIBRARIES SRM_IFCE_INCLUDE_DIR)
mark_as_advanced(SRM_IFCE_LIBRARIES SRM_IFCE_INCLUDE_DIR SRM_IFCE_CFLAGS)
