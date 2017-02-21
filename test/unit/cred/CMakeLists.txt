
add_executable(gfal2_cred_test "test_cred.cpp")

target_link_libraries(gfal2_cred_test
    ${GFAL2_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    gfal2_test_shared
)

add_test(gfal2_cred_test gfal2_cred_test)
