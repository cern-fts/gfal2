cmake_minimum_required (VERSION 2.6)


if (PLUGIN_MOCK)
    file (GLOB src_file "*.c*")

    add_library (plugin_mock SHARED ${src_file})
    target_link_libraries (plugin_mock gfal2 gfal2_transfer)


    set_target_properties(plugin_mock   PROPERTIES
                                LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/src
								CLEAN_DIRECT_OUTPUT 1
								OUTPUT_NAME "gfal_plugin_mock"
								LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

    install(TARGETS		plugin_mock
	        LIBRARY		DESTINATION ${PLUGIN_INSTALL_DIR} )
	    
    install(FILES		"README_PLUGIN_MOCK"
                DESTINATION ${DOC_INSTALL_DIR})

    # install mock configuration files
    LIST(APPEND mock_conf_file "${CMAKE_SOURCE_DIR}/dist/etc/gfal2.d/mock_plugin.conf")
    install(FILES ${mock_conf_file}
                        DESTINATION ${SYSCONF_INSTALL_DIR}/gfal2.d/)

endif (PLUGIN_MOCK)

