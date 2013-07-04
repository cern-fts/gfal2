#pragma once
/*

*	auto-generated header file for file test/posix/test__gfal_posix_mkdir.c 
 
*/

void test_mock_mkdir_lfc(int errcode, char* url, mode_t mode);
void create_srm_mkdir_mock(const char* url, int code);

void gfal2_test__mkdir_posix_lfc_simple();
void test__mkdir_posix_lfc_rec();
void test__mkdir_posix_lfc_rec_with_slash();
void gfal2_test__mkdir_posix_local_simple();
void test__mkdir_posix_local_rec();
void test__mkdir_posix_local_rec_with_slash();
void gfal2_test__mkdir_posix_srm_simple();
void test__mkdir_posix_srm_rec();
void test__mkdir_posix_srm_rec_with_slash(); 

