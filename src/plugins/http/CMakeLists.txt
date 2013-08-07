cmake_minimum_required (VERSION 2.6)


if (PLUGIN_HTTP)
    include     (CheckLibraryExists)

    pkg_check_modules(DAVIX_PKG REQUIRED "davix>=0.0.20")
    pkg_check_modules(DAVIX_COPY_PKG REQUIRED "davix_copy>=0.0.20")




    # Includes
    include_directories (${CMAKE_CURRENT_BINARY_DIR})
    include_directories(${DAVIX_PKG_INCLUDE_DIRS})
    include_directories(${DAVIX_COPY_PKG_INCLUDE_DIRS})

    # link
    link_directories( ${DAVIX_PKG_LIBRARY_DIRS})
    link_directories( ${DAVIX_COPY_PKG_LIBRARY_DIRS})

    # Build plugin
    file (GLOB src_http "gfal_http_*.cpp")
    add_library (plugin_http SHARED ${src_http})

    target_link_libraries(plugin_http gfal2
                                      gfal2_transfer
                                      ${DAVIX_PKG_LIBRARIES}
                                      ${DAVIX_COPY_PKG_LIBRARIES})

    set_target_properties(plugin_http   PROPERTIES  CLEAN_DIRECT_OUTPUT 1
                                        OUTPUT_NAME "gfal_plugin_http"
                                        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

    # Install
    install(TARGETS plugin_http
            LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR})
    install(FILES "README_PLUGIN_HTTP"
            DESTINATION ${DOC_INSTALL_DIR})    

endif (PLUGIN_HTTP)

