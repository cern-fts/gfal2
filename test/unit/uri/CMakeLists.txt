
add_executable(g_test_uri_exe "test_uri.cpp")

target_link_libraries(g_test_uri_exe
    ${GFAL2_LIBRARIES} ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES}
)

add_executable(g_test_parsing_exe "test_parsing.cpp")

target_link_libraries(g_test_parsing_exe
    ${GFAL2_LIBRARIES} ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES}
)

add_test(g_test_uri g_test_uri_exe )
add_test(g_test_parsing g_test_parsing_exe)
