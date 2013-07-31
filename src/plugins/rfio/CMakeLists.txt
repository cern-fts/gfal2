cmake_minimum_required (VERSION 2.6)


if (PLUGIN_RFIO)
    file (GLOB src_rfio "*.c*")

    find_package (DPM REQUIRED)
    include_directories (${DPM_INCLUDE_DIR})
    add_definitions(${DPM_CFLAGS})

    add_library (plugin_rfio SHARED ${src_rfio})

    target_link_libraries (plugin_rfio gfal2 ${DPM_LIBRARIES})

    set_target_properties(plugin_rfio PROPERTIES
                            CLEAN_DIRECT_OUTPUT 1
                            OUTPUT_NAME "gfal_plugin_rfio"
                            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

    install(TARGETS plugin_rfio
            LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR})
    install(FILES "README_PLUGIN_RFIO"
            DESTINATION ${DOC_INSTALL_DIR})

endif (PLUGIN_RFIO)

