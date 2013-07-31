cmake_minimum_required (VERSION 2.6)

add_definitions (-fPIC)

if (IS_IFCE)
    LIST(APPEND gfal2_mds_def "-DMDS_BDII_EXTERNAL=1")
    set (is_ifce_link "is_ifce")
    LIST(APPEND src_mds "${CMAKE_CURRENT_SOURCE_DIR}/gfal_mds.c")
else (IS_IFCE)
    LIST(APPEND gfal2_mds_def "-DMDS_BDII_EXTERNAL=0")
    LIST(APPEND is_ifce_link "ldap_r" "lber")
    LIST(APPEND src_mds  "${CMAKE_CURRENT_SOURCE_DIR}/gfal_mds.c" "${CMAKE_CURRENT_SOURCE_DIR}/gfal_mds_internal.c" "${CMAKE_CURRENT_SOURCE_DIR}/gfal_mds_ldap_internal_layer.c")
endif (IS_IFCE)



find_package (PugiXML)
if (NOT PUGIXML_FOUND)
    message ("PugiXML not found. MDS Cache disabled")
    LIST(APPEND gfal2_mds_def "-DMDS_WITHOUT_CACHE")
    set(mds_cache_link "")
else (NOT PUGIXML_FOUND)
    LIST(APPEND src_mds "${CMAKE_CURRENT_SOURCE_DIR}/gfal_mds_cache.cpp")
    set(mds_cache_link "${PUGIXML_LIBRARIES}")
    LIST(APPEND gfal2_mds_def "")
endif (NOT PUGIXML_FOUND)



set(gfal2_mds_src ${src_mds} PARENT_SCOPE)
set(mds_cache_link ${mds_cache_link} PARENT_SCOPE)
set(is_ifce_link  ${is_ifce_link} PARENT_SCOPE )
set(gfal2_mds_def ${gfal2_mds_def} PARENT_SCOPE)

add_subdirectory (tests)
