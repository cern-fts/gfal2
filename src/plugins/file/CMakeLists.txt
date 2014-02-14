cmake_minimum_required (VERSION 2.6)


if (PLUGIN_FILE)
    file (GLOB src_file "*.c*")

    pkg_check_modules(ZLIB_PKG REQUIRED zlib)


    include_directories(${ZLIB_PKG_INCLUDE_DIRS})


    add_library (plugin_file SHARED ${src_file} ${gfal2_src_checksum})
    target_link_libraries (plugin_file gfal2 ${ZLIB_PKG_LIBRARIES})


    set_target_properties(plugin_file   PROPERTIES
                                LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/src
								CLEAN_DIRECT_OUTPUT 1
								OUTPUT_NAME "gfal_plugin_file"
								LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

    install(TARGETS		plugin_file
	        LIBRARY		DESTINATION ${PLUGIN_INSTALL_DIR} )
	    
    install(FILES		"README_PLUGIN_FILE"
	    	DESTINATION ${DOC_INSTALL_DIR})	    

endif (PLUGIN_FILE)

