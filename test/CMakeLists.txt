
set (UNIT_TESTS       FALSE CACHE STRING "enable compilation of unit tests")
set (FUNCTIONAL_TESTS FALSE CACHE STRING "functional tests for gfal ")
set (STRESS_TESTS     FALSE CACHE STRING "stress tests for gfal ")

include_directories (${CMAKE_SOURCE_DIR}/src)

if (ONLY_TESTS)
    find_package (GFAL2 REQUIRED)
    include_directories (${GFAL2_INCLUDE_DIRS})
else (ONLY_TESTS)
    set(GFAL2_LIBRARIES gfal2 gfal2_transfer)
endif (ONLY_TESTS)

find_package(GTEST)
include_directories(${GTEST_INCLUDE_DIR})

execute_process(COMMAND cmake -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/conf_test  ${CMAKE_CURRENT_BINARY_DIR}/conf_test)

# Common methods for the tests
add_subdirectory(common)

# Tests
if (UNIT_TESTS)
    add_subdirectory(unit)
endif (UNIT_TESTS)

if (FUNCTIONAL_TESTS)
    add_subdirectory(functional)
endif (FUNCTIONAL_TESTS)

if (STRESS_TESTS)
    add_subdirectory(stress-test)
endif (STRESS_TESTS)
