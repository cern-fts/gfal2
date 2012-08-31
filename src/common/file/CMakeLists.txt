#File plugin compilation file


#define gridftp source
FILE(GLOB src_file "*.c*")




IF(PLUGIN_FILE)


add_library(plugin_file  SHARED ${src_file})

target_link_libraries(plugin_file ${gfal2_link}
			)


set_target_properties(plugin_file	 	PROPERTIES
								LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/src
								CLEAN_DIRECT_OUTPUT 1
								OUTPUT_NAME ${OUTPUT_NAME_FILE}
								LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins								
)



install(TARGETS		plugin_file
	    LIBRARY		DESTINATION ${PLUGIN_INSTALL_DIR} )
	    
install(FILES		"README_PLUGIN_FILE"
		DESTINATION ${DOC_INSTALL_DIR})	    

ENDIF(PLUGIN_FILE)

link_directories (${CMAKE_BINARY_DIR}/plugins) 
link_directories (${CMAKE_BINARY_DIR}/src) 

add_subdirectory (test)




