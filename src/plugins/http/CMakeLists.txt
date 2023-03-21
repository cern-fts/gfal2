cmake_minimum_required (VERSION 2.6)


if (PLUGIN_HTTP)
    include     (CheckLibraryExists)

    find_package(Davix REQUIRED)
    find_package(Cryptopp REQUIRED)

    # Includes
    include_directories (${CMAKE_CURRENT_BINARY_DIR})
    include_directories(${DAVIX_INCLUDE_DIR})
    include_directories(${JSONC_INCLUDE_DIRS})
    include_directories(${CRYPTOPP_INCLUDE_DIRS})

    # definitions
    add_definitions (${DAVIX_CFLAGS})

    # Build plugin
    file (GLOB src_http "gfal_http_*.cpp")
    add_library (plugin_http MODULE ${src_http})
    add_library (plugin_http_static STATIC ${src_http})

    target_link_libraries(plugin_http gfal2
          gfal2_transfer
          ${DAVIX_LIBRARIES}
          ${JSONC_LIBRARIES}
          ${CRYPTOPP_LIBRARIES}
    )

    target_link_libraries(plugin_http_static gfal2
            gfal2_transfer
            ${DAVIX_LIBRARIES}
            ${JSONC_LIBRARIES}
            ${CRYPTOPP_LIBRARIES}
            )

    set_target_properties(plugin_http   PROPERTIES  CLEAN_DIRECT_OUTPUT 1
                                        OUTPUT_NAME "gfal_plugin_http"
                                        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

    # Install
    install(TARGETS plugin_http
            LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR})
    install(FILES "README_PLUGIN_HTTP"
            DESTINATION ${DOC_INSTALL_DIR})

    # install http configuration files
    LIST(APPEND http_conf_file "${CMAKE_SOURCE_DIR}/dist/etc/gfal2.d/http_plugin.conf")
    install(FILES ${http_conf_file}
                        DESTINATION ${SYSCONF_INSTALL_DIR}/gfal2.d/)

endif (PLUGIN_HTTP)
