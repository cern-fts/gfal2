#pragma once
/*

*	auto-generated header file for file test/posix/test__gfal_posix_open.c 
 
*/

void test_mock_srm_open_valid(char** tab, char** tab_turl, int* res);

void test_mock_srm_open_invalid(char** tab, char** tab_exp, int* res);

void test_mock_srm_open_write_valid(char** tab, char** tab_turl, int* res);

void test_mock_guid_open_valid(const char * guid1);
void test_mock_lfc_open_valid(const char* lfc_url);


 void gfal2_test_open_posix_all_simple();
void gfal2_test_open_posix_local_simple();
void gfal2_test_open_posix_lfc_simple();
void gfal2_test_open_posix_srm_simple();
void gfal2_test_open_posix_guid_simple(); 

