#
# unit test for transfer system



IF (UNIT_TESTS )

IF (PLUGIN_GRIDFTP)
	FILE(GLOB src_test_gridftp "*.c*")

	add_executable(unit_test_gridftp_plugin	${src_test_gridftp} )
	
	target_link_libraries(unit_test_gridftp_plugin ${OUTPUT_NAME_MAIN} ${CGREEN_LIBRARIES}  ${OUTPUT_NAME_TRANSFER} ${OUTPUT_NAME_GRIDFTP} m)
	
	add_test(test_gridftp_instance unit_test_gridftp_plugin load_gridftp)
	add_test(test_gridftp_handle unit_test_gridftp_plugin handle_creation)
	add_test(test_gridftp_URL unit_test_gridftp_plugin gridftp_parseURL)

	
ENDIF  (PLUGIN_GRIDFTP)

ENDIF (UNIT_TESTS)
