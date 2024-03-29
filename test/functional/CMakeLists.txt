function (add_test_executable name source)
add_executable(${name} ${source})
install(TARGETS ${name}
        DESTINATION ${SHARE_INSTALL_PREFIX}/gfal2/tests)
endfunction(add_test_executable name)


if(FUNCTIONAL_TESTS)
    find_package(GTEST)

    include_directories(
        "${CMAKE_SOURCE_DIR}/src"
        "${CMAKE_SOURCE_DIR}/test"
        "${CMAKE_SOURCE_DIR}/src/posix/"
        "${GLIB2_INCLUDE_DIRS}"
        "${GTEST_INCLUDE_DIR}"
        "${JSONC_INCLUDE_DIRS}"
    )

    # Bring online tests
    add_test_executable(gfal_test_bringonline "gfal_test_bringonline.cpp")
    target_link_libraries(gfal_test_bringonline ${GFAL2_LIBRARIES} gfal2_test_shared)

    function(test_bringonline name dir_path)
        add_test(gfal_test_bringonline_${name} gfal_test_bringonline ${dir_path})
    endfunction(test_bringonline name valid_file)

    # Archive tests
    add_test_executable(gfal_test_archive "gfal_test_archive.cpp")
    target_link_libraries(gfal_test_archive ${GFAL2_LIBRARIES} gfal2_test_shared)

    function(test_archive name dir_path)
        add_test(gfal_test_archive_${name} gfal_test_archive ${dir_path})
    endfunction(test_archive name dir_path)

    # Checksum tests
    add_test_executable(gfal_test_checksum "gfal_test_checksum.cpp")
    target_link_libraries(gfal_test_checksum ${GFAL2_LIBRARIES} gfal2_test_shared)

    function(test_checksum_simple name dir_path algo)
        add_test(gfal_checksum_simple_${name} gfal_test_checksum ${dir_path} ${algo})
    endfunction(test_checksum_simple name valid_file algo)

    # Stat tests
    add_test_executable(gfal_test_stat "gfal_test_stat.cpp")
    target_link_libraries(gfal_test_stat ${GFAL2_LIBRARIES} gfal2_test_shared)

    function(test_stat_all name valid)
        add_test(gfal_stat_${name} gfal_test_stat ${valid})
    endfunction(test_stat_all valid)

    # Access test
    add_test_executable(gfal_test_access "gfal_test_access.cpp")
    target_link_libraries(gfal_test_access ${GFAL2_LIBRARIES} gfal2_test_shared)

    function(test_access name dir_path)
        add_test(gfal_test_access_${name} gfal_test_access ${dir_path})
    endfunction(test_access name dir_path)

    # XAttr test
    add_test_executable(gfal_test_xattr "gfal_test_xattr.cpp")
    target_link_libraries(gfal_test_xattr ${GFAL2_LIBRARIES} gfal2_test_shared)

    function(test_xattr name dir_path)
        add_test(gfal_test_xattr_${name} gfal_test_xattr ${dir_path} ${ARGV2})
    endfunction(test_xattr name dir_path)

    # Deletion tests
    add_test_executable(gfal_test_del "gfal_test_del.cpp")
    target_link_libraries(gfal_test_del ${GFAL2_LIBRARIES} gfal2_test_shared)

    function(test_del name dir_path)
        add_test(gfal_test_del_${name} gfal_test_del ${dir_path})
    endfunction(test_del name path)

    # QoS tests
    add_test_executable(gfal_test_qos "gfal_test_qos.cpp")
    target_link_libraries(gfal_test_qos ${GFAL2_LIBRARIES} gfal2_test_shared)

    function(test_qos name)
        add_test(gfal_test_qos_${name} gfal_test_qos)
    endfunction(test_qos name)

    # Token tests
    add_test_executable(gfal_test_token "gfal_test_token.cpp")
    target_link_libraries(gfal_test_token ${GFAL2_LIBRARIES} gfal2_test_shared)

    function(test_token name)
      add_test(gfal_test_token_${name} gfal_test_token)
    endfunction(test_token name)

    # Rename tests
    add_test_executable(gfal_test_rename "gfal_test_rename.cpp")
    target_link_libraries(gfal_test_rename ${GFAL2_LIBRARIES} gfal2_test_shared)

    function(test_rename name dir_path)
        add_test(gfal_test_rename_${name} gfal_test_rename ${dir_path})
    endfunction(test_rename name dir_path)

    # Chmod tests
    add_test_executable(gfal_test_chmod "gfal_test_chmod.cpp")
    target_link_libraries(gfal_test_chmod ${GFAL2_LIBRARIES} gfal2_test_shared)

    function(test_chmod_all name valid right1 right2 right3 right4)
        add_test(gfal_test_right_change_${name} gfal_test_chmod ${valid} ${right1} ${right2} ${right3} ${right4})
    endfunction(test_chmod_all name valid right1 right2 right3 right4)

    # Rmdir tests
    add_test_executable(gfal_test_rmdir_full "gfal_test_rmdir_full.cpp")
    target_link_libraries(gfal_test_rmdir_full ${GFAL2_LIBRARIES} gfal2_test_shared)

    function(test_rmdir_all name valid_dir)
        add_test(gfal_test_rmdir_full_${name} gfal_test_rmdir_full ${valid_dir})
    endfunction(test_rmdir_all name valid_dir)

    # Readdir tests
    add_test_executable(gfal_test_readdir_full "gfal_test_readdir_full.cpp")
    target_link_libraries(gfal_test_readdir_full ${GFAL2_LIBRARIES} gfal2_test_shared)

    function(test_readdir_full name valid)
            add_test(gfal_test_readdir_full_${name} gfal_test_readdir_full ${valid})
    endfunction(test_readdir_full valid)

    # IO tests
    add_test_executable(gfal_test_rw_full "gfal_test_rw_full.cpp")
    target_link_libraries(gfal_test_rw_full ${GFAL2_LIBRARIES} gfal2_test_shared)

    add_test_executable(gfal_test_rw_seq "gfal_test_rw_seq.cpp")
    target_link_libraries(gfal_test_rw_seq ${GFAL2_LIBRARIES} gfal2_test_shared)

    add_test_executable(gfal_test_rw_seek "gfal_test_rw_seek.cpp")
    target_link_libraries(gfal_test_rw_seek ${GFAL2_LIBRARIES} gfal2_test_shared)

    function(test_rwt_all name path size)
        add_test(gfal_rw_size_${name} gfal_test_rw_full ${path} ${size})
    endfunction(test_rwt_all name path size)

    function(test_rwt_seq name path size_read size)
        add_test(gfal_rw_seq_${name} gfal_test_rw_seq ${path} ${size_read} ${size})
    endfunction(test_rwt_seq name path size_read size)

    function(test_rwt_seek name path size_read size)
        add_test(gfal_rw_seek_${name} gfal_test_rw_seek ${path} ${size_read} ${size})
    endfunction(test_rwt_seek name path size)

    # Mkdir tests
    add_test_executable(gfal_test_mkdir_full "gfal_test_mkdir_full.cpp")
    target_link_libraries(gfal_test_mkdir_full ${GFAL2_LIBRARIES} gfal2_test_shared)

    function(test_mkdir_all name prefix)
        add_test(gfal_mkdir_full_${name} gfal_test_mkdir_full ${prefix})
    endfunction(test_mkdir_all name prefix)

    # Register test
    add_test_executable(gfal_test_register "gfal_test_register.cpp")
    target_link_libraries(gfal_test_register ${GFAL2_LIBRARIES} gfal2_test_shared)

    function(test_register name origin lfc)
        add_test(gfal_test_register_${name} gfal_test_register ${origin} ${lfc})
    endfunction(test_register name origin lfc)

    # Set credentials test
    add_test_executable(gfal_test_set_creds "gfal_test_set_creds.cpp")
    target_link_libraries(gfal_test_set_creds ${GFAL2_LIBRARIES} gfal2_test_shared)

    function(gfal_test_set_credentials name prefix)
        add_test(gfal_test_set_credentials_${name} gfal_test_set_creds ${prefix})
    endfunction(gfal_test_set_credentials name prefix)

    # Posix test
    add_test_executable(gfal_test_posix "gfal_test_posix.cpp")
    target_link_libraries(gfal_test_posix ${GFAL2_LIBRARIES} gfal2_test_shared)

    function(gfal_test_posix name prefix)
        add_test(gfal_test_posix_${name} gfal_test_posix ${prefix})
    endfunction(gfal_test_posix name prefix)

    # Space test
    add_test_executable(gfal_test_space "gfal_test_space.cpp")
    target_link_libraries(gfal_test_space ${GFAL2_LIBRARIES} gfal2_test_shared)
    function(test_space name prefix)
        add_test(gfal_test_space_${name} gfal_test_space ${prefix})
    endfunction(test_space name prefix)

    # Tests for file transfer
    if(MAIN_TRANSFER)

		add_test_executable(gfalt_test_rd3_reorder_protocols "gfalt_test_rd3_reorder_protocols.cpp")
        target_link_libraries(gfalt_test_rd3_reorder_protocols ${GFAL2_TRANSFER_LINK} ${GFAL2_LIBRARIES} gfal2_test_shared)
        function(test_rd3_reorder_protocols name src_dir dst_dir)
            add_test(rd3_reorder_protocols_${name} gfalt_test_rd3_reorder_protocols ${src_dir} ${dst_dir})
        endfunction(test_rd3_reorder_protocols name dst_dir)

        add_test_executable(gfalt_test_copy_file_cancel "gfalt_test_copy_file_cancel.cpp")
        target_link_libraries(gfalt_test_copy_file_cancel ${GFAL2_TRANSFER_LINK} ${GFAL2_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} pthread)

        add_test_executable(gfalt_test_copy_file "gfalt_test_copy_file.cpp")
        target_link_libraries(gfalt_test_copy_file ${GFAL2_TRANSFER_LINK} ${GFAL2_LIBRARIES} gfal2_test_shared)

        add_test_executable(gfalt_test_copy_file_timeout "gfalt_test_copy_file_timeout.cpp")
        target_link_libraries(gfalt_test_copy_file_timeout ${GFAL2_TRANSFER_LINK} ${GFAL2_LIBRARIES} gfal2_test_shared)

        add_test_executable(gfalt_test_copy_file_mkdir "gfalt_test_copy_file_mkdir.cpp")
        target_link_libraries(gfalt_test_copy_file_mkdir ${GFAL2_TRANSFER_LINK} ${GFAL2_LIBRARIES} gfal2_test_shared)

        add_test_executable(gfalt_test_copy_file_replace "gfalt_test_copy_file_replace.cpp")
        target_link_libraries(gfalt_test_copy_file_replace ${GFAL2_TRANSFER_LINK} ${GFAL2_LIBRARIES} gfal2_test_shared)

        add_test_executable(gfalt_test_copy_file_checksum "gfalt_test_copy_file_checksum.cpp")
        target_link_libraries(gfalt_test_copy_file_checksum ${GFAL2_TRANSFER_LINK} ${GFAL2_LIBRARIES} gfal2_test_shared)

        add_test_executable(gfalt_test_copy_file_checksum_user "gfalt_test_copy_file_checksum_user.cpp")
        target_link_libraries(gfalt_test_copy_file_checksum_user ${GFAL2_TRANSFER_LINK} ${GFAL2_LIBRARIES} gfal2_test_shared)

        add_test_executable(gfalt_test_copy_bulk "gfalt_test_copy_bulk.cpp")
        target_link_libraries(gfalt_test_copy_bulk ${GFAL2_TRANSFER_LINK} ${GFAL2_LIBRARIES} gfal2_test_shared)

        add_test_executable(gfalt_test_pasv "gfalt_test_pasv.cpp")
        target_link_libraries(gfalt_test_pasv ${GFAL2_TRANSFER_LINK} ${GFAL2_LIBRARIES} gfal2_test_shared)

        function(test_copy_file_full name src_dir dst_dir)
            add_test(copyfile_checksum_${name} gfalt_test_copy_file_checksum ${src_dir} ${dst_dir})
            add_test(copyfile_checksum_user_${name} gfalt_test_copy_file_checksum_user ${src_dir} ${dst_dir})
            add_test(copyfile_replace_${name} gfalt_test_copy_file_replace ${src_dir} ${dst_dir})
            add_test(copyfile_test_${name} gfalt_test_copy_file ${src_dir} ${dst_dir})
            add_test(copyfile_test_mkdir_${name} gfalt_test_copy_file_mkdir ${src_dir} ${dst_dir})
        endfunction(test_copy_file_full name dst_dir)

        function (test_copy_bulk name src_dir dst_dir)
            add_test(copyfile_bulk_${name} gfalt_test_copy_bulk ${src_dir} ${dst_dir})
        endfunction (test_copy_bulk name src_dir dst_dir)

        function(test_copy_file_no_checksum name src_dir dst_dir)
            add_test(copyfile_replace_${name} gfalt_test_copy_file_replace ${src_dir} ${dst_dir})
            add_test(copyfile_test_${name} gfalt_test_copy_file ${src_dir} ${dst_dir})
            add_test(copyfile_test_mkdir_${name} gfalt_test_copy_file_mkdir ${src_dir} ${dst_dir})
        endfunction(test_copy_file_no_checksum name src_dir dst_dir)

        function(test_pasv name src_dir dst_dir)
            add_test(gfalt_test_pasv_${name} gfalt_test_pasv ${src_dir} ${dst_dir})
        endfunction(test_pasv name src_dir dst_dir)

        add_test_executable(gfalt_test_double_cred "gfalt_test_double_cred.cpp")
        target_link_libraries(gfalt_test_double_cred ${GFAL2_TRANSFER_LINK} ${GFAL2_LIBRARIES} gfal2_test_shared)

        function(test_double_cred name src_dir dst_dir)
            add_test(gfalt_test_double_cred_${name} gfalt_test_double_cred ${src_dir} ${dst_dir})
        endfunction(test_double_cred name src_dir dst_dir)

    endif(MAIN_TRANSFER)

    include(functional-test-parameters.cmake)

    include(functional-test-parameters.cmake)

    # Install ctest file
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CTestTestfile.cmake
            DESTINATION ${SHARE_INSTALL_PREFIX}/gfal2/tests)

endif(FUNCTIONAL_TESTS)
