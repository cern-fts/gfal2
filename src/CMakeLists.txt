cmake_minimum_required (VERSION 2.6)

# Shared utilities can be compiled as static, and then
# included at linking time
add_subdirectory (utils)

set(gfal2_utils_src ${gfal2_utils_src} PARENT_SCOPE)
set(gfal2_utils_c_src ${gfal2_utils_c_src} PARENT_SCOPE)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/utils)

# Core library
if (MAIN_CORE)
	add_subdirectory (core)
	# Command line tool to get GFAL2 version
	add_subdirectory (version)
endif (MAIN_CORE)

# Plugins
add_subdirectory (plugins)

