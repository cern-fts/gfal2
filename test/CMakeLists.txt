## src cmake build script for srm-ifce

## default set of build
SET(UNIT_TESTS FALSE
	CACHE STRING "enable compilation of unit tests")
SET(FUNCTIONAL_TESTS FALSE
	CACHE STRING "functional tests for gfal ")
	

#mock src
FILE(GLOB src_mock "mock/*.c")
SET(headers_mock "mock/")

include_directories( ${headers_mock} ".")

link_directories (${PROJECT_BINARY_DIR}/src) 



add_subdirectory(functional/gfal2)   
add_subdirectory(functional/old)   
