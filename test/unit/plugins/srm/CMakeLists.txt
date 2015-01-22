if (PLUGIN_SRM)

	file (GLOB src_test_srm "*.c*")

    add_executable(unit_test_srm_plugin ${src_test_srm})
	
    target_link_libraries(unit_test_srm_plugin
        ${GFAL2_LIBRARIES} plugin_srm ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} m
    )

	add_dependencies(unit_test_srm_plugin plugin_srm)	
	add_test(test_srm_check_url unit_test_srm_plugin srm_url_check)
	
endif  (PLUGIN_SRM)
