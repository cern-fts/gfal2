
add_executable(g_test_uri_exe "test_uri.cpp")

target_link_libraries(g_test_uri_exe ${GFAL2_LINK} ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} ${GLIBMM_PKG_LIBRARIES})

add_test(g_test_uri g_test_uri_exe )
