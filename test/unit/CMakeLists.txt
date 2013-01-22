
if(UNIT_TESTS)

include_directories(${PROJECT_SOURCE_DIR}/src ".")

add_subdirectory(common/srm)
add_subdirectory(common/gridftp)
add_subdirectory(common/file)
add_subdirectory(global)
add_subdirectory(transfer)

endif(UNIT_TESTS)


