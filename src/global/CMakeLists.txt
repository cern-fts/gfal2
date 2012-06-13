## default set of build

## defaults sources
FILE(GLOB src_global_f "*.c" )
SET(src_global ${src_global_f} PARENT_SCOPE)

FILE(GLOB header_global "gfal_global.h")

install(FILES ${header_global} DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/global)







add_subdirectory (test)
