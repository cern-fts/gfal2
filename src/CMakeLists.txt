## default set of build
SET(MAIN_CORE TRUE
	CACHE STRING "enable compilation of the main library")

SET(PLUGIN_LFC TRUE
	CACHE STRING "enable compilation of the lfc plugin")
SET(PLUGIN_SRM TRUE
	CACHE STRING "enable compilation of the srm plugin")
SET(PLUGIN_RFIO TRUE
	CACHE STRING "enable compilation of the RFIO plugin")
SET(PLUGIN_DCAP TRUE
	CACHE STRING "enable compilation of the DCAP plugin")

	
SET(IS_IFCE FALSE)
# subdir import
add_subdirectory(global)
add_subdirectory(config)
add_subdirectory(externals/utils)
add_subdirectory(externals/g_config_manager)
add_subdirectory(fdesc)

## defaults sources
FILE(GLOB src_posix "posix/*.c")
FILE(GLOB src_common "common/*.c")
FILE(GLOB src_gsimplecache "externals/gsimplecache/*.c")
#version sources
FILE(GLOB src_version "version/*.c")
#define lfc sources
FILE(GLOB src_lfc "common/lfc/*.c")
#define srm sources
FILE(GLOB src_srm "common/srm/*.c")
#define rfio sources
FILE(GLOB src_rfio "common/rfio/*.c")
#define dcap sources
FILE(GLOB src_dcap "common/dcap/*.c")
#define libcpp source
FILE(GLOB src_libcpp "libcpp/*.cpp")
#define logger source
FILE(GLOB src_logger "logger/*.c")
#define exceptions source
FILE(GLOB src_exceptions "exceptions/*.cpp")

# global headers
SET(common_dir	"common/")
SET(posix_dir	"posix/")
SET(mds_dir	"common/mds/")
SET(global_dir	".")
SET(externals_dir	"externals/")


# gfal2 link and link dir
SET(gfal2_link "gfal2")

LIST(APPEND headers_main ${common_dir} ${posix_dir} ${global_dir} 
			${mds_dir}  ${externals_dir})

include_directories(${headers_main} ${GLIB2_PKG_INCLUDE_DIRS} 
						${GTHREAD2_PKG_INCLUDE_DIRS})
add_definitions( -DVERSION="${VERSION_STRING}" -Wall ${GLIB2_PKG_CFLAGS} ${GTHREAD2_PKG_CFLAGS} -D_LARGEFILE64_SOURCE)

#FILE(GLOB src_program "gfal_srm_ifce_version.c")

#FILE(GLOB src_unit_tests "*unittest*.c")
#FILE(GLOB src_system_tests "*systemtest*.c")


## link to internal bdii system if IS_IFCE=false else to the is_interface
IF (IS_IFCE)

	SET(is_ifce_link	"is_ifce")
	add_definitions(-DMDS_BDII_EXTERNAL=1)
	SET(mds_source "common/mds/gfal_common_mds.c")

ELSE (IS_IFCE)
	add_definitions(-DMDS_BDII_EXTERNAL=0)
	FILE(GLOB mds_source "common/mds/*.c")
	SET(is_ifce_link	"ldap_r")
	
ENDIF(IS_IFCE)

LIST(APPEND main_libs ${src_posix} ${src_common} ${src_global} ${src_config} ${mds_source} ${src_uri} ${src_logger} ${src_g_shared_map})


IF (MAIN_CORE)

	add_library(gfal2  SHARED ${main_libs} )
	target_link_libraries(gfal2 ${GLIB2_PKG_LIBRARIES} ${GTHREAD2_PKG_LIBRARIES} 
							${is_ifce_link} "dl")
	set_target_properties(gfal2	 	PROPERTIES
									VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
									SOVERSION ${VERSION_MAJOR}
									CLEAN_DIRECT_OUTPUT 1
									OUTPUT_NAME ${OUTPUT_NAME_MAIN}
	)
	add_PkgConfigFile_for_Library("gfal2.pc" gfal2 
										HEADER_DIRS "gfal2"
										DESCRIPTION "GFAL 2.0 pkgconfig file"
										REQUIRES "glib-2.0")	
	add_executable(gfal2_version ${src_version})
	target_link_libraries(gfal2_version "gfal2")


	install(TARGETS gfal2 gfal2_version
			RUNTIME       DESTINATION ${BIN_INSTALL_DIR}
			LIBRARY       DESTINATION ${LIB_INSTALL_DIR} )
			
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/gfal2.pc DESTINATION ${PKGCONFIG_FILES_DIR})

	install(FILES "gfal_api.h" "gfal_plugins_api.h"
			DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/ ) # install headers main

	install(FILES "logger/gfal_logger.h"
			DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/logger ) # install headers posix

	install(FILES "posix/gfal_posix_api.h"
			DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/posix ) # install headers posix

	install(FILES   "common/gfal_constants.h"
					"common/gfal_common_plugin_interface.h" 
					"common/gfal_prototypes.h"
					"common/gfal_common_plugin.h" 
			DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/common ) # install headers common

	FILE(GLOB l_examples "../testing/example/*.c")
	install(FILES ${l_examples}
			DESTINATION ${DOC_INSTALL_DIR}/examples/ ) # install examples 
			
	install(FILES 	"global/gfal_global.h"
			DESTINATION ${INCLUDE_INSTALL_DIR}/gfal2/global)

ENDIF (MAIN_CORE)



IF(PLUGIN_LFC)

find_package(LFC REQUIRED)
find_package(DPM REQUIRED)


# lfc headers
SET(lfc_dir "common/lfc/")
SET(lfc_headers_dir "${LFC_INCLUDE}/lfc/")
#lcgdm headers
SET(lcgdm_headers_dir "${DPM_INCLUDE}/lcgdm/")
FILE(GLOB lfc_scripts "../dist/scripts/gfal_plugin_lfc/*sh")

add_definitions( -D_REENTRANT)
add_library(plugin_lfc  SHARED ${src_lfc} ${src_gsimplecache})

target_link_libraries(plugin_lfc ${gfal2_link} ${UUID_PKG_LIBRARIES} ${LFC_LIBRARIES})
include_directories(${LFC_INCLUDE_DIR} ${LCGDM_INCLUDE_DIR} ${UUID_PKG_INCLUDE_DIR})


set_target_properties(plugin_lfc	 	PROPERTIES
								CLEAN_DIRECT_OUTPUT 1
								OUTPUT_NAME ${OUTPUT_NAME_LFC}
								LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)

install(TARGETS		plugin_lfc
	    LIBRARY		DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ${lfc_scripts} DESTINATION ${SYSCONF_INSTALL_DIR}/profile.d/ )

install(FILES  "common/lfc/README_PLUGIN_LFC"
		DESTINATION ${DOC_INSTALL_DIR})	
	    
ENDIF(PLUGIN_LFC)


IF(PLUGIN_SRM)

pkg_check_modules(SRM_IFCE_PKG REQUIRED srm-ifce)

FILE(GLOB srm_scripts "../dist/scripts/gfal_plugin_srm/*sh")
add_library(plugin_srm  SHARED ${src_srm} ${src_gsimplecache} )

target_link_libraries(plugin_srm ${gfal2_link} ${SRM_IFCE_PKG_LIBRARIES} gfal2_transfer)
include_directories( ${SRM_IFCE_PKG_INCLUDE_DIR} )
add_definitions( ${SRM_IFCE_PKG_CFLAGS} )

set_target_properties(plugin_srm	 	PROPERTIES
								CLEAN_DIRECT_OUTPUT 1
								OUTPUT_NAME ${OUTPUT_NAME_SRM}
								LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins								
)

install(TARGETS		plugin_srm
	    LIBRARY		DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES ${srm_scripts} DESTINATION ${SYSCONF_INSTALL_DIR}/profile.d/ )

install(FILES  "common/srm/README_PLUGIN_SRM"
		DESTINATION ${DOC_INSTALL_DIR})	

ENDIF(PLUGIN_SRM)


IF(PLUGIN_RFIO)

find_package(DPM REQUIRED)

FILE(GLOB rfio_scripts "../dist/scripts/gfal_plugin_rfio/*sh")
add_library(plugin_rfio  SHARED ${src_rfio} ${src_gsimplecache} )

target_link_libraries(plugin_rfio ${gfal2_link} ${DPM_LIBRARIES})
include_directories( ${DPM_INCLUDE_DIR} )
add_definitions( ${DPM_CFLAGS} )

set_target_properties(plugin_rfio	 	PROPERTIES
								CLEAN_DIRECT_OUTPUT 1
								OUTPUT_NAME ${OUTPUT_NAME_RFIO}
								LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins								
)

install(TARGETS		plugin_rfio
	    LIBRARY		DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ${rfio_scripts} DESTINATION ${SYSCONF_INSTALL_DIR}/profile.d/ )
  	
install(FILES  "common/rfio/README_PLUGIN_RFIO"
		DESTINATION ${DOC_INSTALL_DIR})	
		
ENDIF(PLUGIN_RFIO)



IF(PLUGIN_DCAP)

find_package(DCAP REQUIRED)
FILE(GLOB dcap_scripts "../dist/scripts/gfal_plugin_dcap/*sh")

add_definitions( -D_REENTRANT)
add_library(plugin_dcap  SHARED ${src_dcap} ${src_gsimplecache} )

target_link_libraries(plugin_dcap ${gfal2_link} ${DCAP_LIBRARIES})
include_directories( ${DCAP_INCLUDE_DIR} )
add_definitions( ${DCAP_CFLAGS} )

set_target_properties(plugin_dcap	 	PROPERTIES
								CLEAN_DIRECT_OUTPUT 1
								OUTPUT_NAME ${OUTPUT_NAME_DCAP}
								LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins								
)

install(TARGETS		plugin_dcap
	    LIBRARY		DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ${dcap_scripts} DESTINATION ${SYSCONF_INSTALL_DIR}/profile.d/ )

install(FILES  "common/dcap/README_PLUGIN_DCAP"
		DESTINATION ${DOC_INSTALL_DIR})	

ENDIF(PLUGIN_DCAP)


add_subdirectory(transfer)                         
add_subdirectory(common/gridftp)    
add_subdirectory(common/srm/test)  
                      





