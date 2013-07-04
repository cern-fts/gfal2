## src cmake build script for srm-ifce

## default set of build
SET(UNIT_TESTS FALSE
	CACHE STRING "enable compilation of unit tests")
SET(FUNCTIONAL_TESTS FALSE
	CACHE STRING "functional tests for gfal ")
SET(LOAD_TESTS FALSE
        CACHE STRING "load tests for gfal ")
	

IF(ONLY_TESTS)
	pkg_check_modules(GFAL2_PKG REQUIRED gfal2)
	pkg_check_modules(GFAL2_TRANSFER_PKG REQUIRED gfal_transfer)
	SET(OUTPUT_NAME_MAIN ${GFAL2_PKG_LIBRARIES})
	SET(OUTPUT_NAME_TRANSFER ${GFAL2_TRANSFER_PKG_LIBRARIES})	
ENDIF(ONLY_TESTS)

#mock src
FILE(GLOB src_mock "mock/*.c")
SET(headers_mock "mock/")

include_directories( ${headers_mock} "." ${PROJECT_SOURCE_DIR}/src/externals ${GFAL2_PKG_INCLUDE_DIRS})

link_directories (${PROJECT_BINARY_DIR}/src /)


execute_process(COMMAND cmake -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/conf_test  ${CMAKE_CURRENT_BINARY_DIR}/conf_test) 

#include common library for tests
add_subdirectory(common)


add_subdirectory(functional/gfal2)   
add_subdirectory(functional/old)   
add_subdirectory(stress-test)
add_subdirectory(unit)
