add_executable(gfal2_token_map_test "test_token_map.cpp")
add_executable(gfal2_streaming_option_test "test_streaming_option.cpp")

find_package(Davix REQUIRED)
find_package(JSONC REQUIRED)

file(GLOB src_http "${CMAKE_SOURCE_DIR}/src/plugins/http/gfal_http_*.cpp")
add_library(test_plugin_http STATIC ${src_http})

target_link_libraries(test_plugin_http
  gfal2
  ${DAVIX_LIBRARIES}
  ${JSONC_LIBRARIES})

set(test_plugin_http_link_libraries
  ${GFAL2_LIBRARIES}
  ${GTEST_LIBRARIES}
  ${GTEST_MAIN_LIBRARIES}
  gfal2_test_shared
  ${DAVIX_LIBRARIES}
  test_plugin_http)

target_include_directories(test_plugin_http PRIVATE
  ${DAVIX_INCLUDE_DIR}
  ${JSONC_INCLUDE_DIRS})

target_link_libraries(gfal2_token_map_test
  ${test_plugin_http_link_libraries})

target_include_directories(gfal2_token_map_test PRIVATE
  ${DAVIX_INCLUDE_DIR})

target_link_libraries(gfal2_streaming_option_test
  ${test_plugin_http_link_libraries})

target_include_directories(gfal2_streaming_option_test PRIVATE
  ${DAVIX_INCLUDE_DIR})

add_test(gfal2_token_map_test gfal2_token_map_test)
add_test(gfal2_streaming_option_test gfal2_streaming_option_test)
