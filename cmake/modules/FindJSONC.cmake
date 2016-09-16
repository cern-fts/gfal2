# Find json-c

find_package (PkgConfig)
pkg_check_modules(JSONC_PKG json-c)

if (JSONC_PKG_FOUND)
    set (JSONC_LIBRARIES ${JSONC_PKG_LIBRARIES})
    set (JSONC_INCLUDE_DIRS ${JSONC_PKG_INCLUDE_DIRS})
else (JSONC_PKG)
    find_library(JSONC_LIBRARIES
        NAMES json-c json
        HINTS /lib /lib64 /usr/lib /usr/lib64
        DOC "json-c library"
    )

    find_path(JSONC_INCLUDE_DIRS
        NAMES json.h
        HINTS /usr/include/json /usr/include/json-c
        DOC "json-c headers"
    )
endif (JSONC_PKG_FOUND)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(json-c
    DEFAULT_MSG JSONC_LIBRARIES JSONC_INCLUDE_DIRS
)
mark_as_advanced(JSONC_INCLUDE_DIRS JSONC_LIBRARIES)
