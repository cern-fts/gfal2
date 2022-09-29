find_package (PugiXML)

include_directories(
    "${CMAKE_SOURCE_DIR}/test"
    "${CMAKE_SOURCE_DIR}/src/posix/"
)

add_subdirectory(cancel)
add_subdirectory(config)
add_subdirectory(cred)
add_subdirectory(global)
add_subdirectory(http)
add_subdirectory(mds)
add_subdirectory(transfer)
add_subdirectory(uri)

if (PUGIXML_FOUND)
set (TEST_MDS ./mds/test_mds.cpp)
else (PUGIXML_FOUND)
set (TEST_MDS "")
endif (PUGIXML_FOUND)

if (PLUGIN_HTTP)
    set(TEST_TOKEN_MAP ./http/test_token_map.cpp)
    set(TEST_CUSTOM_HTTP_OPTIONS http/test_custom_http_options.cpp)
    set(HTTP_PLUGIN_LIBRARIES  plugin_http_static)

    include     (CheckLibraryExists)
    find_package(Davix REQUIRED)

    # Includes
    include_directories(${DAVIX_INCLUDE_DIR})

else(PLUGIN_HTTP)
    set(TEST_TOKEN_MAP "")
    set(TEST_CUSTOM_HTTP_OPTIONS "")
    set(HTTP_PLUGIN_LIBRARIES "")
endif (PLUGIN_HTTP)

add_executable(gfal2-unit-tests
    ./cancel/cancel_tests.cpp
    ./config/config_test.cpp
    ./cred/test_cred.cpp
    ./global/global_test.cpp
    ${TEST_TOKEN_MAP}
    ${TEST_CUSTOM_HTTP_OPTIONS}
    ${TEST_MDS}
    ./transfer/tests_callbacks.cpp
    ./transfer/tests_params.cpp
    ./uri/test_uri.cpp
    ./uri/test_parsing.cpp
)

target_include_directories(gfal2-unit-tests PRIVATE
    ${CMAKE_SOURCE_DIR}/test
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(gfal2-unit-tests
    ${GFAL2_LIBRARIES} ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} gfal2_test_shared ${HTTP_PLUGIN_LIBRARIES}
)

install(TARGETS gfal2-unit-tests
  DESTINATION ${BIN_INSTALL_DIR}/)

add_test(unit-tests gfal2-unit-tests)
