cmake_minimum_required (VERSION 2.6)


if (PLUGIN_LFC)
    find_package (LFC REQUIRED)
    add_definitions (-D_REENTRANT)

    include_directories(${LFC_INCLUDE_DIR} ${LCGDM_INCLUDE_DIR} ${UUID_PKG_INCLUDE_DIR})

    file (GLOB src_lfc "*.c*")
    add_library (plugin_lfc SHARED ${src_lfc})

    target_link_libraries(plugin_lfc gfal2 gfal2_utils gfal2_mds ${LFC_LIBRARIES} ${UUID_PKG_LIBRARIES})

    set_target_properties(plugin_lfc PROPERTIES
                            CLEAN_DIRECT_OUTPUT 1
                            OUTPUT_NAME "gfal_plugin_lfc"
                            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

    install (TARGETS plugin_lfc
             LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR})
    install (FILES "README_PLUGIN_LFC"
             DESTINATION ${DOC_INSTALL_DIR})

endif (PLUGIN_LFC)
