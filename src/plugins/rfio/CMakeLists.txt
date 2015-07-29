cmake_minimum_required (VERSION 2.6)


if (PLUGIN_RFIO)
    file (GLOB src_rfio "*.c*")

    find_package (DPM REQUIRED)
    include_directories (${DPM_INCLUDE_DIR})
    add_definitions(${DPM_CFLAGS})

    add_library (plugin_rfio MODULE ${src_rfio})

    target_link_libraries (plugin_rfio gfal2 ${DPM_LIBRARIES})

    set_target_properties(plugin_rfio PROPERTIES
                            CLEAN_DIRECT_OUTPUT 1
                            OUTPUT_NAME "gfal_plugin_rfio"
                            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

    install(TARGETS plugin_rfio
            LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR})
    install(FILES "README_PLUGIN_RFIO"
            DESTINATION ${DOC_INSTALL_DIR})


    # install rfio configuration files
    LIST(APPEND rfio_conf_file "${CMAKE_SOURCE_DIR}/dist/etc/gfal2.d/rfio_plugin.conf")
    install(FILES ${rfio_conf_file}
                        DESTINATION ${SYSCONF_INSTALL_DIR}/gfal2.d/)

endif (PLUGIN_RFIO)

