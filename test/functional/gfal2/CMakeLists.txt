#
# compilation lines for examples


# gfal stat
FILE(GLOB src_gfalstat "gfal_teststat.c")
FILE(GLOB src_gfal_testchmod "gfal_testchmod.c")
FILE(GLOB src_gfalread "gfal_testread.c")
FILE(GLOB src_gfalwrite "gfal_testwrite.c")
FILE(GLOB src_gfalrw "gfal_testrw.c")
FILE(GLOB src_gfal_rw_full "gfal_testrw_full.c")
FILE(GLOB src_gfalget "gfal_testget.c")
FILE(GLOB src_gfaltestcreatedir "gfal_testcreatedir.c")
FILE(GLOB src_gfaltestrightchange "gfal_test_right_change.c")
FILE(GLOB src_gfaltestrmdirfull "gfal_test_rmdir_full.c")
FILE(GLOB src_test_readdir_full "gfal_test_readdir_full.c")
FILE(GLOB src_gfalmkdir_full "gfal_mkdir_full.c")
FILE(GLOB src_gfaltcopy "gfalt_copyfile.c")

SET(COPY_SRC "*.sh")
SET(COPY_DST "${CMAKE_CURRENT_BINARY_DIR}/")

IF (FUNCTIONAL_TESTS)



	include_directories( "${CMAKE_SOURCE_DIR}/src "  "${CMAKE_SOURCE_DIR}/src/posix/" ${GLIB2_PKG_INCLUDE_DIRS} )

	add_executable(gfal_stat	${src_gfalstat} )
	target_link_libraries(gfal_stat ${OUTPUT_NAME_MAIN} )
	
	FUNCTION(stat_test_all name valid)
		add_test(gfal_stat_${name} gfal_stat ${valid})
	ENDFUNCTION(stat_test_all valid)
	
	FUNCTION(chmod_test_all name valid right1 right2 right3 right4)
		add_test(gfal_test_right_change_${name} gfal_test_right_change ${valid} ${right1} ${right2} ${right3} ${right4})
	ENDFUNCTION(chmod_test_all name valid right1 right2 right3 right4)	
	

	FUNCTION(rmdir_test_all name valid_dir valid_file)
		add_test(gfal_test_rmdir_full_${name} gfal_test_rmdir_full ${valid_dir} ${valid_file} )
	ENDFUNCTION(rmdir_test_all name valid right1 right2 right3 right4)	
	
	FUNCTION(test_readdir_full name valid)
		add_test(gfal_test_readdir_full_${name} gfal_test_readdir_full ${valid})
	ENDFUNCTION(test_readdir_full valid)	

	FUNCTION(rwt_test_all name path size)
		add_test(gfal_rw_size_${name} gfal_rw_full ${path} ${size})
	ENDFUNCTION(rwt_test_all name path size)
	
	add_executable(gfal_read	${src_gfalread} )
	target_link_libraries(gfal_read ${OUTPUT_NAME_MAIN} )
	
	add_executable(gfal_write	${src_gfalwrite} )
	target_link_libraries(gfal_write ${OUTPUT_NAME_MAIN} )	

	add_executable(gfal_rw	${src_gfalrw} )
	target_link_libraries(gfal_rw ${OUTPUT_NAME_MAIN} )
	
	add_executable(gfal_rw_full	${src_gfal_rw_full} )
	target_link_libraries(gfal_rw_full ${OUTPUT_NAME_MAIN} )	
	
	add_executable(gfal_testget	${src_gfalget} )
	target_link_libraries(gfal_testget ${OUTPUT_NAME_MAIN} )
	
	add_executable(gfal_testchmod	${src_gfal_testchmod} )
	target_link_libraries(gfal_testchmod ${OUTPUT_NAME_MAIN} )	

	add_executable(gfal_createdir	${src_gfaltestcreatedir} )
	target_link_libraries(gfal_createdir ${OUTPUT_NAME_MAIN} )
	

	add_executable(gfal_test_mkdir_full	${src_gfalmkdir_full} )
	target_link_libraries(gfal_test_mkdir_full ${OUTPUT_NAME_MAIN} ${GLIB2_PKG_LIBRARIES})
	
	add_executable(gfal_test_right_change ${src_gfaltestrightchange})
	target_link_libraries(gfal_test_right_change ${OUTPUT_NAME_MAIN} ${GLIB2_PKG_LIBRARIES})	
	
	add_executable(gfal_test_rmdir_full ${src_gfaltestrmdirfull})
	target_link_libraries(gfal_test_rmdir_full ${OUTPUT_NAME_MAIN} ${GLIB2_PKG_LIBRARIES})	

	add_executable(gfal_test_readdir_full	${src_test_readdir_full} )
	target_link_libraries(gfal_test_readdir_full ${OUTPUT_NAME_MAIN} ${GLIB2_PKG_LIBRARIES})	
	

	FUNCTION(mkdir_test_all name prefix)
		add_test(gfal_mkdir_full_${name} gfal_test_mkdir_full ${prefix})
	ENDFUNCTION(mkdir_test_all name prefix)	
	
	IF (MAIN_TRANSFER)

		add_executable(gfal_copyfile ${src_gfaltcopy} )
		target_link_libraries(gfal_copyfile ${OUTPUT_NAME_TRANSFER} ${OUTPUT_NAME_MAIN})

		copy_files(${COPY_SRC} ${COPY_DST})
		add_test(functional_copyfile gfal2_copyfile.sh)
		add_test(functional_copyfile_srm gfal2_copyfile_srm.sh)
	ENDIF (MAIN_TRANSFER)	

	include(functional-test-parameters.cmake)	
	
ENDIF  (FUNCTIONAL_TESTS)
