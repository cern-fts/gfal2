cmake_minimum_required (VERSION 2.6)

file (GLOB src_exceptions   "exceptions/*.c*")
file (GLOB src_gsimplecache "gsimplecache/*.c*")
file (GLOB src_uri          "uri/*.c*")
file (GLOB src_checksums    "${CMAKE_CURRENT_SOURCE_DIR}/checksums/*.c*")

LIST(APPEND gfal2_utils_src ${src_exceptions})

LIST(APPEND gfal2_utils_c_src   ${src_gsimplecache}
                                ${src_uri}
                                ${src_checksums})

add_subdirectory (mds)

set(gfal2_mds_src ${gfal2_mds_src} PARENT_SCOPE)
set(gfal2_utils_c_src ${gfal2_utils_c_src} PARENT_SCOPE)
set(gfal2_src_checksum ${src_checksums}    PARENT_SCOPE)

set(mds_cache_link ${mds_cache_link} PARENT_SCOPE)
set(is_ifce_link  ${is_ifce_link} PARENT_SCOPE )
set(gfal2_utils_src ${gfal2_utils_src} PARENT_SCOPE)
set(gfal2_mds_def ${gfal2_mds_def} PARENT_SCOPE)

install (FILES "uri/gfal_uri.h"
         DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/utils)
