#
# compilation lines for examples


# gfal stat
FILE(GLOB src_gfalstat "gfal_teststat.c")
FILE(GLOB src_gfalread "gfal_testread.c")
FILE(GLOB src_gfalwrite "gfal_testwrite.c")
FILE(GLOB src_gfalrw "gfal_testrw.c")
FILE(GLOB src_gfalget "gfal_testget.c")
FILE(GLOB src_gfaltestcreatedir "gfal_testcreatedir.c")
FILE(GLOB src_gfaltcopy "gfalt_copyfile.c")

SET(COPY_SRC "*.sh")
SET(COPY_DST "${CMAKE_CURRENT_BINARY_DIR}/")

IF (FUNCTIONAL_TESTS)


	include_directories( "${CMAKE_SOURCE_DIR}/src "  "${CMAKE_SOURCE_DIR}/src/posix/" ${GLIB2_PKG_INCLUDE_DIRS} )

	add_executable(gfal_stat	${src_gfalstat} )
	target_link_libraries(gfal_stat ${OUTPUT_NAME_MAIN} )
	
	add_executable(gfal_read	${src_gfalread} )
	target_link_libraries(gfal_read ${OUTPUT_NAME_MAIN} )
	
	add_executable(gfal_write	${src_gfalwrite} )
	target_link_libraries(gfal_write ${OUTPUT_NAME_MAIN} )	

	add_executable(gfal_rw	${src_gfalrw} )
	target_link_libraries(gfal_rw ${OUTPUT_NAME_MAIN} )
	
	add_executable(gfal_testget	${src_gfalget} )
	target_link_libraries(gfal_testget ${OUTPUT_NAME_MAIN} )

	add_executable(gfal_createdir	${src_gfaltestcreatedir} )
	target_link_libraries(gfal_createdir ${OUTPUT_NAME_MAIN} )
	
	add_executable(gfal_copyfile ${src_gfaltcopy} )
	target_link_libraries(gfal_copyfile ${OUTPUT_NAME_TRANSFER} )


	copy_files(${COPY_SRC} ${COPY_DST})
	add_test(functional_copyfile gfal2_copyfile.sh)
	add_test(functional_copyfile_srm gfal2_copyfile_srm.sh)
	
ENDIF  (FUNCTIONAL_TESTS)
