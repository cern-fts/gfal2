include_directories(
    "${CMAKE_SOURCE_DIR}/test"
    "${CMAKE_SOURCE_DIR}/src/posix/"
)

add_subdirectory(cancel)
add_subdirectory(config)
add_subdirectory(cred)
add_subdirectory(global)
add_subdirectory(mds)
add_subdirectory(transfer)
add_subdirectory(uri)


add_executable(gfal2-unit-tests

    ./cancel/cancel_tests.cpp
    ./config/config_test.cpp
    ./cred/test_cred.cpp
    ./global/global_test.cpp
    ./mds/test_mds.cpp
    ./transfer/tests_callbacks.cpp
    ./transfer/tests_params.cpp
    ./uri/test_uri.cpp
)

target_include_directories(gfal2-unit-tests PRIVATE
    ${CMAKE_SOURCE_DIR}/test
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)


target_link_libraries(gfal2-unit-tests
    ${GFAL2_LIBRARIES} ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} gfal2_test_shared
)

install(TARGETS gfal2-unit-tests
  DESTINATION ${BIN_INSTALL_DIR}/)

add_test(unit-tests gfal2-unit-tests)