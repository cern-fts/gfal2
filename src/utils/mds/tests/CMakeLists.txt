cmake_minimum_required (VERSION 2.6)


add_definitions(${gfal2_mds_def})
add_executable (test_gfal_mds "test_gfal_mds.c" ${src_mds})
target_link_libraries (test_gfal_mds gfal2 ${is_ifce_link} ${mds_cache_link})
