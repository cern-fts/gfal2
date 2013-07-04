#
# unit test for transfer system



IF (UNIT_TESTS )


link_directories (${CMAKE_BINARY_DIR}/plugins)
link_directories (${CMAKE_BINARY_DIR}/src)


IF (PLUGIN_GRIDFTP)
	FILE(GLOB src_test_gridftp "*.c*")

        pkg_check_modules(GLOBUS_GASS_COPY_PKG REQUIRED globus-gass-copy)

        include_directories( "../ " "../libcpp/" ${GLIB2_PKG_INCLUDE_DIRS}
                                                ${GTHREAD2_PKG_INCLUDE_DIRS}  ${GLIBMM_PKG_INCLUDE_DIRS})

        add_definitions( ${GLOBUS_GASS_COPY_PKG_CFLAGS}    ${OpenMP_C_FLAGS})

        add_executable(unit_test_gridftp_plugin_exe	${src_test_gridftp} )
	
        target_link_libraries(unit_test_gridftp_plugin_exe ${GFAL2_LINK} ${GFAL2_TRANSFER_LINK}
                                plugin_gridftp m
                                gtest gtest_main)
	
	add_dependencies(unit_test_gridftp_plugin_exe plugin_gridftp)
	add_test(unit_test_gridftp_plugin unit_test_gridftp_plugin_exe )
	
ENDIF  (PLUGIN_GRIDFTP)

ENDIF (UNIT_TESTS)
