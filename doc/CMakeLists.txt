
SET(MAIN_DOC TRUE
	CACHE STRING "enable installation of the main library documentation")

IF(MAIN_DOC)

	file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/build/html/")
	addDoxyGeneration("Doxyfile.in")

	install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/build/html/"
			DESTINATION ${DOC_INSTALL_DIR}/html/)

	install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/examples/"
			DESTINATION ${DOC_INSTALL_DIR}/examples)

ENDIF(MAIN_DOC)



install(FILES "man/gfal2_version.1"
		DESTINATION ${MAN_INSTALL_DIR}/man1/) # install examples
