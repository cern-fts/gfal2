#
# unti test for transfer system

link_directories (${CMAKE_BINARY_DIR}/plugins) 
link_directories (${CMAKE_BINARY_DIR}/src) 

IF (UNIT_TESTS )

IF (PLUGIN_SRM)
	FILE(GLOB src_test_srm "*.c*")

	link_directories (${PROJECT_BINARY_DIR}/src) 
        add_executable(unit_test_srm_plugin ${src_test_srm})
	
        target_link_libraries(unit_test_srm_plugin ${GFAL2_LINK} ${GFAL2_TRANSFER_LINK}
                    ${SRM_IFCE_PKG_LIBRARIES} plugin_srm m
                    ${GFAL2_LINK} gtest gtest_main)
	add_dependencies(unit_test_srm_plugin plugin_srm)	
	add_test(test_srm_check_url unit_test_srm_plugin srm_url_check)


	
ENDIF  (PLUGIN_SRM)

ENDIF (UNIT_TESTS)
