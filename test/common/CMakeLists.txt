add_library(test_shared SHARED gfal_lib_test.c gfal_gtest_asserts.cpp)
target_link_libraries (test_shared ${GFAL2_LIBRARIES} ${GTEST_LIBRARIES})
