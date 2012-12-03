#pragma once
/*

*	auto-generated header file for file test/posix/test__gfal_posix_read.c 
 
*/


void test_mock_read_posix_srm(const char* content);

void test_generic_read_simple(char* url_exist, const char* filename);

void test_generic_pread_simple(char* url_exist, const char* filename);


void gfal2_test_read_posix_local_simple();
void gfal2_test_read_posix_lfc_simple(); 

void gfal2_test_read_posix_srm_simple();

void gfal2_test_read_posix_guid_simple();




void gfal2_test_pread_posix_local_simple();
void gfal2_test_pread_posix_srm_simple();
void gfal2_test_pread_posix_lfc_simple();


void gfal2_test_pread_posix_guid_simple();

