
find_package (PugiXML)
if (PUGIXML_FOUND)
    add_executable(mds_test "test_mds.cpp")

    target_link_libraries(mds_test
        ${GFAL2_LIBRARIES} ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES}
    )

    add_test(mds_test mds_test)
endif (PUGIXML_FOUND)
