if (MAIN_TRANSFER)

    add_executable (unit_test_transfer_params_exe
        tests_params.cpp
    )
    target_link_libraries(unit_test_transfer_params_exe
        ${GFAL2_LIBRARIES} ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} m
    )

    add_executable (unit_test_transfer_callbacks_exe
        tests_callbacks.cpp
    )
    target_link_libraries(unit_test_transfer_callbacks_exe
        ${GFAL2_LIBRARIES} ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} m
    )

    add_test(unit_test_transfer_params unit_test_transfer_params_exe)

    add_test(unit_test_transfer_callbacks unit_test_transfer_callbacks_exe)

endif  (MAIN_TRANSFER)
