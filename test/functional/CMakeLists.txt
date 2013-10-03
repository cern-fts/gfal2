#
# compilation lines for examples

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
FILE(GLOB src_gfaltcopy_cancel "gfalt_copyfile_cancel.c")
FILE(GLOB src_gfaltcopy_fts "gfalt_copyfile_fts_style.c")
FILE(GLOB src_gfaltcopy_fts_thread "gfalt_copyfile_fts_style_thread_safety.c")
FILE(GLOB src_gfaltcopy_test "gfalt_copyfile_test.c")
FILE(GLOB src_gfaltcopy_test_timeout "gfalt_copyfile_test_timeout.c")
FILE(GLOB src_gfaltcopy_test_thread_safety_timeout "gfalt_copyfile_test_thread_safey_timeout.c")
FILE(GLOB src_gfaltcopy_mkdir_test "gfalt_copyfile_mkdir_test.c")
FILE(GLOB src_gfaltcopy_perf "gfalt_copyfile_with_performance_marker.c")
FILE(GLOB src_copyfile_replace "gfalt_copyfile_replace_test.c")
FILE(GLOB src_copyfile_checksum "gfalt_copyfile_checksum_test_simple.c")
FILE(GLOB src_copyfile_checksum_user "gfalt_copyfile_checksum_test_userdefined.c")
FILE(GLOB src_testdir "gfal_testdir.c")
FILE(GLOB src_bring_online "gfal_bring_online.c")
FILE(GLOB src_mv "gfal_mv.c")

SET(COPY_SRC "*.sh")
SET(COPY_DST "${CMAKE_CURRENT_BINARY_DIR}/")

IF (FUNCTIONAL_TESTS)


        include_directories( "${CMAKE_SOURCE_DIR}/src "  "${CMAKE_SOURCE_DIR}/src/posix/"
                    ${GLIB2_PKG_INCLUDE_DIRS} ${NETTLE_PKG_INCLUDE_DIRS} )

        add_executable(gfal_stat        ${src_gfalstat} )
        target_link_libraries(gfal_stat ${GFAL2_LINK} )

        add_executable(gfal_access        ${src_gfalaccess} )
        target_link_libraries(gfal_access ${GFAL2_LINK} )

        add_executable(gfal_checksum_tool        ${src_gfal_checksum_tool})
        target_link_libraries(gfal_checksum_tool ${GFAL2_LINK})
        
        FUNCTION(stat_test_all name valid)
                add_test(gfal_stat_${name} gfal_stat ${valid})
        ENDFUNCTION(stat_test_all valid)

        FUNCTION(test_gfal_test_del_nonex name dir_path)
                add_test(gfal_test_del_nonex_${name} gfal_test_del_nonex ${dir_path})
        ENDFUNCTION(test_gfal_test_del_nonex name path)        

        FUNCTION(test_gfal_test_del name dir_path)
                add_test(gfal_test_del_${name} gfal_test_del ${dir_path})
        ENDFUNCTION(test_gfal_test_del name path)        

        FUNCTION(test_gfal_test_mkdir_unlink name dir_path)
                add_test(gfal_test_mkdir_unlink_${name} gfal_test_mkdir_unlink ${dir_path})
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
                
        add_executable(gfal_read        ${src_gfalread} )
        target_link_libraries(gfal_read ${GFAL2_LINK} )
        
        add_executable(gfal_write        ${src_gfalwrite} )
        target_link_libraries(gfal_write ${GFAL2_LINK} )        

        add_executable(gfal_rw        ${src_gfalrw} )
        target_link_libraries(gfal_rw ${GFAL2_LINK} )

        add_executable(gfal_test_del_nonex   ${src_gfal_test_del_nonex})
        target_link_libraries(gfal_test_del_nonex ${GFAL2_LINK} test_shared)

        add_executable(gfal_test_del   ${src_gfal_test_del}) 
        target_link_libraries(gfal_test_del ${GFAL2_LINK} test_shared)
        
        add_executable(gfal_test_mkdir_unlink   ${src_gfal_test_mkdir_unlink})
        target_link_libraries(gfal_test_mkdir_unlink ${GFAL2_LINK} test_shared)

        add_executable(gfal_rw_full        ${src_gfal_rw_full} )
        target_link_libraries(gfal_rw_full ${GFAL2_LINK} test_shared)
        
        add_executable(gfal_rw_seq        ${src_gfal_rw_seq} )
        target_link_libraries(gfal_rw_seq ${GFAL2_LINK} test_shared) 
        
        add_executable(gfal_testget        ${src_gfalget} )
        target_link_libraries(gfal_testget ${GFAL2_LINK} )
        
        add_executable(gfal_testchmod        ${src_gfal_testchmod} )
        target_link_libraries(gfal_testchmod ${GFAL2_LINK} )        

        add_executable(gfal_createdir        ${src_gfaltestcreatedir} )
        target_link_libraries(gfal_createdir ${GFAL2_LINK} )
        

        add_executable(gfal_test_mkdir_full        ${src_gfalmkdir_full} )
        target_link_libraries(gfal_test_mkdir_full ${GFAL2_LINK} ${GLIB2_PKG_LIBRARIES} test_shared)
        
        add_executable(gfal_test_right_change ${src_gfaltestrightchange} ${src_lib_test})
        target_link_libraries(gfal_test_right_change ${GFAL2_LINK} ${GLIB2_PKG_LIBRARIES} test_shared)
        
        add_executable(gfal_test_rmdir_full ${src_gfaltestrmdirfull})
        target_link_libraries(gfal_test_rmdir_full ${GFAL2_LINK} ${GLIB2_PKG_LIBRARIES} test_shared)

        add_executable(gfal_test_readdir_full        ${src_test_readdir_full} )
        target_link_libraries(gfal_test_readdir_full ${GFAL2_LINK} ${GLIB2_PKG_LIBRARIES} test_shared)

        add_executable(gfal_ls ${src_testdir})
        target_link_libraries(gfal_ls ${GFAL2_LINK} ${GLIB2_PKG_LIBRARIES})
   
        add_executable(gfal_bring_online ${src_bring_online})
        target_link_libraries(gfal_bring_online ${GFAL2_LINK} ${GLIB2_PKG_LIBRARIES})
        
        add_executable(gfal_mv ${src_mv})
        target_link_libraries(gfal_mv ${GFAL2_LINK} ${GLIB2_PKG_LIBRARIES})

        FUNCTION(mkdir_test_all name prefix)
                add_test(gfal_mkdir_full_${name} gfal_test_mkdir_full ${prefix})
        ENDFUNCTION(mkdir_test_all name prefix)        
        
        IF (MAIN_TRANSFER)

                add_executable(gfal_copyfile ${src_gfaltcopy} )
                target_link_libraries(gfal_copyfile ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK})
                
                add_executable(gfal_copyfile_cancel ${src_gfaltcopy_cancel} )
                target_link_libraries(gfal_copyfile_cancel ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK} ${CMAKE_THREAD_LIBS_INIT} pthread)

                add_executable(gfal_copyfile_fts ${src_gfaltcopy_fts} )
                target_link_libraries(gfal_copyfile_fts ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK})

                add_executable(gfal_copyfile_fts_thread ${src_gfaltcopy_fts_thread} ${src_lib_test})
                target_link_libraries(gfal_copyfile_fts_thread ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK} test_shared)

                add_executable(gfal_copyfile_perf ${src_gfaltcopy_perf} )
                target_link_libraries(gfal_copyfile_perf ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK})
                
                add_executable(gfal_copyfile_test ${src_gfaltcopy_test} ${src_lib_test})
                target_link_libraries(gfal_copyfile_test ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK} test_shared)

                add_executable(gfal_copyfile_thead_safety_test ${src_gfaltcopy_test_thread_safety_timeout} ${src_lib_test})
                target_link_libraries(gfal_copyfile_thead_safety_test ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK} test_shared)

                add_executable(gfal_copyfile_test_timeout ${src_gfaltcopy_test_timeout} ${src_lib_test})
                target_link_libraries(gfal_copyfile_test_timeout ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK} test_shared)
                
                add_executable(gfal_copyfile_mkdir_test ${src_gfaltcopy_mkdir_test} ${src_lib_test})
                target_link_libraries(gfal_copyfile_mkdir_test ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK} test_shared)
                
                add_executable(copyfile_replace ${src_copyfile_replace})
                target_link_libraries(copyfile_replace ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK} test_shared)

                add_executable(copyfile_checksum ${src_copyfile_checksum})
                target_link_libraries(copyfile_checksum ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK} test_shared)

                add_executable(copyfile_checksum_user ${src_copyfile_checksum_user})
                target_link_libraries(copyfile_checksum_user ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK} test_shared)
                
#                copy_files(${COPY_SRC} ${COPY_DST})
#                add_test(functional_copyfile gfal2_copyfile.sh)
#                add_test(functional_copyfile_srm gfal2_copyfile_srm.sh)
                
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


        FUNCTION(copy_file_test_timeout name src_file dir_src)
                add_test(copy_file_test_timeout_${name} gfal_copyfile_test_timeout ${src_file} ${dir_src})
        ENDFUNCTION(copy_file_test_timeout name src_file dir_src)

        ENDIF (MAIN_TRANSFER)

        include(functional-test-parameters.cmake)        
        
ENDIF  (FUNCTIONAL_TESTS)
