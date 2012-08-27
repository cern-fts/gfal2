#
# compilation lines for examples

# lib test
FILE(GLOB src_lib_test "gfal_lib_test.c")

# list tests
FILE(GLOB src_gfalchecksum_test "gfal_testchecksum.c")
FILE(GLOB src_gfalstat "gfal_teststat.c")
FILE(GLOB src_gfalaccess "gfal_testaccess.c")
FILE(GLOB src_gfal_testchmod "gfal_testchmod.c")
FILE(GLOB src_gfal_checksum_tool "gfal_checksum_tool.c")
FILE(GLOB src_gfalread "gfal_testread.c")
FILE(GLOB src_gfalwrite "gfal_testwrite.c")
FILE(GLOB src_gfalrw "gfal_testrw.c")
FILE(GLOB src_gfal_test_del_nonex "gfal_test_del_nonex.c")
FILE(GLOB src_gfal_test_del "gfal_test_del.c")
FILE(GLOB src_gfal_test_mkdir_unlink "gfal_test_mkdir_unlink.c")
FILE(GLOB src_gfal_rw_full "gfal_testrw_full.c")
FILE(GLOB src_gfal_rw_seq "gfal_testrw_seq.c")
FILE(GLOB src_gfalget "gfal_testget.c")
FILE(GLOB src_gfaltestcreatedir "gfal_testcreatedir.c")
FILE(GLOB src_gfaltestrightchange "gfal_test_right_change.c")
FILE(GLOB src_gfaltestrmdirfull "gfal_test_rmdir_full.c")
FILE(GLOB src_test_readdir_full "gfal_test_readdir_full.c")
FILE(GLOB src_gfalmkdir_full "gfal_mkdir_full.c")
FILE(GLOB src_gfaltcopy "gfalt_copyfile.c")
FILE(GLOB src_gfaltcopy_fts "gfalt_copyfile_fts_style.c")
FILE(GLOB src_gfaltcopy_test "gfalt_copyfile_test.c")
FILE(GLOB src_gfaltcopy_test_timeout "gfalt_copyfile_test_timeout.c")
FILE(GLOB src_gfaltcopy_mkdir_test "gfalt_copyfile_mkdir_test.c")
FILE(GLOB src_gfaltcopy_perf "gfalt_copyfile_with_performance_marker.c")
FILE(GLOB src_copyfile_replace "gfalt_copyfile_replace_test.c")
FILE(GLOB src_copyfile_checksum "gfalt_copyfile_checksum_test_simple.c")
FILE(GLOB src_copyfile_checksum_user "gfalt_copyfile_checksum_test_userdefined.c")

SET(COPY_SRC "*.sh")
SET(COPY_DST "${CMAKE_CURRENT_BINARY_DIR}/")

IF (FUNCTIONAL_TESTS)

  #  pkg_search_module(NETTLE_PKG REQUIRED nettle)

        include_directories( "${CMAKE_SOURCE_DIR}/src "  "${CMAKE_SOURCE_DIR}/src/posix/"
                    ${GLIB2_PKG_INCLUDE_DIRS} ${NETTLE_PKG_INCLUDE_DIRS} )

        add_executable(gfal_stat	${src_gfalstat} )
	target_link_libraries(gfal_stat ${OUTPUT_NAME_MAIN} )

        add_executable(gfal_access	${src_gfalaccess} )
	target_link_libraries(gfal_access ${OUTPUT_NAME_MAIN} )

   #     add_executable(gfal_checksum_test	${src_gfalchecksum_test} ${src_lib_test} )
    #    target_link_libraries(gfal_checksum_test ${OUTPUT_NAME_MAIN} ${NETTLE_PKG_LIBRARIES})

        add_executable(gfal_checksum_tool	${src_gfal_checksum_tool})
        target_link_libraries(gfal_checksum_tool ${OUTPUT_NAME_MAIN})
	
	FUNCTION(stat_test_all name valid)
		add_test(gfal_stat_${name} gfal_stat ${valid})
	ENDFUNCTION(stat_test_all valid)

	FUNCTION(test_gfal_test_del_nonex name path)
		add_test(gfal_test_del_nonex_${name} gfal_test_del_nonex ${path})
	ENDFUNCTION(test_gfal_test_del_nonex name path)	

	FUNCTION(test_gfal_test_del name path)
		add_test(gfal_test_del_${name} gfal_test_del ${path})
	ENDFUNCTION(test_gfal_test_del name path)	

	FUNCTION(test_gfal_test_mkdir_unlink name path)
		add_test(gfal_test_mkdir_unlink_${name} gfal_test_mkdir_unlink ${path})
	ENDFUNCTION(test_gfal_test_mkdir_unlink name path)	

        FUNCTION(checksum_test_all name valid)
       #         add_test(gfal_checksum_${name} gfal_checksum_test ${valid})
        ENDFUNCTION(checksum_test_all name valid)

        FUNCTION(checksum_test_simple name valid_file algo)
                add_test(gfal_checksum_simple_${name} gfal_checksum_tool ${valid_file} ${algo})
        ENDFUNCTION(checksum_test_simple name valid_file algo)
	
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
	
	FUNCTION(rwt_test_seq name path size size_read)
		add_test(gfal_rw_seq_${name} gfal_rw_seq ${path} ${size} ${size_read})		
	ENDFUNCTION(rwt_test_seq name path size size_read)
		
	add_executable(gfal_read	${src_gfalread} )
	target_link_libraries(gfal_read ${OUTPUT_NAME_MAIN} )
	
	add_executable(gfal_write	${src_gfalwrite} )
	target_link_libraries(gfal_write ${OUTPUT_NAME_MAIN} )	

	add_executable(gfal_rw	${src_gfalrw} )
	target_link_libraries(gfal_rw ${OUTPUT_NAME_MAIN} )

	add_executable(gfal_test_del_nonex   ${src_gfal_test_del_nonex} )
	target_link_libraries(gfal_test_del_nonex ${OUTPUT_NAME_MAIN} )

	add_executable(gfal_test_del   ${src_gfal_test_del} )
	target_link_libraries(gfal_test_del ${OUTPUT_NAME_MAIN} )
	
	add_executable(gfal_test_mkdir_unlink   ${src_gfal_test_mkdir_unlink} )
	target_link_libraries(gfal_test_mkdir_unlink ${OUTPUT_NAME_MAIN} )

	add_executable(gfal_rw_full	${src_gfal_rw_full} )
	target_link_libraries(gfal_rw_full ${OUTPUT_NAME_MAIN} )	
	
	add_executable(gfal_rw_seq	${src_gfal_rw_seq} )
	target_link_libraries(gfal_rw_seq ${OUTPUT_NAME_MAIN} )		
	
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

                add_executable(gfal_copyfile_fts ${src_gfaltcopy_fts} )
                target_link_libraries(gfal_copyfile_fts ${OUTPUT_NAME_TRANSFER} ${OUTPUT_NAME_MAIN})

                add_executable(gfal_copyfile_perf ${src_gfaltcopy_perf} )
                target_link_libraries(gfal_copyfile_perf ${OUTPUT_NAME_TRANSFER} ${OUTPUT_NAME_MAIN})
		
		add_executable(gfal_copyfile_test ${src_gfaltcopy_test} ${src_lib_test})
                target_link_libraries(gfal_copyfile_test ${OUTPUT_NAME_TRANSFER} ${OUTPUT_NAME_MAIN})

                add_executable(gfal_copyfile_test_timeout ${src_gfaltcopy_test_timeout} ${src_lib_test})
                target_link_libraries(gfal_copyfile_test_timeout ${OUTPUT_NAME_TRANSFER} ${OUTPUT_NAME_MAIN})
		
		add_executable(gfal_copyfile_mkdir_test ${src_gfaltcopy_mkdir_test} ${src_lib_test})
		target_link_libraries(gfal_copyfile_mkdir_test ${OUTPUT_NAME_TRANSFER} ${OUTPUT_NAME_MAIN})		
		
		add_executable(copyfile_replace ${src_copyfile_replace} ${src_lib_test})
		target_link_libraries(copyfile_replace ${OUTPUT_NAME_TRANSFER} ${OUTPUT_NAME_MAIN})

                add_executable(copyfile_checksum ${src_copyfile_checksum} ${src_lib_test})
                target_link_libraries(copyfile_checksum ${OUTPUT_NAME_TRANSFER} ${OUTPUT_NAME_MAIN})

                add_executable(copyfile_checksum_user ${src_copyfile_checksum_user} ${src_lib_test})
                target_link_libraries(copyfile_checksum_user ${OUTPUT_NAME_TRANSFER} ${OUTPUT_NAME_MAIN})
		
#		copy_files(${COPY_SRC} ${COPY_DST})
#		add_test(functional_copyfile gfal2_copyfile.sh)
#		add_test(functional_copyfile_srm gfal2_copyfile_srm.sh)
		
	FUNCTION(copy_file_test_full name src_file dir_src)
                add_test(copyfile_checksum_${name} copyfile_checksum ${src_file} ${dir_src})
                add_test(copyfile_checksum_user_${name} copyfile_checksum_user ${src_file} ${dir_src})
		add_test(copyfile_replace_${name} copyfile_replace ${src_file} ${dir_src})
		add_test(copyfile_test_${name} gfal_copyfile_test ${src_file} ${dir_src})
		add_test(copyfile_test_mkdir_${name} gfal_copyfile_mkdir_test ${src_file} ${dir_src})		
	ENDFUNCTION(copy_file_test_full name dir_src)


        FUNCTION(copy_file_test_simple name src_file dir_src)
                add_test(copyfile_test_${name} gfal_copyfile_test ${src_file} ${dir_src})
        ENDFUNCTION(copy_file_test_simple name src_file dir_src)

        ENDIF (MAIN_TRANSFER)

	include(functional-test-parameters.cmake)	
	
ENDIF  (FUNCTIONAL_TESTS)
