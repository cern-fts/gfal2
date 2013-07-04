cmake_minimum_required (VERSION 2.6)


if (PLUGIN_SRM)
    file (GLOB src_srm "*.c*")

    pkg_check_modules(SRM_IFCE_PKG REQUIRED srm-ifce>=1.15.0)
    add_definitions( ${SRM_IFCE_PKG_CFLAGS} )
    include_directories( ${SRM_IFCE_PKG_INCLUDE_DIR})

    add_library(plugin_srm SHARED ${src_srm})
    target_link_libraries(plugin_srm gfal2 gfal2_transfer gfal2_utils gfal2_mds
                                     ${SRM_IFCE_PKG_LIBRARIES} ${is_ifce_link} gomp)

    set_target_properties(plugin_srm PROPERTIES
                            CLEAN_DIRECT_OUTPUT 1
                            OUTPUT_NAME "gfal_plugin_srm"
                            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

    install(TARGETS plugin_srm
            LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR})
    install(FILES "README_PLUGIN_SRM"
            DESTINATION ${DOC_INSTALL_DIR})

endif (PLUGIN_SRM)

