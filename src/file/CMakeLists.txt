## default set of build

## defaults sources
FILE(GLOB src_file "*.c" )
SET(src_file ${src_file} PARENT_SCOPE)

FILE(GLOB header_file_api "gfal_file_api.h")

install(FILES ${header_file_api} DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/file)


