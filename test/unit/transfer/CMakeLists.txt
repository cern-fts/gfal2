if (MAIN_TRANSFER)

    file (GLOB src_test_transfer "*.c*")

    add_executable (unit_test_transfer_params_exe
        ${src_test_transfer}
    )
  
    target_link_libraries(unit_test_transfer_params_exe
        ${GFAL2_LIBRARIES} ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} m
    )
    
    add_test(unit_test_transfer_params unit_test_transfer_params_exe)
    
endif  (MAIN_TRANSFER)
