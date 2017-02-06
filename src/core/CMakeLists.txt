cmake_minimum_required (VERSION 2.6)

# Sources needed for the core library
file (GLOB src_cancel     "cancel/*.c*")
file (GLOB src_common     "common/*.c" "future/*.c")
file (GLOB src_config     "config/*.c")
file (GLOB src_exceptions "exceptions/*.c*")
file (GLOB src_gconfig    "g_config_manager/*.c*")
file (GLOB src_file       "file/*.c*")
file (GLOB src_global     "global/*c*")
file (GLOB src_logger     "logger/*.c*")
file (GLOB src_posix      "posix/*.c*")

# Core library
add_definitions(-D__GFAL2_BUILD__)

include_directories(${gfal2_utils_includes})

add_definitions (${gfal2_utils_definitions})
add_library (gfal2 SHARED ${src_cancel}
                          ${src_common}
                          ${src_config}
                          ${src_gconfig}
                          ${src_file}
                          ${src_global}
                          ${src_logger}
                          ${src_posix}
                          ${src_exceptions}
                          ${gfal2_utils_c_src})

target_link_libraries (gfal2 "stdc++" ) # hotfix for libc TLS init bug in EPEL 5
target_link_libraries (gfal2 ${GLIB2_PKG_LIBRARIES} ${GTHREAD2_PKG_LIBRARIES})
target_link_libraries (gfal2 "dl")
target_link_libraries (gfal2 ${gfal2_utils_libraries})

set_target_properties (gfal2 PROPERTIES
                             VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
                             SOVERSION ${VERSION_MAJOR}
                             CLEAN_DIRECT_OUTPUT 1
                             OUTPUT_NAME ${OUTPUT_NAME_MAIN})

# pkgconfig for gfal2
add_PkgConfigFile_for_Library ("gfal2.pc" gfal2
                                        HEADER_DIRS "gfal2"
                                        CFLAGS " -D_FILE_OFFSET_BITS=64 "
                                        DESCRIPTION "GFAL 2.0 pkgconfig file"
                                        REQUIRES "glib-2.0")

# Install core library
install (TARGETS gfal2 LIBRARY DESTINATION ${LIB_INSTALL_DIR})
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/gfal2.pc DESTINATION ${PKGCONFIG_FILES_DIR})

# Install development headers
install (FILES "gfal_api.h" "gfal_plugins_api.h"
         DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/)
install (FILES "common/gfal_cancel.h"
         DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/cancel)
install (FILES "logger/gfal_logger.h"
         DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/logger)
install (FILES "posix/gfal_posix_api.h"
         DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/posix)
install (FILES "common/gfal_constants.h"
               "common/gfal_common_plugin_interface.h"
               "common/gfal_prototypes.h"
               "common/gfal_common_plugin.h"
               "common/gfal_types.h"
               "common/gfal_deprecated.h"
               "common/gfal_common_err_helpers.h"
         DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/common)
install (FILES "common/gfal_common.h"
         DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/global)
install (FILES "config/gfal_config.h"
         DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/config)
install (FILES "g_config_manager/g_config_manager.h"
         DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/g_config_manager)
install (FILES "file/gfal_file_api.h"
         DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/file)
install (FILES "file/gfal_file_handle.h"
         DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/fdesc)

# Transfer library
if (MAIN_TRANSFER)
    add_subdirectory (transfer)
endif (MAIN_TRANSFER)

