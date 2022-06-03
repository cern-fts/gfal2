add_executable(gfal2_test_uri "test_uri.cpp")

target_link_libraries(gfal2_test_uri
    ${GFAL2_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
)

add_executable(gfal2_test_parsing "test_parsing.cpp")

target_link_libraries(gfal2_test_parsing
    ${GFAL2_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
)

add_test(gfal2_test_uri gfal2_test_uri)
add_test(gfal2_test_parsing gfal2_test_parsing)
