## src cmake build script for srm-ifce

## default set of build

	
## defaults sources
FILE(GLOB src_test_global "*.c")
FILE(GLOB src_test_posix "posix/*.c")
FILE(GLOB src_test_common "common/*.c")
FILE(GLOB src_test_gsimplecache "externals/*.c")
FILE(GLOB src_gskiplist "${CMAKE_SOURCE_DIR}/src/externals/gskiplist/*.c")


#define lfc sources
FILE(GLOB src_test_lfc "common/lfc/*.c")
#define srm sources
FILE(GLOB src_test_srm "common/mds/*.c")
#define rfio sources
FILE(GLOB src_test_rfio "common/rfio/*.c")


LIST(APPEND src_tests_all ${src_test_global} ${src_test_posix} 
				${src_test_common} ${src_test_gsimplecache} ${src_mock} ${src_test_lfc} 
					${src_test_srm} ${src_test_rfio} ${src_gsimplecache} ${src_gskiplist})

# global headers
SET(src_dir	"${CMAKE_SOURCE_DIR}/src/")
SET(common_dir	"${CMAKE_SOURCE_DIR}/src/common/")
SET(posix_dir	"${CMAKE_SOURCE_DIR}/src/posix/")
SET(mds_dir	"${CMAKE_SOURCE_DIR}/src/common/mds/")
SET(global_dir	".")




IF (FUNCTIONAL_TESTS)

	find_package(LFC REQUIRED)
	find_package(DPM REQUIRED)

	include_directories( ${GLIB2_PKG_INCLUDE_DIRS} 
						${GTHREAD2_PKG_INCLUDE_DIRS} ${common_dir} ${posix_dir} ${global_dir} ${src_dir})
						
    #include lfc
 	include_directories(   ${LFC_INCLUDE_DIR} ${LCGDM_INCLUDE_DIR}	)
 	
 	link_directories( "${CMAKE_CURRENT_BINARY_DIR}/../src/" "${CMAKE_BINARY_DIR}/plugins/")	
 						
	add_definitions( -Wall ${GLIB2_PKG_CFLAGS} ${GTHREAD2_PKG_CFLAGS} -D_LARGEFILE64_SOURCE -D_REENTRANT)
	## unit tests
	add_executable(test_all_with_mock	${src_tests_all} )
	target_link_libraries(test_all_with_mock ${OUTPUT_NAME_MAIN} ${CGREEN_LIBRARIES} m )
	
	#target link lfc
	target_link_libraries(test_all_with_mock ${OUTPUT_NAME_LFC})	
	#target link srm
	target_link_libraries(test_all_with_mock ${OUTPUT_NAME_SRM})		
	# target link rfio 
	target_link_libraries(test_all_with_mock ${OUTPUT_NAME_RFIO})
	
	
#env_test.PrependENVPath('LCG_GFAL_INFOSYS', lfc_gfal_infosys); # setup the bdii access point
#env_test.PrependENVPath('LFC_HOST', lfc_host); # setup the bdii access point
#env_test.PrependENVPath('GFAL_PLUGIN_LIST', "libgfal_plugin_lfc.so:libgfal_plugin_srm.so:libgfal_plugin_rfio.so"); # setup the GFAL PLUGIN SEARCH PATH

	add_test(test-all test_all_with_mock)
ENDIF (FUNCTIONAL_TESTS)

