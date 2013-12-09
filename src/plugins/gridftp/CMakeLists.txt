cmake_minimum_required (VERSION 2.6)

if (PLUGIN_GRIDFTP)
    file (GLOB_RECURSE src_gridftp "*.c*")

    pkg_check_modules(GLOBUS_GASS_COPY_PKG REQUIRED globus-gass-copy)

    add_definitions( ${GLOBUS_GASS_COPY_PKG_CFLAGS})
    add_library (plugin_gridftp SHARED ${src_gridftp})

    target_link_libraries(plugin_gridftp gfal2 gfal2_transfer
			              ${GLOBUS_GASS_COPY_PKG_LIBRARIES}
                          gomp)
    include_directories(${GLOBUS_GASS_COPY_PKG_INCLUDE_DIRS}
	        			${GRIDFTP_IFCE_INCLUDE_DIRS} 	 
        				${GLIBMM_PKG_INCLUDE_DIRS})


    set_target_properties(plugin_gridftp    PROPERTIES
								LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/src
								CLEAN_DIRECT_OUTPUT 1
								OUTPUT_NAME "gfal_plugin_gridftp"
								LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)



    install(TARGETS		plugin_gridftp
	        LIBRARY		DESTINATION ${PLUGIN_INSTALL_DIR} )
	    
    install(FILES		"README_PLUGIN_GRIDFTP"
	    	DESTINATION ${DOC_INSTALL_DIR})	    

    # install gsiftp configuration files
    LIST(APPEND gsiftp_conf_file "${CMAKE_SOURCE_DIR}/dist/etc/gfal2.d/gsiftp_plugin.conf")
    install(FILES ${gsiftp_conf_file}
                        DESTINATION ${SYSCONF_INSTALL_DIR}/gfal2.d/)

endif (PLUGIN_GRIDFTP)


