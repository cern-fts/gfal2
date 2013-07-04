## src cmake build script for srm-ifce

## default set of build
SET(UNIT_TESTS FALSE
	CACHE STRING "enable compilation of unit tests")
SET(FUNCTIONAL_TESTS FALSE
	CACHE STRING "functional tests for gfal ")
SET(STRESS_TESTS FALSE
        CACHE STRING "stress tests for gfal ")
	

if(ONLY_TESTS)
	pkg_check_modules(GFAL2_PKG REQUIRED gfal2)
	pkg_check_modules(GFAL2_TRANSFER_PKG REQUIRED gfal_transfer)
	set(GFAL2_LINK ${GFAL2_PKG_LIBRARIES})
	set(GFAL2_TRANSFER_LINK ${GFAL2_TRANSFER_PKG_LIBRARIES})
    include_directories(${GFAL2_PKG_INCLUDE_DIRS})
    include_directories(${GFAL2_TRANSFER_PKG_INCLUDE_DIRS})
else(ONLY_TESTS)
  set(GFAL2_LINK gfal2)
  set(GFAL2_TRANSFER_LINK gfal2_transfer)
endif(ONLY_TESTS)

execute_process(COMMAND cmake -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/conf_test  ${CMAKE_CURRENT_BINARY_DIR}/conf_test) 

# Common methods for the tests
add_subdirectory(common)
include_directories(.)
include_directories(../src/externals)

# Tests
add_subdirectory(functional)
add_subdirectory(stress-test)
add_subdirectory(unit)

# Disabled
# It may be interesting to adapt, so keep them
# add_subdirectory(old-gfal-tests)

