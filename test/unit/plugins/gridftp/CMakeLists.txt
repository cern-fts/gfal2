#
# unit test for transfer system

IF (UNIT_TESTS )


link_directories (${CMAKE_BINARY_DIR}/plugins)
link_directories (${CMAKE_BINARY_DIR}/src)


IF (PLUGIN_GRIDFTP)
        FILE(GLOB src_test_gridftp "*.cpp")

        pkg_check_modules(GLOBUS_GASS_COPY_PKG REQUIRED globus-gass-copy)

        include_directories( "../ " "../libcpp/" ${GLIB2_PKG_INCLUDE_DIRS}
                                                ${GTHREAD2_PKG_INCLUDE_DIRS})

        add_definitions( ${GLOBUS_GASS_COPY_PKG_CFLAGS}    ${OpenMP_C_FLAGS})

        add_executable(unit_test_gridftp_plugin_bin ${src_test_gridftp} )

      target_link_libraries(unit_test_gridftp_plugin_bin pthread gfal2 plugin_gridftp
                                ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} )

	

        add_dependencies(unit_test_gridftp_plugin_bin plugin_gridftp)
        add_test(unit_test_gridftp_plugin unit_test_gridftp_plugin_bin )
	
ENDIF  (PLUGIN_GRIDFTP)

ENDIF (UNIT_TESTS)
