
/* unit test for posix getxattr func */


#include <cgreen/cgreen.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>


#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_constants.h>
#include <unit_test_util.h>
#include "gfal_posix_api.h"
#include "gfal_posix_internal.h"


#include <common/gfal__test_common_srm.h>
#include <common/gfal__test_plugin.h>
#include <mock/gfal_mds_mock_test.h>
#include <mock/gfal_lfc_mock_test.h>
#include <mock/gfal_srm_mock_test.h>

void gfal2_test_getxattr_guid(const char* good_lfn, const char* guid, const char* enoent_lfn, const char* eaccess_lfn);
void gfal2_test_getxattr_status(const char* good_url, const char* status, const char* enoent_url, const char* eaccess_url);

void init_mock_srm_getxattr(){
	setup_mock_lfc();
	setup_mock_srm();

	
}

void gfal2_test_getxattr_guid_lfn_base(){
	gfal2_test_getxattr_guid(TEST_LFC_OPEN_EXIST, TEST_GUID_ONLY_READ_ACCESS+5, TEST_LFC_OPEN_NOEXIST, TEST_LFC_OPEN_NOACCESS);
}

void gfal2_test_getxattr_guid(const char* good_lfn, const char* guid, const char* enoent_lfn, const char* eaccess_lfn){
	int ret;
	init_mock_srm_getxattr();
	char buffer[2048] = {0};
	
	add_mock_error_lfc_guid_resolution(enoent_lfn, ENOENT);
	
	ret = gfal_getxattr(enoent_lfn, GFAL_XATTR_GUID, buffer, 2048);
	assert_true_with_message(ret < 0 && errno==ENOENT && gfal_posix_code_error()==ENOENT, " must be a enoent %d %d %d",ret,errno,  gfal_posix_code_error() );
	gfal_posix_clear_error();	

	add_mock_error_lfc_guid_resolution(eaccess_lfn, EACCES);
		
	ret = gfal_getxattr(eaccess_lfn, GFAL_XATTR_GUID, buffer, 2048);
	assert_true_with_message(ret < 0 && errno==EACCES && gfal_posix_code_error()==EACCES, " must be a eacces %d %d %d",ret,errno,  gfal_posix_code_error() );
	gfal_posix_clear_error();	
		
	ret = gfal_getxattr(good_lfn, GFAL_XATTR_GUID, NULL, 0);
	assert_true_with_message(ret > 0 && errno==0 && gfal_posix_code_error()==0 
								&& buffer[0] == '\0', " must be an empty buffer %d %d %d", ret, errno, gfal_posix_code_error());	
	gfal_posix_check_error();

	add_mock_valid_lfc_guid_resolution(good_lfn,  guid);
	
	ret = gfal_getxattr(good_lfn, GFAL_XATTR_GUID, buffer, 2048);
	assert_true_with_message(ret > 0 && errno==0 && gfal_posix_code_error()==0 
								, " must be a success %d %d %d ", ret, errno, gfal_posix_code_error());	
	assert_true_with_message(strcmp(buffer, guid) == 0, "invalid value return %s %s", buffer, guid);
	gfal_posix_check_error();
	gfal_posix_clear_error();		
}


void gfal2_test_getxattr_status_srm_base(){
	gfal2_test_getxattr_status(TEST_SRM_OPEN_EXIST, GFAL_XATTR_STATUS_ONLINE, TEST_SRM_OPEN_NOEXIST, TEST_SRM_OPEN_NOACCESS);
}

/**
 * 
 * test for the extended attribute status on a srm file  ( bringsonline )
 */
void gfal2_test_getxattr_status(const char* good_url, const char* status, const char* enoent_url, const char* eaccess_url){
	int ret;
	init_mock_srm_getxattr();
	char buffer[2048] = {0};

	add_mock_srm_ls_error(enoent_url, TEST_SRM_DPM_FULLENDPOINT_URL, ENOENT, " enoent error ");
	
	ret = gfal_getxattr(enoent_url, GFAL_XATTR_STATUS, buffer, 2048);
	assert_true_with_message(ret < 0 && errno==ENOENT && gfal_posix_code_error()==ENOENT, " must be a enoent %d %d %d",ret,errno,  gfal_posix_code_error() );
	gfal_posix_clear_error();	

	add_mock_srm_ls_error(enoent_url, TEST_SRM_DPM_FULLENDPOINT_URL, EACCES, " eacces error ");
			
	ret = gfal_getxattr(eaccess_url, GFAL_XATTR_STATUS, buffer, 2048);
	assert_true_with_message(ret < 0 && errno==EACCES && gfal_posix_code_error()==EACCES, " must be a eacces %d %d %d",ret,errno,  gfal_posix_code_error() );
	gfal_posix_clear_error();	


		
	ret = gfal_getxattr(good_url, GFAL_XATTR_STATUS, NULL, 0);
	assert_true_with_message(ret > 0 && errno==0 && gfal_posix_code_error()==0 
								&& buffer[0] == '\0', " must be an empty buffer %d %d %d", ret, errno, gfal_posix_code_error());	
	gfal_posix_check_error();

	add_mock_srm_ls_locality_valid(good_url, TEST_SRM_DPM_FULLENDPOINT_URL, GFAL_LOCALITY_ONLINE_);
	
	ret = gfal_getxattr(good_url, GFAL_XATTR_STATUS, buffer, 2048);
	assert_true_with_message(ret > 0 && errno==0 && gfal_posix_code_error()==0 
								, " must be a success %d %d %d ", ret, errno, gfal_posix_code_error());	
	assert_true_with_message(strcmp(buffer, status) == 0, "invalid value return %s %s", buffer, status);
	gfal_posix_check_error();
	gfal_posix_clear_error();		
}
