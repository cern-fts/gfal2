file (GLOB src_test_cancel "*.c*")

add_executable(unit_test_transfer_cancel_exe
    ${src_test_cancel}
)

target_link_libraries(unit_test_transfer_cancel_exe
    ${GFAL2_LIBRARIES} ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} m
)

add_test(unit_test_transfer_cancel unit_test_transfer_cancel_exe)
