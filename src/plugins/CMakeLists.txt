cmake_minimum_required (VERSION 2.6)

link_directories(${CMAKE_CURRENT_BINARY_DIR}/../core)

add_subdirectory (dcap)
add_subdirectory (file)
add_subdirectory (gridftp)
add_subdirectory (http)
add_subdirectory (lfc)
add_subdirectory (rfio)
add_subdirectory (srm)
add_subdirectory (mock)

