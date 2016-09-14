# - Try to find XROOTD libraries
#
#  XROOTD_FOUND - System has XROOTD
#  XROOTD_INCLUDE_DIR - The XROOTD include directory
#  XROOTD_LIBRARIES - The libraries needed to use XROOTD
#
# XROOTD_LOCATION
#   setting this enables search for xrootd libraries / headers in this location


# -----------------------------------------------------
# XROOTD Libraries
# -----------------------------------------------------
find_library(XROOTD_CL
    NAMES XrdCl
    HINTS ${XROOTD_LOCATION}/lib ${XROOTD_LOCATION}/lib64 ${XROOTD_LOCATION}/lib32
    DOC "xrootd cl"
)
find_library(XROOTD_CLIENT
    NAMES XrdClient
    HINTS ${XROOTD_LOCATION}/lib ${XROOTD_LOCATION}/lib64 ${XROOTD_LOCATION}/lib32
    DOC "xrootd client"
)
find_library(XROOTD_POSIX
    NAMES XrdPosix
    HINTS ${XROOTD_LOCATION}/lib ${XROOTD_LOCATION}/lib64 ${XROOTD_LOCATION}/lib32
    DOC "xrootd posix libraries"
)
find_library(XROOTD_UTIL
    NAMES XrdUtils
    HINTS ${XROOTD_LOCATION}/lib ${XROOTD_LOCATION}/lib64 ${XROOTD_LOCATION}/lib32
    DOC "xrootd util"
)

set(XROOTD_LIBRARIES
    ${XROOTD_CL}
    ${XROOTD_CLIENT}
    ${XROOTD_POSIX}
    ${XROOTD_UTIL}
)
if(XROOTD_LIBRARIES)
    message(STATUS "xrootd library found in ${XROOTD_LIBRARIES}")
endif()

# -----------------------------------------------------
# XROOTD Include Directories
# -----------------------------------------------------
find_path(XROOTD_INCLUDE_DIR
    NAMES XrdVersion.hh
    HINTS ${XROOTD_LOCATION} ${XROOTD_LOCATION}/include ${XROOTD_LOCATION}/include/* 
          ${XROOTD_LOCATION}/src/
          /usr/include/xrootd ${CMAKE_INSTALL_PREFIX}/include/xrootd
    DOC "The xrootd include directory"
)
if(XROOTD_INCLUDE_DIR)
    message(STATUS "xrootd includes found in ${XROOTD_INCLUDE_DIR}")
endif()

# -----------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set XROOTD_FOUND to TRUE if
# all listed variables are TRUE
# -----------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(xrootd DEFAULT_MSG XROOTD_LIBRARIES XROOTD_INCLUDE_DIR)
mark_as_advanced(XROOTD_INCLUDE_DIR XROOTD_LIBRARIES)
