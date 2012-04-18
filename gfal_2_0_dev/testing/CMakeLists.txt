#
# compilation lines for examples
SET(GFAL_EXAMPLES FALSE
	CACHE STRING "Example for gfal ")

# gfal stat
FILE(GLOB src_gfalstat "example/gfal_teststat.c")
FILE(GLOB src_gfalread "example/gfal_testread.c")
FILE(GLOB src_gfalread "example/gfal_testwrite.c")
FILE(GLOB src_gfalrw "example/gfal_testrw.c")
FILE(GLOB src_gfalget "example/gfal_testget.c")
FILE(GLOB src_gfaltcopy "example/gfalt_copyfile.c")

IF (GFAL_EXAMPLES)


	include_directories( "../src "  "../src/posix/" ${GLIB2_PKG_INCLUDE_DIRS} )

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
	
	add_executable(gfal_copyfile ${src_gfaltcopy} )
	target_link_libraries(gfal_copyfile ${OUTPUT_NAME_TRANSFER} )
	
ENDIF  (GFAL_EXAMPLES)
