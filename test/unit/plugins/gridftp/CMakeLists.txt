link_directories (${CMAKE_BINARY_DIR}/plugins)
link_directories (${CMAKE_BINARY_DIR}/src)
    
    
if (PLUGIN_GRIDFTP)
    file (GLOB src_test_gridftp "*.cpp")
    
    add_executable(unit_test_gridftp_plugin_bin ${src_test_gridftp} )
    
    target_link_libraries(unit_test_gridftp_plugin_bin
        pthread gfal2 plugin_gridftp
        ${GTEST_LIBRARIES}
    )
        
    add_dependencies(unit_test_gridftp_plugin_bin plugin_gridftp)
    add_test(unit_test_gridftp_plugin unit_test_gridftp_plugin_bin )
	
endif (PLUGIN_GRIDFTP)
