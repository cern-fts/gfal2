
/* unit test for posixrmdir func */


#include <cgreen/cgreen.h>
#include <time.h>
#include <string.h>
#include <glib.h>
#include <stdio.h>
#include <errno.h>

#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_constants.h>
#include <posix/gfal_posix_internal.h>
#include <posix/gfal_posix_api.h>



#include "../unit_test_constants.h"
#include "../common/gfal__test_plugin.h"
#include "../mock/gfal_mds_mock_test.h"
#include "../mock/gfal_lfc_mock_test.h"
#include "../mock/gfal_srm_mock_test.h"
#include "../common/gfal__test_common_srm.h"
#include "test__gfal_posix_mkdir.h"
#include "test__gfal_posix_rmdir.h"

void test_mock_rmdir_lfc(int errcode, char* url, mode_t mode){
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	if( gfal_check_GError(&mock_err))
		return;

	always_return(lfc_mock_endtrans,0);
	always_return(lfc_mock_starttrans,0);
	will_respond(lfc_mock_rmdir, errcode, want_string(path, url+4));
#endif	
	
}


void create_srm_rmdir_mock(const char* url, int code){
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	setup_mock_srm();
	if( gfal_check_GError(&mock_err))
		return;
	define_mock_endpoints(TEST_SRM_DPM_FULLENDPOINT_URL);
	will_respond(mds_mock_sd_get_se_types_and_endpoints, 0, want_string(host, TEST_SRM_DPM_CORE_URL), want_non_null(se_types), want_non_null(se_endpoints));
	will_respond(srm_mock_srm_context_init, 0, want_non_null(context), want_string(srm_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL));
	define_mock_defined_srm_rmdir_output((char*)url, code);
	will_respond(srm_mock_srm_rmdir, 0, want_non_null(context), want_non_null(input), want_non_null(output));	

#endif		
	
	
}


void test_generic_rmdir_enoent(char* existingfile){
	int ret = gfal_rmdir(existingfile);
	assert_true_with_message(ret != 0 && gfal_posix_code_error() == ENOENT, " must be an invalid deletion, this dir is not existing : %d ", gfal_posix_code_error());
	gfal_posix_clear_error();	
	
}



void test_generic_rmdir_normal(char* existingfile){
	gfal_posix_clear_error();
	int ret = gfal_mkdir(existingfile,0664);
	assert_true_with_message(ret ==0 || gfal_posix_code_error() == EEXIST, " must be a valid dir creation %d %d", ret, gfal_posix_code_error());

	
	ret = gfal_rmdir(existingfile);
	assert_true_with_message(ret ==0 || gfal_posix_code_error() == 0, " must be a valid dir deletion %d %d", ret, gfal_posix_code_error());	
}


void test_generic_rmdir(char* existingfile, char* nonemptydir){
	int ret = -1;
	
	ret = gfal_rmdir(NULL);
	assert_true_with_message(ret != 0 && gfal_posix_code_error() == EFAULT, " must be a NULL path %d", gfal_posix_code_error());


	test_generic_rmdir_normal(existingfile);

	test_generic_rmdir_enoent(existingfile)	;
	
	
	ret = gfal_rmdir(nonemptydir);
	assert_true_with_message(ret != 0 && gfal_posix_code_error() == ENOTEMPTY, " must be an invalid deletion, this dir is not empty : %d ", gfal_posix_code_error());
	gfal_posix_clear_error();		
	
}

void test__rmdir_posix_lfc_simple()
{
	test_mock_mkdir_lfc(0, TEST_LFC_RMDIR_CREATED, 0664);
	test_mock_rmdir_lfc(0, TEST_LFC_RMDIR_CREATED, 0664);
	test_mock_rmdir_lfc(-ENOENT, TEST_LFC_RMDIR_CREATED, 0664);
	test_mock_rmdir_lfc(-ENOTEMPTY, TEST_LFC_RMDIR_EEXIST, 0664);
	test_generic_rmdir(TEST_LFC_RMDIR_CREATED, TEST_LFC_RMDIR_EEXIST);	
	
}





void test__rmdir_posix_lfc_slash()
{
	test_mock_rmdir_lfc(0, TEST_LFC_RMDIR_CREATED, 0664);
	test_mock_rmdir_lfc(-ENOENT, TEST_LFC_RMDIR_CREATED, 0664);
	test_mock_rmdir_lfc(-ENOTEMPTY, TEST_LFC_RMDIR_EEXIST, 0664);
	test_generic_rmdir(TEST_LFC_RMDIR_CREATED_SLASH, TEST_LFC_RMDIR_EEXIST);		
}






// srm part

void test__rmdir_posix_srm_simple()
{
	create_srm_mkdir_mock(TEST_SRM_RMDIR_CREATED, 0);
	create_srm_rmdir_mock(TEST_SRM_RMDIR_CREATED, 0);
	test_generic_rmdir_normal(TEST_SRM_RMDIR_CREATED);
	create_srm_rmdir_mock(TEST_SRM_RMDIR_CREATED, ENOENT);
	test_generic_rmdir_enoent(TEST_SRM_RMDIR_CREATED);
	//test_generic_rmdir(TEST_SRM_RMDIR_CREATED, TEST_SRM_RMDIR_EEXIST);	
}






// local part

void test__rmdir_posix_local_simple()
{
	system(TEST_LOCAL_RMDIR_EEXIST_COMMAND);
	test_generic_rmdir(TEST_LOCAL_RMDIR_CREATED, TEST_LOCAL_RMDIR_EEXIST);

}






void test__rmdir_posix_local_slash()
{

	system(TEST_LOCAL_RMDIR_EEXIST_COMMAND);
	test_generic_rmdir(TEST_LOCAL_RMDIR_CREATED_SLASH, TEST_LOCAL_RMDIR_EEXIST);	
}
