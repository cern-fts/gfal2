cmake_minimum_required (VERSION 2.6)

if (PLUGIN_XROOTD)

    find_package(XROOTD REQUIRED)

    include_directories(
        ${XROOTD_INCLUDE_DIR}
        ${JSONC_INCLUDE_DIRS}
        ${UUID_INCLUDE_DIRS}
    )

    file (GLOB src_xrootd "*.cpp")

    add_library (plugin_xrootd MODULE ${src_xrootd})
    add_library (plugin_xrootd_static STATIC ${src_xrootd})

    target_link_libraries (plugin_xrootd
        ${GFAL2_PKG_LIBRARIES}
        gfal2_transfer
        ${XROOTD_LIBRARIES}
        ${JSONC_LIBRARIES}
        ${UUID_LIBRARIES}
    )
    target_link_libraries(plugin_xrootd_static
        ${GFAL2_PKG_LIBRARIES}
        gfal2_transfer
        ${XROOTD_LIBRARIES}
        ${JSONC_LIBRARIES}
        ${UUID_LIBRARIES}
    )

    set_target_properties(plugin_xrootd PROPERTIES
                          CLEAN_DIRECT_OUTPUT 1
                          OUTPUT_NAME "gfal_plugin_xrootd"
                          LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

    install(TARGETS plugin_xrootd
            LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR})

    # install xrootd configuration files
    list (APPEND xrootd_conf_file "${CMAKE_SOURCE_DIR}/dist/etc/gfal2.d/xrootd_plugin.conf")
    install(FILES ${xrootd_conf_file}
            DESTINATION ${SYSCONF_INSTALL_DIR}/gfal2.d/)

    # readme
    install(FILES "README_PLUGIN_XROOTD"
            DESTINATION ${DOC_INSTALL_DIR})

endif ()
