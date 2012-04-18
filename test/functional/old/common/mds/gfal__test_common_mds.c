

/* unit test for common_mds*/


#include <cgreen/cgreen.h>
#include <glib.h>
#include <string.h>
#include <common/mds/gfal_common_mds.h>
#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_constants.h>
#include <common/gfal_common_internal.h>
#include <common/gfal_common_errverbose.h>
#include <posix/gfal_posix_api.h>
#include <posix/gfal_posix_internal.h>
#include "../../unit_test_constants.h"
#include "../../mock/gfal_mds_mock_test.h" 



void gfal2_test_check_bdii_endpoints_srm()
{

	char **se_types=NULL;
	char **se_endpoints=NULL;

	GError * err=NULL;
	int ret=-1;
	char* endpoints[] = { TEST_MDS_VALID_ENDPOINT_URL,
						NULL };
	char** ptr = endpoints;
	
#if USE_MOCK

#endif	
	
	while(*ptr != NULL){
		se_types=NULL;
		se_endpoints=NULL;
		ret = gfal_mds_get_se_types_and_endpoints (*ptr, &se_types, &se_endpoints, &err);
		assert_true_with_message(ret == 0, " ret of bdii with error %d %d", ret, errno);
		assert_true_with_message(ret == 0 && 
				((strings_are_equal(*se_types, "srm_v1") && strstr(*se_endpoints, TEST_MDS_VALID_ENDPOINT_RESU_1) != NULL) ||
				(strings_are_equal(*(se_types+1), "srm_v1") && strstr(*(se_endpoints+1), TEST_MDS_VALID_ENDPOINT_RESU_1) != NULL)) 	, " check if ountain first value ");
		g_strfreev(se_types);
		g_strfreev(se_endpoints);
		g_clear_error(&err);
		ptr++;
	}

	se_types=NULL;
	se_endpoints=NULL;	
	ret = gfal_mds_get_se_types_and_endpoints (TEST_MDS_INVALID_ENDPOINT_URL, &se_types, &se_endpoints, &err);		
	assert_true_with_message(ret != 0 &&  err->code == ENXIO , "must fail, invalid url");
	g_clear_error(&err);
	g_strfreev(se_types);
	g_strfreev(se_endpoints);
}


void gfal2_gfal2_test_check_bdii_endpoints_srm_ng()
{
	gfal_mds_endpoint tabendpoint[100];

	GError * err=NULL;
	int ret=-1;
	int i;
	char* endpoints[] = { TEST_MDS_VALID_ENDPOINT_URL,
						NULL };
	char** ptr = endpoints;
	
	while(*ptr != NULL){
		
		ret = gfal_mds_resolve_srm_endpoint(*ptr, tabendpoint, 100, &err);
		assert_true_with_message(ret > 0, " ret of bdii with error %d %d ", ret, errno);
		if(err)
			gfal_release_GError(&err);
		else{
			gboolean countain = FALSE;
			for(i =0; i < ret; ++i){
				countain = (strstr(tabendpoint[i].url, TEST_MDS_VALID_ENDPOINT_RESU_2) != NULL)?TRUE:countain;
			}
			assert_true_with_message(countain, " must countain the endpoint ");
		}		
		ptr++;
	}

	ret = gfal_mds_resolve_srm_endpoint (TEST_MDS_INVALID_ENDPOINT_URL, tabendpoint, 100, &err);	
	assert_true_with_message(ret < 0 &&  err->code == ENXIO , "must fail, invalid url");
	g_clear_error(&err);
}



void gfal__test_get_lfchost_bdii()
{
	GError* tmp_err =NULL;
	errno = 0;
	gfal_handle handle = gfal_initG(&tmp_err);
	assert_true_with_message(handle !=NULL && tmp_err == NULL, "Error while init handle");
	if(tmp_err)
		return;		
	
#if USE_MOCK

#endif
	char* lfc = gfal_get_lfchost_bdii(handle, &tmp_err);
	assert_true_with_message(lfc!=NULL && strings_are_equal(lfc, "avalid.lfc.value.fr"), "must return correct lfc value");
	gfal_check_GError(&tmp_err);
	lfc = gfal_get_lfchost_bdii(handle, &tmp_err);
	assert_true_with_message(lfc== NULL, "must return correct lfc value");
	g_clear_error(&tmp_err);
	free(lfc);
	gfal_handle_freeG(handle);	
}



void gfal__test_get_lfchost_bdii_with_nobdii()
{
	GError* tmp_err =NULL;
	errno = 0;
	gfal_handle handle = gfal_initG(&tmp_err);
	assert_true_with_message(tmp_err==NULL, "Error while init handle");	
	if(tmp_err)
		return;
	gfal_set_nobdiiG(handle, TRUE);	
#if USE_MOCK

#endif

	char* lfc = gfal_get_lfchost_bdii(handle, &tmp_err); // No bdii connected
	assert_true_with_message(lfc == NULL && tmp_err != NULL && tmp_err->code== EPROTONOSUPPORT, " must return an error, nobdii option checked");

	//g_printerr(" lfc name : %s ", lfc);
	free(lfc);	
	g_clear_error(&tmp_err);
	gfal_handle_freeG(handle);
}
