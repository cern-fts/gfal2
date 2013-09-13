add_library(test_shared SHARED gfal_lib_test.c)

target_link_libraries (test_shared ${GFAL2_TRANSFER_LINK})