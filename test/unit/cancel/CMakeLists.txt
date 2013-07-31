#
# unti test for transfer system



IF (UNIT_TESTS )



        include_directories( "../ " "../libcpp/" ${GLIB2_PKG_INCLUDE_DIRS}
                                                ${GTHREAD2_PKG_INCLUDE_DIRS}  )
        FILE(GLOB src_test_cancel "*.c*")

        add_executable(unit_test_transfer_cancel_exe	${src_test_cancel} )
	
        target_link_libraries(unit_test_transfer_cancel_exe ${GFAL2_LINK} ${GFAL2_TRANSFER_LINK} m
                          ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
	
        add_test(unit_test_transfer_cancel unit_test_transfer_cancel_exe)
	

ENDIF (UNIT_TESTS)
