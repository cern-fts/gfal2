#pragma once
/*

*	auto-generated header file for file test/common/gfal__test_common_srm.c 
 
*/



void mock_srm_access_error_response(char* surl, int merror);
void mock_srm_access_right_response(char* surl);

void test_srm_mock_chmod(char* url, int retcode);

void gfal2_test_create_srm_handle();
void gfal2_test__gfal_convert_full_surl();
void gfal2_test_gfal_get_hostname_from_surl();
void gfal2_test_gfal_full_endpoint_checkG();
void test_gfal_get_async_1();
void gfal2_test_gfal_get_endpoint_and_setype_from_bdiiG();
void gfal2_test_gfal_select_best_protocol_and_endpointG();

void gfal2_test_gfal_srm_determine_endpoint_full_endpointG();
void gfal2_test_gfal_auto_get_srm_endpoint_full_endpoint_with_no_bdiiG();
void gfal2_test_gfal_srm_determine_endpoint_not_fullG();
void gfal2_test_gfal_srm_getTURLS_one_success();
void gfal2_test_gfal_srm_getTURLS_bad_urls();
void gfal2_test_gfal_srm_getTURLS_pipeline_success();
void gfal2_test_srm_get_checksum();



