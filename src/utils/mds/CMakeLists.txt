cmake_minimum_required (VERSION 2.6)

add_definitions (-fPIC)

if (IS_IFCE)
    add_definitions (-DMDS_BDII_EXTERNAL=1)
    set (is_ifce_link "is_ifce")
    set (src_mds "gfal_mds.c")
else (IS_IFCE)
    add_definitions (-DMDS_BDII_EXTERNAL=0)
    set (is_ifce_link "ldap_r;lber")
    set (src_mds "gfal_mds.c" "gfal_mds_internal.c" "gfal_mds_ldap_internal_layer.c")
endif (IS_IFCE)


find_package (PugiXML)
if (NOT PUGIXML_FOUND)
    message ("PugiXML not found. MDS Cache disabled")
    add_definitions (-DMDS_WITHOUT_CACHE)
else (NOT PUGIXML_FOUND)
    list (APPEND src_mds "gfal_mds_cache.cpp")
endif (NOT PUGIXML_FOUND)

add_library (gfal2_mds STATIC ${src_mds})

target_link_libraries (gfal2_mds ${is_ifce_link})

if (PUGIXML_FOUND)
    target_link_libraries (gfal2_mds ${PUGIXML_LIBRARIES})
endif (PUGIXML_FOUND)

add_subdirectory (tests)
