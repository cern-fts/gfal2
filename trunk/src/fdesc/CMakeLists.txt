## default set of build

## defaults sources

FILE(GLOB header_fdesc "gfal_file_handle.h")

install(FILES ${header_fdesc} DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/fdesc)

