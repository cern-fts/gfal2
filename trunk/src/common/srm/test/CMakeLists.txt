#
# unti test for transfer system

link_directories (${CMAKE_BINARY_DIR}/plugins) 
link_directories (${CMAKE_BINARY_DIR}/src) 

IF (UNIT_TESTS )

IF (PLUGIN_SRM)
	FILE(GLOB src_test_srm "*.c*")

	link_directories (${PROJECT_BINARY_DIR}/src) 
	add_executable(unit_test_srm_plugin	${src_test_srm} )
	
        target_link_libraries(unit_test_srm_plugin ${OUTPUT_NAME_MAIN} ${CGREEN_LIBRARIES}  ${SRM_IFCE_PKG_LIBRARIES} ${OUTPUT_NAME_TRANSFER} ${OUTPUT_NAME_SRM} m)
	
	add_test(test_srm_check_url unit_test_srm_plugin srm_url_check)


	
ENDIF  (PLUGIN_SRM)

ENDIF (UNIT_TESTS)
