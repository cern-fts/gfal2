cmake_minimum_required (VERSION 2.6)

if (MAIN_TRANSFER)
        file (GLOB src_trans "*.cpp" )
        list (APPEND header_transfer "gfal_transfer.h" "gfal_transfer_types.h" "gfal_transfer_plugins.h")

        add_definitions( ${GLIB2_PKG_CFLAGS} ${GTHREAD2_PKG_CFLAGS} ${GLIBMM_PKG_CFLAGS})

        add_library(gfal2_transfer  SHARED ${src_trans} ${src_libcpp} ${src_exceptions} )
        target_link_libraries(gfal2_transfer ${GLIB2_PKG_LIBRARIES} ${GTHREAD2_PKG_LIBRARIES})
        target_link_libraries(gfal2_transfer ${UUID_PKG_LIBRARIES} ${OUTPUT_NAME_MAIN} ${GLIBMM_PKG_LIBRARIES})
        target_link_libraries(gfal2_transfer gfal2_utils)
        target_link_libraries("dl")

        set_target_properties(gfal2_transfer PROPERTIES
                                     LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/src/core
                                     VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
                                     SOVERSION ${VERSION_MAJOR}
                                     CLEAN_DIRECT_OUTPUT 1
                                     OUTPUT_NAME "gfal_transfer")
        add_PkgConfigFile_for_Library("gfal_transfer.pc" gfal2_transfer 
                                       HEADER_DIRS "gfal2"
                                       DESCRIPTION "gfal_transfer pkgconfig file"
                                       REQUIRES "gfal2")

        install(TARGETS gfal2_transfer
        LIBRARY DESTINATION ${LIB_INSTALL_DIR} )
            
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/gfal_transfer.pc DESTINATION ${PKGCONFIG_FILES_DIR})
        install(FILES ${header_transfer} DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/transfer)
        install(FILES "README_TRANSFER"
                DESTINATION ${DOC_INSTALL_DIR})

endif (MAIN_TRANSFER)

