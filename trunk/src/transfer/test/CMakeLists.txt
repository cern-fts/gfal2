#
# unti test for transfer system



IF (UNIT_TESTS )

IF (MAIN_TRANSFER)
	FILE(GLOB src_test_transfer "*.c*")

	add_executable(unit_test_transfer_params	${src_test_transfer} )
	
	target_link_libraries(unit_test_transfer_params ${OUTPUT_NAME_MAIN} ${CGREEN_LIBRARIES}  gfal2_transfer m)
	
	add_test(test_transfer_params unit_test_transfer_params create_params)
	add_test(test_transfer_params2 unit_test_transfer_params test_timeout)
	add_test(test_transfer_nbstream unit_test_transfer_params test_nbstreams)
	add_test(test_transfer_nbstream_c unit_test_transfer_params test_nbstreams_c)
	add_test(test_transfer_lock unit_test_transfer_params test_lock)
	add_test(test_transfer_params2_c unit_test_transfer_params test_timeout_c)
	add_test(test_transfer_filecopy_instance unit_test_transfer_params test_filecopy_instance)
	add_test(test_transfer_filecopy_instance_c unit_test_transfer_params test_filecopy_instance_c)
	add_test(test_transfer_filecopy_instance_prob unit_test_transfer_params test_filecopy_instance_c_problem)
	
ENDIF  (MAIN_TRANSFER)

ENDIF (UNIT_TESTS)
