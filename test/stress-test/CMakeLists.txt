FILE(GLOB src_loadtest "gfalt_copyfile_fts_style_load_test.c")

IF (STRESS_TESTS)

        include_directories( "${CMAKE_SOURCE_DIR}/src "  "${CMAKE_SOURCE_DIR}/src/posix/"
                    ${GLIB2_PKG_INCLUDE_DIRS} ${NETTLE_PKG_INCLUDE_DIRS} )

        add_executable(fts_seq_copy_files	${src_loadtest} ${src_lib_test})
        target_link_libraries(fts_seq_copy_files ${OUTPUT_NAME_TRANSFER} ${OUTPUT_NAME_MAIN} )
	
ENDIF  (STRESS_TESTS)

