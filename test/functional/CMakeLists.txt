
if(FUNCTIONAL_TESTS)
        find_package(GTEST)

        include_directories(
            "${CMAKE_SOURCE_DIR}/src"
            "${CMAKE_SOURCE_DIR}/src/posix/"
            "${GLIB2_PKG_INCLUDE_DIRS}"
            "${GTEST_INCLUDE_DIR}"
        )
        
        # Bring online tests
        add_executable(gfal_test_bringonline "gfal_test_bringonline.cpp")
        target_link_libraries(gfal_test_bringonline ${GFAL2_LINK} test_shared)
        
        function(test_bringonline name dir_path)
            add_test(gfal_test_bringonline_${name} gfal_test_bringonline ${dir_path})
        endfunction(test_bringonline name valid_file)

        # Checksum tests
        add_executable(gfal_test_checksum "gfal_test_checksum.cpp")
        target_link_libraries(gfal_test_checksum ${GFAL2_LINK} test_shared)
        
        function(checksum_test_simple name dir_path algo)
            add_test(gfal_checksum_simple_${name} gfal_test_checksum ${dir_path} ${algo})
        endfunction(checksum_test_simple name valid_file algo)
        
        # Stat tests
        add_executable(gfal_test_stat "gfal_test_stat.c")
        target_link_libraries(gfal_test_stat ${GFAL2_LINK} test_shared)
        
        function(stat_test_all name valid)
            add_test(gfal_stat_${name} gfal_test_stat ${valid})
        endfunction(stat_test_all valid)
        
        # Deletion tests
        add_executable(gfal_test_del_nonex "gfal_test_del_nonex.c")
        target_link_libraries(gfal_test_del_nonex ${GFAL2_LINK} test_shared)
        
        add_executable(gfal_test_del "gfal_test_del.cpp") 
        target_link_libraries(gfal_test_del ${GFAL2_LINK} test_shared)
        
        add_executable(gfal_test_mkdir_unlink "gfal_test_mkdir_unlink.c")
        target_link_libraries(gfal_test_mkdir_unlink ${GFAL2_LINK} test_shared)

        function(test_gfal_test_del_nonex name dir_path)
            add_test(gfal_test_del_nonex_${name} gfal_test_del_nonex ${dir_path})
        endfunction(test_gfal_test_del_nonex name path)        

        function(test_gfal_test_del name dir_path)
            add_test(gfal_test_del_${name} gfal_test_del ${dir_path})
        endfunction(test_gfal_test_del name path)        

        function (test_gfal_test_mkdir_unlink name dir_path)
            add_test(gfal_test_mkdir_unlink_${name} gfal_test_mkdir_unlink ${dir_path})
        endfunction (test_gfal_test_mkdir_unlink name path)        

        # Chmod tests
        add_executable(gfal_test_chmod "gfal_test_chmod.cpp")
        target_link_libraries(gfal_test_chmod ${GFAL2_LINK} ${GLIB2_PKG_LIBRARIES} test_shared)
        
        function(chmod_test_all name valid right1 right2 right3 right4)
            add_test(gfal_test_right_change_${name} gfal_test_chmod ${valid} ${right1} ${right2} ${right3} ${right4})
        endfunction(chmod_test_all name valid right1 right2 right3 right4)        
        
        # Rmdir tests
        add_executable(gfal_test_rmdir_full "gfal_test_rmdir_full.cpp")
        target_link_libraries(gfal_test_rmdir_full ${GFAL2_LINK} ${GLIB2_PKG_LIBRARIES} test_shared)

        function(rmdir_test_all name valid_dir)
            add_test(gfal_test_rmdir_full_${name} gfal_test_rmdir_full ${valid_dir})
        endfunction(rmdir_test_all name valid_dir)
        
        # Readdir tests
        add_executable(gfal_test_readdir_full "gfal_test_readdir_full.cpp")
        target_link_libraries(gfal_test_readdir_full ${GFAL2_LINK} ${GLIB2_PKG_LIBRARIES} test_shared)
        
        function(test_readdir_full name valid)
                add_test(gfal_test_readdir_full_${name} gfal_test_readdir_full ${valid})
        endfunction(test_readdir_full valid)        

        # IO tests
        add_executable(gfal_test_rw_full "gfal_test_rw_full.cpp")
        target_link_libraries(gfal_test_rw_full ${GFAL2_LINK} test_shared)
            
        add_executable(gfal_test_rw_seq "gfal_test_rw_seq.cpp")
        target_link_libraries(gfal_test_rw_seq ${GFAL2_LINK} test_shared)
        
        add_executable(gfal_test_rw_seek "gfal_test_rw_seek.cpp")
        target_link_libraries(gfal_test_rw_seek ${GFAL2_LINK} test_shared) 
        
        function(rwt_test_all name path size)
            add_test(gfal_rw_size_${name} gfal_test_rw_full ${path} ${size})
        endfunction(rwt_test_all name path size)
        
        function(rwt_test_seq name path size_read size)
            add_test(gfal_rw_seq_${name} gfal_test_rw_seq ${path} ${size_read} ${size})
        endfunction(rwt_test_seq name path size_read size)
        
        function(rwt_test_seek name path size_read size)
            add_test(gfal_rw_seek_${name} gfal_test_rw_seek ${path} ${size_read} ${size})
        endfunction(rwt_test_seek name path size) 

        # Mkdir tests
        add_executable(gfal_test_mkdir_full "gfal_test_mkdir_full.cpp")
        target_link_libraries(gfal_test_mkdir_full ${GFAL2_LINK} ${GLIB2_PKG_LIBRARIES} test_shared)
        
        function(mkdir_test_all name prefix)
            add_test(gfal_mkdir_full_${name} gfal_test_mkdir_full ${prefix})
        endfunction(mkdir_test_all name prefix)    

        # Set credentials test
        add_executable(gfal_test_set_creds "gfal_test_set_creds.c")
        target_link_libraries(gfal_test_set_creds ${GFAL2_LINK} ${GLIB2_PKG_LIBRARIES})

        function(gfal_test_set_credentials name prefix)
            add_test(gfal_test_set_credentials_${name} gfal_test_set_creds ${prefix})
        endfunction(gfal_test_set_credentials name prefix)    

        # Tests for file transfer
        if(MAIN_TRANSFER)
                
                add_executable(gfalt_test_copy_file_cancel "gfalt_test_copy_file_cancel.cpp")
                target_link_libraries(gfalt_test_copy_file_cancel ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK} ${CMAKE_THREAD_LIBS_INIT} pthread)
                
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
                
                add_executable(gfalt_test_copy_bulk "gfalt_test_copy_bulk.cpp")
                target_link_libraries(gfalt_test_copy_bulk ${GFAL2_TRANSFER_LINK} ${GFAL2_LINK} test_shared)
                
                function(copy_file_test_full name src_dir dst_dir)
                    add_test(copyfile_checksum_${name} gfalt_test_copy_file_checksum ${src_dir} ${dst_dir})
                    add_test(copyfile_checksum_user_${name} gfalt_test_copy_file_checksum_user ${src_dir} ${dst_dir})
                    add_test(copyfile_replace_${name} gfalt_test_copy_file_replace ${src_dir} ${dst_dir})
                    add_test(copyfile_test_${name} gfalt_test_copy_file ${src_dir} ${dst_dir})
                    add_test(copyfile_test_mkdir_${name} gfalt_test_copy_file_mkdir ${src_dir} ${dst_dir})
                    add_test(copyfile_bulk_${name} gfalt_test_copy_bulk ${src_dir} ${dst_dir})
                endfunction(copy_file_test_full name dst_dir)

                function(copy_file_test_no_checksum name src_dir dst_dir)
                    add_test(copyfile_replace_${name} gfalt_test_copy_file_replace ${src_dir} ${dst_dir})
                    add_test(copyfile_test_${name} gfalt_test_copy_file ${src_dir} ${dst_dir})
                    add_test(copyfile_test_mkdir_${name} gfalt_test_copy_file_mkdir ${src_dir} ${dst_dir})
                    add_test(copyfile_bulk_${name} gfalt_test_copy_bulk ${src_dir} ${dst_dir})
                endfunction(copy_file_test_no_checksum name src_dir dst_dir)

                function(copy_file_test_simple name src_dir dst_dir)
                    add_test(copyfile_test_${name} gfalt_test_copy_file ${src_dir} ${dst_dir})
                endfunction(copy_file_test_simple name src_dir dst_dir)

        endif(MAIN_TRANSFER)

        include(functional-test-parameters.cmake)        
        
endif(FUNCTIONAL_TESTS)
