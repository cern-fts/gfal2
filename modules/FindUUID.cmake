#
# This module detects if uuid is installed and determines where the
# include files and libraries are.
#
# This code sets the following variables:
# 
# UUID_LIBRARIES       = full path to the uuid libraries
# UUID_INCLUDE_DIR     = include dir to be used when using the uuid library
# UUID_FOUND           = set to true if uuid was found successfully
#
# UUID_LOCATION
#   setting this enables search for uuid libraries / headers in this location

find_package (PkgConfig)
pkg_check_modules(UUID_PKG uuid)

if (UUID_PKG_FOUND)
    set (UUID_LIBRARIES ${UUID_PKG_LIBRARIES})
    set (UUID_INCLUDE_DIRS ${UUID_PKG_INCLUDE_DIRS})
    if (NOT UUID_INCLUDE_DIRS)
        set (UUID_INCLUDE_DIRS "/usr/include")
    endif (NOT UUID_INCLUDE_DIRS)
    set (UUID_DEFINITIONS "${UUID_PKG_CFLAGS} ${UUID_PKG_CFLAGS_OTHER}")
else (UUID_PKG_FOUND)

    find_library(UUID_LIBRARIES
        NAMES uuid
        HINTS ${UUID_LOCATION} 
              ${CMAKE_INSTALL_PREFIX}/uuid/*/${PLATFORM}/
        DOC "The uuid library"
    )

    find_path(UUID_INCLUDE_DIRS 
        NAMES uuid.h
        HINTS ${UUID_LOCATION}/include/*
              ${CMAKE_INSTALL_PREFIX}/uuid/*/${PLATFORM}/
        DOC "The uuid include directory"
    )

    set (UUID_DEFINITIONS "")
endif (UUID_PKG_FOUND)

if (UUID_LIBRARIES)
    message (STATUS "UUID libraries: ${UUID_LIBRARIES}")
endif (UUID_LIBRARIES)
if (UUID_INCLUDE_DIRS)
    message (STATUS "UUID include dir: ${UUID_INCLUDE_DIRS}")
endif (UUID_INCLUDE_DIRS)

# -----------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set UUID_FOUND to TRUE if 
# all listed variables are TRUE
# -----------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args (UUID DEFAULT_MSG
    UUID_LIBRARIES  UUID_INCLUDE_DIRS
)
mark_as_advanced(UUID_INCLUDE_DIRS UUID_LIBRARIES)
