cmake_minimum_required (VERSION 2.6)

# Shared utilities can be compiled as static, and then
# included at linking time
add_subdirectory (utils)

# Core library
if (MAIN_CORE)
	add_subdirectory (core)
	# Command line tool to get GFAL2 version
	add_subdirectory (version)
endif (MAIN_CORE)

# Plugins
add_subdirectory (plugins)

