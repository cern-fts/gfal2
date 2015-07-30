add_library(gfal2_test_shared SHARED gfal_lib_test.c gfal_gtest_asserts.cpp)
target_link_libraries (gfal2_test_shared ${GFAL2_LIBRARIES} ${GTEST_LIBRARIES})

if (FUNCTIONAL_TESTS)
    install (TARGETS gfal2_test_shared LIBRARY
        DESTINATION ${LIB_INSTALL_DIR})
endif ()

