##
## Doxygen macro, allow Doxygen generation from cmake
## do a ""make doc" for the generation


# for old version < 3.0 cmake variables replace config files ( not supported )  ( needed for EPEL 5 support )
# EPYDOC_MODULE_PATH path of the module to configure
# EPYDOC_MODULE_URL project url
# EPYDOC_MODULE_NAME project name
#


macro(addEpydocGeneration EPYDOC_CONFIG_LOCATION)

        IF(NOT EPYDOC_FOUND)
            execute_process(COMMAND epydoc --version
                            OUTPUT_VARIABLE EPYDOC_VERSION_UNPARSED
                            ERROR_VARIABLE EPYDOC_ERROR
                            )
            IF(${EPYDOC_ERROR})
                message(SEND_ERROR "epydoc not found....")

            ELSE(${EPYDOC_ERROR})
                string(REGEX REPLACE ".*version (.*)\n" "\\1" EPYDOC_VERSION ${EPYDOC_VERSION_UNPARSED})
                message(STATUS " epydoc version ..... ${EPYDOC_VERSION} ")
            ENDIF(${EPYDOC_ERROR})


        ENDIF(NOT EPYDOC_FOUND)


        IF(${EPYDOC_VERSION} VERSION_GREATER  "3.0.0")
            configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${EPYDOC_CONFIG_LOCATION} ${CMAKE_CURRENT_BINARY_DIR}/epydoc_config @ONLY)

            add_custom_target(doc
            epydoc --config ${CMAKE_CURRENT_BINARY_DIR}/epydoc_config -v
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generate API documentation with epydoc" VERBATIM
            )
        ELSE(${EPYDOC_VERSION} VERSION_GREATER  "3.0.0") # VERSION TOO OLD, NO CONFIG FILE MANAGEMENT

            add_custom_target(doc
            epydoc --html  -u ${EPYDOC_MODULE_URL} -n ${EPYDOC_MODULE_NAME} -v -o ${CMAKE_CURRENT_BINARY_DIR}/html ${EPYDOC_MODULE_PATH}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generate API documentation with epydoc" VERBATIM
            )
        ENDIF(${EPYDOC_VERSION} VERSION_GREATER  "3.0.0")
endmacro(addEpydocGeneration EPYDOC_CONFIG_LOCATION)
