include_directories(
    "${CMAKE_SOURCE_DIR}/test"
    "${CMAKE_SOURCE_DIR}/src/posix/"
)

add_subdirectory(cancel)
add_subdirectory(config)
add_subdirectory(cred)
add_subdirectory(global)
add_subdirectory(mds)
add_subdirectory(transfer)
add_subdirectory(uri)
