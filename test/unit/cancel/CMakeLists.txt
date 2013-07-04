#
# unti test for transfer system



IF (UNIT_TESTS )



        include_directories( "../ " "../libcpp/" ${GLIB2_PKG_INCLUDE_DIRS}
                                                ${GTHREAD2_PKG_INCLUDE_DIRS}  )
        FILE(GLOB src_test_cancel "*.c*")

        add_executable(unit_test_transfer_cancel_exe	${src_test_cancel} )
	
        target_link_libraries(unit_test_transfer_cancel_exe ${OUTPUT_NAME_MAIN} ${OUTPUT_NAME_TRANSFER} m
                                        gtest gtest_main)
	
        add_test(unit_test_transfer_cancel unit_test_transfer_cancel_exe)
	

ENDIF (UNIT_TESTS)
