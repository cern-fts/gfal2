
add_executable(config_test "config_test.cpp")

target_link_libraries(config_test
    ${GFAL2_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    gfal2_test_shared
)

add_test(config_test config_test)
