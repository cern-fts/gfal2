## default set of build


## defaults sources
FILE(GLOB src_trans "*.cpp" )
FILE(GLOB src_libcpp "../libcpp/*.cpp")


# gfal2 link and link dir
SET(gfal2_link "gfal2")





IF (MAIN_TRANSFER)


	LIST(APPEND header_transfer "gfal_transfer.h" "gfal_transfer_types.h")

	include_directories( "../ " "../libcpp/" ${GLIB2_PKG_INCLUDE_DIRS} 
						${GTHREAD2_PKG_INCLUDE_DIRS}  ${GLIBMM_PKG_INCLUDE_DIRS})
	add_definitions( ${GLIB2_PKG_CFLAGS} ${GTHREAD2_PKG_CFLAGS} ${GLIBMM_PKG_CFLAGS})
	
        add_library(gfal2_transfer  SHARED ${src_trans} ${src_libcpp} ${src_exceptions} )
	target_link_libraries(gfal2_transfer ${GLIB2_PKG_LIBRARIES} ${GTHREAD2_PKG_LIBRARIES} 
							${UUID_PKG_LIBRARIES} ${gfal2_link}  ${GLIBMM_PKG_LIBRARIES} "dl")
	set_target_properties(gfal2_transfer	 	PROPERTIES
									LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/src
									VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
									SOVERSION ${VERSION_MAJOR}
									CLEAN_DIRECT_OUTPUT 1
									OUTPUT_NAME ${OUTPUT_NAME_TRANSFER}
	)
	add_PkgConfigFile_for_Library("gfal_transfer.pc" gfal2_transfer 
										HEADER_DIRS "gfal2"
										DESCRIPTION "gfal_transfer pkgconfig file"
										REQUIRES "gfal2")

	install(TARGETS gfal2_transfer
			LIBRARY       DESTINATION ${LIB_INSTALL_DIR} )
			
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/gfal_transfer.pc DESTINATION ${PKGCONFIG_FILES_DIR})
	install(FILES ${header_transfer} DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/transfer)
	install(FILES "README_TRANSFER"
			DESTINATION ${DOC_INSTALL_DIR})
ENDIF  (MAIN_TRANSFER)


link_directories (${CMAKE_BINARY_DIR}/src) 
