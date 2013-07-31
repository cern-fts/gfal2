

SET(src_global "global_test.cpp")

add_executable(g_test_gfal2_text_exe ${src_global} ${gfal2_utils_src}
               )
target_link_libraries(g_test_gfal2_text_exe ${GFAL2_LINK} ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} ${GLIBMM_PKG_LIBRARIES})

add_test(g_test_gfal2_text g_test_gfal2_text_exe )



