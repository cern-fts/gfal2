
if(UNIT_TESTS)

find_package(GTEST)
include_directories(${PROJECT_SOURCE_DIR}/src ".")

add_subdirectory(plugins/srm)
add_subdirectory(plugins/gridftp)
add_subdirectory(plugins/file)
add_subdirectory(global)
add_subdirectory(cancel)
add_subdirectory(transfer)

endif(UNIT_TESTS)


