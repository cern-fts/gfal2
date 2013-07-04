

SET(src_global "global_test.cpp")

add_executable(g_test_gfal2_text_exe ${src_global}
               )
target_link_libraries(g_test_gfal2_text_exe ${GFAL2_LINK} gtest gtest_main gfal2_utils) 

add_test(g_test_gfal2_text g_test_gfal2_text_exe )



