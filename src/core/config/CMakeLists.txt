## default set of build

## defaults sources
FILE(GLOB src_config_f "*.c" )
SET(src_config ${src_config_f} PARENT_SCOPE)

FILE(GLOB header_config "gfal_config.h")

install(FILES ${header_config} DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/config)







