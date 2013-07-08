cmake_minimum_required (VERSION 2.6)

add_executable (test_gfal_mds test_gfal_mds.c)
target_link_libraries (test_gfal_mds gfal2 gfal2_mds)
