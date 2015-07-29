cmake_minimum_required (VERSION 2.6)

add_executable (test_xrootd_sanitize sanitize.cpp)
target_link_libraries (test_xrootd_sanitize plugin_xrootd_static)

add_test(test_xrootd_sanitize test_xrootd_sanitize)
