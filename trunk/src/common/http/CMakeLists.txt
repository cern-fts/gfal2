cmake_minimum_required (VERSION 2.6)

include     (CheckLibraryExists)

FILE(GLOB src_http "*.cpp")

pkg_check_modules(GSOAP_SSL_PKG REQUIRED "gsoapssl++")
find_package(gSOAP REQUIRED)

message(STATUS "Using gsoapssl flags ${GSOAP_SSL_PKG_CFLAGS}")

# Defaults
if (NOT DEFINED DELEGATION_WSDL)
  set (DELEGATION_WSDL "${CMAKE_CURRENT_SOURCE_DIR}/delegation-1.1.0.wsdl")
endif (NOT DEFINED DELEGATION_WSDL)

# Compile the WSDL
add_custom_command (OUTPUT typemap.dat
                    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/typemap.dat ${CMAKE_CURRENT_BINARY_DIR}/typemap.dat)
add_custom_command (OUTPUT delegation.h
                    COMMAND ${GSOAP_WSDL2H} -I/usr/share/gsoap/import/ -o${CMAKE_CURRENT_BINARY_DIR}/delegation.h ${DELEGATION_WSDL}
                    DEPENDS typemap.dat)
add_custom_command (OUTPUT soapC.cpp soapClient.cpp
                    COMMAND ${GSOAP_SOAPCPP2} -I/usr/share/gsoap/import/ ${CMAKE_CURRENT_BINARY_DIR}/delegation.h
                    DEPENDS delegation.h)

# Includes
include_directories (${CMAKE_CURRENT_BINARY_DIR} ${GRIDSITE_INCLUDE_DIR}
                     ${GSOAP_INCLUDE_DIR})

include_directories(/usr/include/davix)

add_definitions(${GSOAP_SSL_PKG_CFLAGS})

add_library     (plugin_http SHARED ${src_http}
                                    ${src_gsimplecache}
                                    ${CMAKE_CURRENT_BINARY_DIR}/soapC.cpp
                                    ${CMAKE_CURRENT_BINARY_DIR}/soapClient.cpp)

target_link_libraries(plugin_http ${gfal2_link} davix gsoapssl++ gridsite)


set_target_properties(plugin_http   PROPERTIES  CLEAN_DIRECT_OUTPUT 1
                                                OUTPUT_NAME ${OUTPUT_NAME_HTTP}
                                                LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)

install(TARGETS plugin_http
            LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES "README_PLUGIN_HTTP"
            DESTINATION ${DOC_INSTALL_DIR})
