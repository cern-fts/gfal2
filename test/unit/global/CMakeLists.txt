
add_executable(gfal2_test_exe "global_test.cpp" ${gfal2_utils_src})

target_link_libraries(gfal2_test_exe ${GFAL2_LINK} ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})

add_test(gfal2_test_exe gfal2_test_exe)
