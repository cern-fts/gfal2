#
# compilation lines for examples


#${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES}

SET(COPY_SRC "*.sh")
SET(COPY_DST "${CMAKE_CURRENT_BINARY_DIR}/")

IF (FUNCTIONAL_TESTS)
        find_package(GTEST)

        include_directories(
            "${CMAKE_SOURCE_DIR}/src"
            "${CMAKE_SOURCE_DIR}/src/posix/"
            ${GLIB2_PKG_INCLUDE_DIRS}
            ${GTEST_INCLUDE_DIR}
        )

        add_executable(gfal_test_checksum "gfal_test_checksum.c")
        target_link_libraries(gfal_test_checksum ${GFAL2_LINK} test_shared)

        add_executable(gfal_stat        "gfal_stat.c")
        target_link_libraries(gfal_stat ${GFAL2_LINK})

        add_executable(gfal_access        "gfal_access.c")
        target_link_libraries(gfal_access ${GFAL2_LINK})

        add_executable(gfal_checksum_tool        "gfal_checksum_tool.c")
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

        FUNCTION(checksum_test_simple name dir_path algo)
                add_test(gfal_checksum_simple_${name} gfal_test_checksum ${dir_path} ${algo})
        ENDFUNCTION(checksum_test_simple name valid_file algo)
        
        FUNCTION(chmod_test_all name valid right1 right2 right3 right4)
                add_test(gfal_test_right_change_${name} gfal_test_right_change ${valid} ${right1} ${right2} ${right3} ${right4})
        ENDFUNCTION(chmod_test_all name valid right1 right2 right3 right4)        
        

        FUNCTION(rmdir_test_all name valid_dir)
                add_test(gfal_test_rmdir_full_${name} gfal_test_rmdir_full ${valid_dir})
        ENDFUNCTION(rmdir_test_all name valid_dir)
        
        FUNCTION(test_readdir_full name valid)
                add_test(gfal_test_readdir_full_${name} gfal_test_readdir_full ${valid})
        ENDFUNCTION(test_readdir_full valid)        

        FUNCTION(rwt_test_all name path size)
                add_test(gfal_rw_size_${name} gfal_test_rw_full ${path} ${size})
        ENDFUNCTION(rwt_test_all name path size)
        
        FUNCTION(rwt_test_seq name path size size_read)
                add_test(gfal_rw_seq_${name} gfal_test_rw_seq ${path} ${size} ${size_read})                
        ENDFUNCTION(rwt_test_seq name path size size_read)
                
        add_executable(gfal_test_read        "gfal_test_read.c")
        target_link_libraries(gfal_test_read ${GFAL2_LINK} test_shared)
        
        add_executable(gfal_test_write        "gfal_test_write.c")
        target_link_libraries(gfal_test_write ${GFAL2_LINK} )        

        add_executable(gfal_test_rw        "gfal_test_rw.c")
        target_link_libraries(gfal_test_rw ${GFAL2_LINK})

        add_executable(gfal_test_del_nonex        "gfal_test_del_nonex.c")
        target_link_libraries(gfal_test_del_nonex ${GFAL2_LINK} test_shared)

        add_executable(gfal_test_del        "gfal_test_del.cpp") 
        target_link_libraries(gfal_test_del ${GFAL2_LINK} test_shared)
        
        add_executable(gfal_test_mkdir_unlink        "gfal_test_mkdir_unlink.c")
        target_link_libraries(gfal_test_mkdir_unlink ${GFAL2_LINK} test_shared)

        add_executable(gfal_test_rw_full        "gfal_test_rw_full.c")
        target_link_libraries(gfal_test_rw_full ${GFAL2_LINK} test_shared)
        
        add_executable(gfal_test_rw_seq        "gfal_test_rw_seq.c")
        target_link_libraries(gfal_test_rw_seq ${GFAL2_LINK} test_shared) 
        
        add_executable(gfal_test_get        "gfal_test_get.c")
        target_link_libraries(gfal_test_get ${GFAL2_LINK})
        
        add_executable(gfal_test_chmod        "gfal_test_chmod.c")
        target_link_libraries(gfal_test_chmod ${GFAL2_LINK})        

        add_executable(gfal_test_createdir        "gfal_test_createdir.c")
        target_link_libraries(gfal_test_createdir ${GFAL2_LINK} )

        add_executable(gfal_test_mkdir_full        "gfal_test_mkdir_full.c")
        target_link_libraries(gfal_test_mkdir_full ${GFAL2_LINK} ${GLIB2_PKG_LIBRARIES} test_shared)
        
        add_executable(gfal_test_right_change "gfal_test_right_change.c")
        target_link_libraries(gfal_test_right_change ${GFAL2_LINK} ${GLIB2_PKG_LIBRARIES} test_shared)
        
        add_executable(gfal_test_rmdir_full        "gfal_test_rmdir_full.c")
        target_link_libraries(gfal_test_rmdir_full ${GFAL2_LINK} ${GLIB2_PKG_LIBRARIES} test_shared)

        add_executable(gfal_test_readdir_full        "gfal_test_readdir_full.c")
        target_link_libraries(gfal_test_readdir_full ${GFAL2_LINK} ${GLIB2_PKG_LIBRARIES} test_shared)

        add_executable(gfal_ls        "gfal_ls.c")
        target_link_libraries(gfal_ls ${GFAL2_LINK} ${GLIB2_PKG_LIBRARIES})
   
        add_executable(gfal_bring_online "gfal_bring_online.c")
        target_link_libraries(gfal_bring_online ${GFAL2_LINK} ${GLIB2_PKG_LIBRARIES})
        
        add_executable(gfal_mv "gfal_mv.c")
        target_link_libraries(gfal_mv ${GFAL2_LINK} ${GLIB2_PKG_LIBRARIES})

        FUNCTION(mkdir_test_all name prefix)
                add_test(gfal_mkdir_full_${name} gfal_test_mkdir_full ${prefix})
        ENDFUNCTION(mkdir_test_all name prefix)        
        
        IF (MAIN_TRANSFER)

                add_executable(gfalt_copy_file "gfalt_copy_file.c")
                target_link_libraries(gfalt_copy_file ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK})
                
                add_executable(gfalt_test_copy_file_cancel "gfalt_test_copy_file_cancel.cpp")
                target_link_libraries(gfalt_test_copy_file_cancel ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK} ${CMAKE_THREAD_LIBS_INIT} pthread)

                add_executable(gfalt_copy_file_fts "gfalt_copy_file_fts.c")
                target_link_libraries(gfalt_copy_file_fts ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK})

                add_executable(gfalt_copy_file_fts_thread "gfalt_copy_file_fts_thread.c")
                target_link_libraries(gfalt_copy_file_fts_thread ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK} test_shared)

                add_executable(gfalt_copy_file_perf "gfalt_copy_file_perf.c")
                target_link_libraries(gfalt_copy_file_perf ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK})
                
                add_executable(gfalt_test_copy_file "gfalt_test_copy_file.cpp")
                target_link_libraries(gfalt_test_copy_file ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK} test_shared)

                add_executable(gfalt_test_copy_file_thead_safety "gfalt_test_copy_file_thead_safety.cpp")
                target_link_libraries(gfalt_test_copy_file_thead_safety ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK} test_shared)

                add_executable(gfalt_test_copy_file_timeout "gfalt_test_copy_file_timeout.cpp")
                target_link_libraries(gfalt_test_copy_file_timeout ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK} test_shared)
                
                add_executable(gfalt_test_copy_file_mkdir "gfalt_test_copy_file_mkdir.cpp")
                target_link_libraries(gfalt_test_copy_file_mkdir ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK} test_shared)
                
                add_executable(gfalt_test_copy_file_replace "gfalt_test_copy_file_replace.cpp")
                target_link_libraries(gfalt_test_copy_file_replace ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK} test_shared)

                add_executable(gfalt_test_copy_file_checksum "gfalt_test_copy_file_checksum.cpp")
                target_link_libraries(gfalt_test_copy_file_checksum ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK} test_shared)

                add_executable(gfalt_test_copy_file_checksum_user "gfalt_test_copy_file_checksum_user.cpp")
                target_link_libraries(gfalt_test_copy_file_checksum_user ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK} test_shared)
                
#                copy_files(${COPY_SRC} ${COPY_DST})
#                add_test(functional_copyfile gfal2_copyfile.sh)
#                add_test(functional_copyfile_srm gfal2_copyfile_srm.sh)
                
        FUNCTION(copy_file_test_full name src_dir dst_dir)
                add_test(copyfile_checksum_${name} gfalt_test_copy_file_checksum ${src_dir} ${dst_dir})
                add_test(copyfile_checksum_user_${name} gfalt_test_copy_file_checksum_user ${src_dir} ${dst_dir})
                add_test(copyfile_replace_${name} gfalt_test_copy_file_replace ${src_dir} ${dst_dir})
                add_test(copyfile_test_${name} gfalt_test_copy_file ${src_dir} ${dst_dir})
                add_test(copyfile_test_mkdir_${name} gfalt_test_copy_file_mkdir ${src_dir} ${dst_dir})
        ENDFUNCTION(copy_file_test_full name dst_dir)


        FUNCTION(copy_file_test_simple name src_dir dst_dir)
                add_test(copyfile_test_${name} gfalt_test_copy_file ${src_dir} ${dst_dir})
        ENDFUNCTION(copy_file_test_simple name src_dir dst_dir)


#        FUNCTION(copy_file_test_timeout name src_file dst_dir)
#                add_test(copy_file_test_timeout_${name} gfal_copyfile_test_timeout ${src_file} ${dst_dir})
#        ENDFUNCTION(copy_file_test_timeout name src_file dst_dir)

        ENDIF (MAIN_TRANSFER)

        include(functional-test-parameters.cmake)        
        
ENDIF  (FUNCTIONAL_TESTS)
