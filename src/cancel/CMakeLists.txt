## default set of build

## defaults sources
FILE(GLOB src_cancel_f "*.c" )
SET(src_cancel ${src_cancel_f} PARENT_SCOPE)

FILE(GLOB header_cancel "gfal_cancel.h")

install(FILES ${header_cancel} DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/cancel)


