
/* unit test for common_srm */


#include <cgreen/cgreen.h>
#include "srm/gfal_common_srm_access.h"
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "gfal_common_internal.h"
#include "common/srm/gfal_common_srm.h"
#include "../unit_test_constants.h"


void test_create_srm_access_check_file()
{
	GError* err=NULL;
	gfal_handle handle = gfal_initG(&err);
	if(handle == NULL){
		assert_true_with_message(FALSE, " handle is not properly allocated");
		return;
	}

	int check = gfal_srm_accessG(handle, TEST_SRM_VALID_SURL_EXAMPLE1, F_OK, &err);
	if(check != 0 || err){
		assert_true_with_message(FALSE, " must be a valid surl \n");
		gfal_release_GError(&err);
		return;
	}
	check = gfal_srm_accessG(handle, TEST_SRM_INVALID_SURL_EXAMPLE2, F_OK, &err);
	if(check ==0  || err->code != ENOENT ){
		assert_true_with_message(FALSE, " must be an invalid surl ");
		gfal_release_GError(&err);
		return;
	}
	g_clear_error(&err);
	gfal_handle_freeG(handle);
}


void test_create_srm_access_read_file()
{
	GError* err=NULL;
	gfal_handle handle = gfal_initG(&err);
	if(handle == NULL){
		assert_true_with_message(FALSE, " handle is not properly allocated");
		return;
	}

	int check = gfal_srm_accessG(handle, TEST_SRM_VALID_SURL_EXAMPLE1, R_OK, &err);
	if(check != 0 || err){
		assert_true_with_message(FALSE, " must be a  readable file \n");
		gfal_release_GError(&err);
		return;
	}
	check = gfal_srm_accessG(handle, TEST_SRM_INVALID_SURL_EXAMPLE2, R_OK, &err);
	if(check == 0  || err->code != ENOENT){
		assert_true_with_message(FALSE, " must be an invalid surl ");
		gfal_release_GError(&err);
		return;
	}
	g_clear_error(&err);
	gfal_handle_freeG(handle);
}
