cmake_minimum_required (VERSION 2.6)

add_definitions (-fPIC)

if (IS_IFCE)
    add_definitions (-DMDS_BDII_EXTERNAL=1)
    set (is_ifce_link "is_ifce")
    set (src_mds "gfal_mds.c")
else (IS_IFCE)
    add_definitions (-DMDS_BDII_EXTERNAL=0)
    set (is_ifce_link "ldap_r")
    file (GLOB src_mds "*.c*")
endif (IS_IFCE)


add_library (gfal2_mds STATIC ${src_mds})

target_link_libraries (gfal2_mds ${is_ifce_link})

add_subdirectory (tests)
