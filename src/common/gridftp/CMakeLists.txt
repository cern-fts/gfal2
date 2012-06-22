#grid ftp plugin compilation file
SET(PLUGIN_GRIDFTP TRUE
	CACHE STRING "enable compilation of the GRIDFTP plugin")

#define gridftp source
FILE(GLOB src_gridftp "*.c*")




IF(PLUGIN_GRIDFTP)

pkg_check_modules(GLOBUS_GASS_COPY_PKG REQUIRED globus-gass-copy)

#FILE(GLOB gridFTP_scripts "../dist/scripts/gfal_plugin_dcap/*sh")

add_definitions( ${GLOBUS_GASS_COPY_PKG_CFLAGS}    ${OpenMP_C_FLAGS})
add_library(plugin_gridftp  SHARED ${src_gridftp} ${src_libcpp} ${src_exceptions})

target_link_libraries(plugin_gridftp ${gfal2_link}
			${GLOBUS_GASS_COPY_PKG_LIBRARIES} gfal2_transfer
                        gomp
			)
include_directories( ${GLOBUS_GASS_COPY_PKG_INCLUDE_DIRS}
				${GRIDFTP_IFCE_INCLUDE_DIRS} 	 
				${GLIBMM_PKG_INCLUDE_DIRS}
                                ${OpenMP_INCLUDE_DIRS}
				)


set_target_properties(plugin_gridftp	 	PROPERTIES
								LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/src
								CLEAN_DIRECT_OUTPUT 1
								OUTPUT_NAME ${OUTPUT_NAME_GRIDFTP}
								LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins								
)



install(TARGETS		plugin_gridftp
	    LIBRARY		DESTINATION ${PLUGIN_INSTALL_DIR} )
	    
install(FILES		"README_PLUGIN_GRIDFTP"
		DESTINATION ${DOC_INSTALL_DIR})	    
#install(FILES ${dcap_scripts} DESTINATION ${SYSCONF_INSTALL_DIR}/profile.d/ )

ENDIF(PLUGIN_GRIDFTP)

link_directories (${CMAKE_BINARY_DIR}/plugins) 
link_directories (${CMAKE_BINARY_DIR}/src) 

add_subdirectory (test)




