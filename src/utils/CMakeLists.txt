cmake_minimum_required (VERSION 2.6)

# Dependencies
if (IS_IFCE)
    list (APPEND gfal2_utils_definitions "-DMDS_BDII_EXTERNAL=1")
    set (is_ifce_link "is_ifce")
    list (APPEND src_mds "${CMAKE_CURRENT_SOURCE_DIR}/mds/gfal_mds.c")
else (IS_IFCE)
    list (APPEND gfal2_utils_definitions "-DMDS_BDII_EXTERNAL=0")
    list (APPEND is_ifce_link "ldap_r" "lber")
    list (APPEND src_mds
        "${CMAKE_CURRENT_SOURCE_DIR}/mds/gfal_mds.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/mds/gfal_mds_internal.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/mds/gfal_mds_ldap_internal_layer.c"
    )
endif (IS_IFCE)

find_package (PugiXML)
if (NOT PUGIXML_FOUND)
    message ("PugiXML not found. MDS Cache disabled")
    list (APPEND gfal2_utils_definitions "-DMDS_WITHOUT_CACHE=1")
    set (mds_cache_link "")
else (NOT PUGIXML_FOUND)
    list (APPEND src_mds
        "${CMAKE_CURRENT_SOURCE_DIR}/mds/gfal_mds_cache.cpp"
    )
    set (mds_cache_link "${PUGIXML_LIBRARIES}")
endif (NOT PUGIXML_FOUND)

# Link
list (APPEND gfal2_utils_libraries
    ${is_ifce_link}
    ${mds_cache_link}
)

# Sources
file (GLOB src_exceptions   "${CMAKE_CURRENT_SOURCE_DIR}/exceptions/*.c*")
file (GLOB src_gsimplecache "${CMAKE_CURRENT_SOURCE_DIR}/gsimplecache/*.c*")
file (GLOB src_uri          "${CMAKE_CURRENT_SOURCE_DIR}/uri/*.c*")
file (GLOB src_checksums    "${CMAKE_CURRENT_SOURCE_DIR}/checksums/*.c*")

list (APPEND gfal2_utils_src ${src_exceptions})

list (APPEND gfal2_utils_c_src
    ${src_gsimplecache}
    ${src_uri}
    ${src_checksums}
    ${src_mds}
)

set (gfal2_utils_c_src ${gfal2_utils_c_src} PARENT_SCOPE)
set (gfal2_utils_src ${gfal2_utils_src} PARENT_SCOPE)
set (gfal2_utils_libraries ${gfal2_utils_libraries} PARENT_SCOPE)
set (gfal2_utils_definitions ${gfal2_utils_definitions} PARENT_SCOPE)

# Install public headers
install (FILES "uri/gfal2_uri.h"
         DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/utils)
