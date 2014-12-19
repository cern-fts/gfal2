#
# unti test for transfer system



IF (UNIT_TESTS )

IF (MAIN_TRANSFER)

    include_directories( "../ " "../libcpp/" ${GLIB2_PKG_INCLUDE_DIRS}
                                                ${GTHREAD2_PKG_INCLUDE_DIRS})
    FILE(GLOB src_test_transfer "*.c*")

    add_executable(unit_test_transfer_params_exe    ${src_test_transfer} )
  
    target_link_libraries(unit_test_transfer_params_exe ${GFAL2_LINK} ${GFAL2_TRANSFER_LINK} m
                          ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
    
    add_test(unit_test_transfer_params unit_test_transfer_params_exe)
    
ENDIF  (MAIN_TRANSFER)

ENDIF (UNIT_TESTS)
