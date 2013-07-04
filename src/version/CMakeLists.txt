cmake_minimum_required (VERSION 2.6)

add_executable (gfal2_version gfal_version)
target_link_libraries (gfal2_version "gfal2")

install (TARGETS gfal2_version
         RUNTIME DESTINATION ${BIN_INSTALL_DIR})

