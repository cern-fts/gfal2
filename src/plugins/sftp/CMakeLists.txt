cmake_minimum_required (VERSION 2.6)

if (PLUGIN_SFTP)
    find_package (LIBSSH2 REQUIRED)

    include_directories(${LIBSSH2_INCLUDE_DIR})

    file (GLOB src_sftp "*.c*")
    add_library (plugin_sftp MODULE ${src_sftp})

    target_link_libraries(plugin_sftp
        gfal2 gfal2_transfer
        ${LIBSSH2_LIBRARIES}
        ${UUID_PKG_LIBRARIES}
    )

    set_target_properties(plugin_sftp PROPERTIES
        CLEAN_DIRECT_OUTPUT 1
        OUTPUT_NAME "gfal_plugin_sftp"
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
    )

    install (TARGETS plugin_sftp
        LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
    )
    install (FILES "README_PLUGIN_SFTP"
        DESTINATION ${DOC_INSTALL_DIR}
    )

    list (APPEND sftp_conf_file "${CMAKE_SOURCE_DIR}/dist/etc/gfal2.d/sftp_plugin.conf")
    install(FILES ${sftp_conf_file}
        DESTINATION ${SYSCONF_INSTALL_DIR}/gfal2.d/
    )
endif (PLUGIN_SFTP)
