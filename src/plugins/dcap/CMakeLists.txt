cmake_minimum_required (VERSION 2.6)


if (PLUGIN_DCAP)
    file (GLOB src_dcap "*.c*")

    find_package(DCAP REQUIRED)
    include_directories(${DCAP_INCLUDE_DIR})

    add_definitions(-D_REENTRANT)
    add_definitions(${DCAP_CFLAGS})

    add_library(plugin_dcap MODULE ${src_dcap})

    target_link_libraries(plugin_dcap gfal2 ${DCAP_LIBRARIES})

    set_target_properties(plugin_dcap PROPERTIES
                            CLEAN_DIRECT_OUTPUT 1
                            OUTPUT_NAME "gfal_plugin_dcap"
                            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

    install(TARGETS plugin_dcap
            LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR})

    install(FILES  "README_PLUGIN_DCAP"
            DESTINATION ${DOC_INSTALL_DIR})

    # install dcap configuration files
    LIST(APPEND dcap_conf_file "${CMAKE_SOURCE_DIR}/dist/etc/gfal2.d/dcap_plugin.conf")
    install(FILES ${dcap_conf_file}
                        DESTINATION ${SYSCONF_INSTALL_DIR}/gfal2.d/)

endif (PLUGIN_DCAP)

