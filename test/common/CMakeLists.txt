if(FUNCTIONAL_TESTS)
    find_package(GTEST)
    include_directories(${GTEST_INCLUDE_DIR})

    add_library(test_shared SHARED gfal_lib_test.c gfal_gtest_asserts.cpp)
    target_link_libraries (test_shared ${GFAL2_TRANSFER_LINK} ${GTEST_LIBRARIES})
endif (FUNCTIONAL_TESTS)
