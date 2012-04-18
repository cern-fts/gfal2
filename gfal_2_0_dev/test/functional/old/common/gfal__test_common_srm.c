
/* unit test for common_srm */


#include <cgreen/cgreen.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include <time.h> 

#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_common_plugin.h>


#include <common/gfal_common_internal.h>
#include <common/srm/gfal_common_srm_internal_layer.h>
#include <common/srm/gfal_common_srm_endpoint.h>
#include <common/srm/gfal_common_srm.h>
#include <common/srm/gfal_common_srm_checksum.h>
#include <common/mds/gfal_common_mds.h>
#include <posix/gfal_posix_internal.h>



#include "gfal__test_plugin.h"
#include "../unit_test_constants.h"
#include "../mock/gfal_mds_mock_test.h"
#include "../mock/gfal_srm_mock_test.h"
#include "gfal__test_common_srm.h"






void mock_srm_access_right_response(char* surl){
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	setup_mock_srm();
	if( gfal_check_GError(&mock_err))
		return;
	gfal_plugins_instance(handle, NULL);
	char* surls[] = { surl, NULL };
	char* turls[] = { "nawak", NULL };
	int status[] = { 0, 0 };

	define_mock_srmv2_filestatus(1, surls, NULL,  turls, status);
	define_mock_endpoints(TEST_SRM_DPM_FULLENDPOINT_URL);
	will_respond(srm_mock_srm_context_init, 0, want_non_null(context), want_string(srm_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL));
	will_respond(srm_mock_srm_check_permission, 1, want_non_null(context), want_non_null(statuses), want_non_null(input));		
#endif
}

void mock_srm_access_error_response(char* surl, int merror){
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	setup_mock_srm();
	if( gfal_check_GError(&mock_err))
		return;	
	char* explanation2[] = { "enoent mock", NULL };
	int status2[] = { merror, 0 };
	char* surls[] = { surl, NULL };	
	define_mock_srmv2_filestatus(1, surls, explanation2, NULL, status2);
	define_mock_endpoints(TEST_SRM_DPM_FULLENDPOINT_URL);
	will_respond(srm_mock_srm_context_init, 0, want_non_null(context), want_string(srm_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL));
	will_respond(srm_mock_srm_check_permission, 1, want_non_null(context), want_non_null(statuses), want_non_null(input));	
#endif
}



void test_srm_mock_chmod(char* url, int retcode){
#if USE_MOCK
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
	
	setup_mock_srm();
	define_mock_endpoints(TEST_SRM_DPM_FULLENDPOINT_URL);

	will_respond(srm_mock_srm_context_init, 0, want_non_null(context), want_string(srm_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL));
	will_respond(srm_mock_srm_setpermission, retcode, want_non_null(context), want_non_null(input));		
#endif	
	
}





void gfal2_test_create_srm_handle()
{
	GError* err=NULL;
	gfal_handle handle = gfal_initG(&err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");
	assert_true_with_message(err==NULL, " error must be NULL");
	gfal_handle_freeG(handle);
}

void gfal2_test__gfal_convert_full_surl()
{
	int res = -1;
	char buff[2048];
	GError* tmp_err=NULL;
	res=  gfal_get_fullendpointG(TEST_SRM_DPM_FULLENDPOINT_PREFIX, buff, 2048, &tmp_err);
	assert_true_with_message( res == 0 && strings_are_equal(buff, TEST_SRM_DPM_FULLENDPOINT_URL) && tmp_err==NULL, " must be a successfull endpoint convertion");
	g_clear_error(&tmp_err);
	res=  gfal_get_fullendpointG(TEST_SRM_DPM_FULLENDPOINT_PREFIX, buff, 2, &tmp_err);	
	assert_true_with_message( res != 0 && tmp_err!=NULL && tmp_err->code == ENOBUFS, " must be a buffer to small");	
	g_clear_error(&tmp_err);
}

/*
void test_gfal_get_async_1()
{
	GError* err=NULL;
	gfal_handle handle = gfal_initG(&err);
	if(handle == NULL){
			gfal_release_GError(&err);
			assert_true_with_message(FALSE, " handle fail to initiated");
	}
	GList* list = g_list_append(NULL,"srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/testfile002");
	int r = gfal_get_asyncG(handle,list,&err);
	if(r <0){
			gfal_release_GError(&err);
			assert_true_with_message(FALSE, "must be a success");
			return;
	}	
	gfal_handle_freeG(handle);
}
*/





void gfal2_test_gfal_full_endpoint_checkG()
{
	GError* err = NULL;
	assert_true_with_message( gfal_check_fullendpoint_in_surlG( "srm://srm-pps:8443/srm/managerv2?SFN=/castor/cern.ch/grid/dteam/castordev/test-srm-pps_8443-srm2_d0t1-ed6b7013-5329-4f5b-aaba-0e1341f30663",&err) ," fail, must be a success" );
	gfal_check_GError(&err);
	assert_true_with_message( gfal_check_fullendpoint_in_surlG("srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it", &err) == FALSE, " fail, must be a failure : bad url");
	gfal_check_GError(&err);
	assert_true_with_message( gfal_check_fullendpoint_in_surlG( "srm://lxb5409.cern.ch:8446/srm/managerv2?SFN=/dpm/cern.ch/home/dteam/srmv2_tests/test_lfc_3897",&err) ," fail, must be a success" );
	gfal_check_GError(&err);
}
	
void gfal2_test_gfal_get_endpoint_and_setype_from_bdiiG(){
	GError* err= NULL;
	int i1;
	char buff_endpoint[2048];
	memset(buff_endpoint, '\0', sizeof(char)*2048);
	enum gfal_srm_proto proto;
	
#if USE_MOCK
	setup_mock_bdii();
	char buff_tmp[2048];
	char* p = TEST_SRM_DPM_ENDPOINT_PREFIX+ strlen(GFAL_PREFIX_SRM);
	g_strlcpy(buff_tmp, p, strchr(p, '/')-p+1);
	define_se_endpoints = calloc(sizeof(char*), 4);
	for(i1=0;i1 <3; ++i1)
		define_se_endpoints[i1]= strdup(TEST_SRM_DPM_FULLENDPOINT_URL);
	define_se_types= calloc(sizeof(char*), 4);
	char* types[] = { "srm_v1", "srm_v2", "srm_v1"};
	for(i1=0;i1 <3; ++i1)
		define_se_types[i1]= strdup(types[i1]);	




#endif
	gfal_handle handle = gfal_initG(&err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");	
	if(handle==NULL)
		return;
	gfal_srmv2_opt opts;
	gfal_srm_opt_initG(&opts, handle);
	char* surl = TEST_SRM_DPM_ENDPOINT_PREFIX;
	int ret = gfal_get_endpoint_and_setype_from_bdiiG(&opts, surl, buff_endpoint, 2048, &proto, &err);
	assert_true_with_message( ret ==0 && err== NULL && strings_are_equal(buff_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL) && proto== PROTO_SRMv2, " must be a valid endpoint resolution");
	gfal_check_GError(&err);
	memset(buff_endpoint, '\0', sizeof(char)*2048);
	ret = gfal_get_endpoint_and_setype_from_bdiiG(&opts, "srm://lxb540dfshhhh9.cern.ch:8446/test/invalid", buff_endpoint, 2048, &proto, &err);
	assert_true_with_message(ret != 0 && err != NULL && err->code==ENXIO && *buff_endpoint== '\0', " must fail, invalid point");
	g_clear_error(&err);
	gfal_handle_freeG(handle);
}


void gfal2_test_gfal_srm_determine_endpoint_full_endpointG()
{
#if USE_MOCK
	setup_mock_bdii();

#endif
	GError* err = NULL;
	char buff_endpoint[2048];
	enum gfal_srm_proto proto;
	gfal_handle handle = gfal_initG(&err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");
	if(handle==NULL)
		return;
	gfal_srmv2_opt opts;
	gfal_srm_opt_initG(&opts, handle);
	int ret =-1;
	ret = gfal_srm_determine_endpoint(&opts, TEST_SRM_DPM_FULLENDPOINT_PREFIX, buff_endpoint, 2048, &proto, &err);
	assert_true_with_message( ret ==0 && err == NULL && strings_are_equal(buff_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL), " must be a succesfull endpoint determination %d %ld %s", ret, err, buff_endpoint);
	gfal_check_GError(&err);	
	
	ret = gfal_srm_determine_endpoint(&opts, "srm://srm-pps:8443/srm/managerv2?SFN=/castor/cern.ch/grid/dteam/castordev/test-srm-pps_8443-srm2_d0t1-ed6b7013-5329-4f5b", buff_endpoint, 2048, &proto, &err);
	assert_true_with_message( ret ==0 && err == NULL && strings_are_equal(buff_endpoint, "httpg://srm-pps:8443/srm/managerv2"), " must be a succesfull endpoint determination 2 %d %ld %s", ret, err, buff_endpoint);
	gfal_check_GError(&err);
	gfal_handle_freeG(handle);	
}




void gfal2_test_gfal_auto_get_srm_endpoint_full_endpoint_with_no_bdiiG()
{
#if USE_MOCK
	setup_mock_bdii();

#endif
	GError* err = NULL;
	char buff_endpoint[2048];
	enum gfal_srm_proto proto;
	gfal_handle handle = gfal_initG(&err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");
	if(handle==NULL)
		return;
	gfal_plugins_instance(handle, NULL);
	gfal_srmv2_opt opts;
	gfal_srm_opt_initG(&opts, handle);
	gfal_set_nobdiiG(handle, TRUE);
	int ret =-1;
	ret = gfal_srm_determine_endpoint(&opts, TEST_SRM_DPM_FULLENDPOINT_PREFIX, buff_endpoint, 2048, &proto, &err);
	assert_true_with_message( ret ==0 && err == NULL && strings_are_equal(buff_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL), " must be a succesfull endpoint determination %d %ld %s", ret, err, buff_endpoint);
	gfal_check_GError(&err);	
	
	ret = gfal_srm_determine_endpoint(&opts, "srm://srm-pps:8443/srm/managerv2?SFN=/castor/cern.ch/grid/dteam/castordev/test-srm-pps_8443-srm2_d0t1-ed6b7013-5329-4f5b", buff_endpoint, 2048, &proto, &err);
	assert_true_with_message( ret ==0 && err == NULL && strings_are_equal(buff_endpoint, "httpg://srm-pps:8443/srm/managerv2"), " must be a succesfull endpoint determination 2 %d %ld %s", ret, err, buff_endpoint);
	gfal_check_GError(&err);
	memset(buff_endpoint,0, sizeof(char)*2048);
	ret = gfal_srm_determine_endpoint(&opts, TEST_SRM_VALID_SURL_EXAMPLE1, buff_endpoint, 2048, &proto, &err);
	assert_true_with_message( ret !=0 && err != NULL && err->code== EINVAL && *buff_endpoint=='\0', " must be a reported error, bdii is disable");
	g_clear_error(&err);
	gfal_handle_freeG(handle);	
}



void gfal2_test_gfal_srm_determine_endpoint_not_fullG()
{
	
#if USE_MOCK
	int i1;
	setup_mock_bdii();
	char buff_tmp[2048];
	char* p = TEST_SRM_DPM_ENDPOINT_PREFIX+ strlen(GFAL_PREFIX_SRM);
	g_strlcpy(buff_tmp, p, strchr(p, '/')-p+1);
	define_se_endpoints = calloc(sizeof(char*), 4);
	for(i1=0;i1 <3; ++i1)
		define_se_endpoints[i1]= strdup(TEST_SRM_DPM_FULLENDPOINT_URL);
	define_se_types= calloc(sizeof(char*), 4);
	char* types[] = { "srm_v1", "srm_v2", "srm_v1"};
	for(i1=0;i1 <3; ++i1)
		define_se_types[i1]= strdup(types[i1]);	



#endif
	GError* err = NULL;
	char buff_endpoint[2048];
	enum gfal_srm_proto proto;
	gfal_handle handle = gfal_initG(&err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");
	if(handle==NULL)
		return;
	gfal_srmv2_opt opts;
	gfal_srm_opt_initG(&opts, handle);
	gfal_plugins_instance(handle, NULL);
	int ret =-1;
	ret = gfal_srm_determine_endpoint(&opts, TEST_SRM_DPM_ENDPOINT_PREFIX, buff_endpoint, 2048, &proto, &err);
	assert_true_with_message( ret ==0 && err == NULL && strings_are_equal(buff_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL), " must be a succesfull endpoint determination %d %ld %s", ret, err, buff_endpoint);
	gfal_check_GError(&err);	
	
	ret = gfal_srm_determine_endpoint(&opts, "http://google.com", buff_endpoint, 2048, &proto, &err);
	assert_true_with_message( ret !=0 && err != NULL, "error must be reported %d %s", ret, ((err)?err->message:""));
	g_clear_error(&err);
	gfal_handle_freeG(handle);	
}







void gfal2_test_gfal_get_hostname_from_surl()
{
	GError * tmp_err=NULL;
	char hostname[2048];
	memset(hostname, '\0', 2048);
	int ret= gfal_get_hostname_from_surlG(TEST_SRM_VALID_SURL_EXAMPLE1, hostname, 2048, &tmp_err);
	assert_true_with_message( ret==0  && tmp_err==NULL && *hostname!='\0', " must be a success");
	
	char* p = strchr(TEST_SRM_DPM_ENDPOINT_PREFIX+7, '/');
	assert_true_with_message(p!=NULL, " no / contained in the url");
	if(!p)	
		return;
	assert_true_with_message( strncmp(TEST_SRM_DPM_ENDPOINT_PREFIX+6,hostname, p-TEST_SRM_DPM_ENDPOINT_PREFIX-7) == 0, " must be the same string");
}



void gfal2_test_gfal_select_best_protocol_and_endpointG()
{
	char endpoint[2048];
	memset(endpoint, '\0', sizeof(char)*2048);
	enum gfal_srm_proto srm_type;
	GError * err= NULL;
	gfal_handle handle  = gfal_initG(&err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");
	if(handle==NULL)
		return;
	gfal_plugins_instance(handle, NULL);
	gfal_srmv2_opt opts;
	gfal_srm_opt_initG(&opts, handle);
	gfal_set_default_storageG(&opts, PROTO_SRMv2);
	char* endpoint_list[] = { "everest", "montblanc", NULL};
	char* se_type_list[] = { "srm_v1", "srm_v2", NULL };
	int ret = gfal_select_best_protocol_and_endpointG(&opts, se_type_list, endpoint_list, endpoint, 2048, &srm_type, &err);
	assert_true_with_message(ret ==0 && err == NULL, " must be a succefull call to the best select");
	gfal_check_GError(&err);

	assert_true_with_message(strings_are_equal(endpoint,"montblanc"), " reponse not match correctly");
	// try with another version by default
	gfal_set_default_storageG(&opts, PROTO_SRM);
	ret = gfal_select_best_protocol_and_endpointG(&opts, se_type_list, endpoint_list, endpoint, 2048, &srm_type, &err);
	assert_true_with_message(ret ==0 && err == NULL, " must be a succefull call to the best select");
	gfal_check_GError(&err);
	assert_true_with_message(strings_are_equal(endpoint,"everest") , "must be a valid check");	
	gfal_handle_freeG(handle);
}


void gfal2_test_gfal_srm_getTURLS_one_success()
{
	setup_mock_srm();
	setup_mock_bdii();
	int i1;
#if USE_MOCK
	define_se_endpoints = calloc(sizeof(char*), 4); // set the response of the MDS layer for endpoint
	for(i1=0;i1 <3; ++i1)
		define_se_endpoints[i1]= strdup(TEST_SRM_DPM_FULLENDPOINT_URL);
	define_se_types= calloc(sizeof(char*), 4);
	char* types[] = { "srm_v1", "srm_v2", "srm_v1"};
	for(i1=0;i1 <3; ++i1)
		define_se_types[i1]= strdup(types[i1]);	


	will_respond(srm_mock_srm_context_init, 0, want_non_null(context), want_string(srm_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL));
	defined_get_output = calloc(sizeof(struct srmv2_pinfilestatus),1);
	defined_get_output[0].turl= strdup(TEST_SRM_TURL_EXAMPLE1);
	will_respond(srm_mock_srm_prepare_to_get, 1, want_non_null(context), want_non_null(input), want_non_null(output));
	always_return(srm_mock_srm_srmv2_filestatus_delete,0);
	always_return(srm_mock_srm_srmv2_pinfilestatus_delete,0);
#endif
	GError* tmp_err=NULL;
	gfal_handle handle  = gfal_initG(&tmp_err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");
	if(handle==NULL)
		return;
	gfal_srmv2_opt opts;
	gfal_plugins_instance(handle, NULL);	
	gfal_srm_opt_initG(&opts, handle);

	gfal_srm_result* resu=NULL;
	char* surls[] = {TEST_SRM_VALID_SURL_EXAMPLE1, NULL};
	int ret = gfal_srm_getTURLS(&opts, surls, &resu, &tmp_err);
	assert_true_with_message(ret ==1 && resu != NULL && tmp_err == NULL, " must be a successfull request");
	gfal_check_GError(&tmp_err);
	assert_true_with_message(resu[0].err_code == 0 && *(resu[0].err_str)== '\0' && strings_are_equal(resu[0].turl, TEST_SRM_TURL_EXAMPLE1), 
				" must be a valid turl, maybe the turl has changed %d %ld %s",resu[0].err_code, resu[0].err_str, resu[0].turl);	
	free(resu);
	gfal_handle_freeG(handle);
}


void gfal2_test_gfal_srm_getTURLS_bad_urls()
{
	GError* tmp_err=NULL;
	gfal_handle handle  = gfal_initG(&tmp_err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");
	if(handle==NULL)
		return;
	gfal_srm_result* resu=NULL;
	gfal_srmv2_opt opts;
	gfal_srm_opt_initG(&opts, handle);
	char* surls[] = {NULL, NULL};
	int ret = gfal_srm_getTURLS(&opts, surls, &resu, &tmp_err);
	assert_true_with_message(ret <=0 && resu == NULL && tmp_err != NULL, " must be a failure, invalid SURLs ");
	g_clear_error(&tmp_err);
	free(resu);
	gfal_handle_freeG(handle);
}


void gfal2_test_gfal_srm_getTURLS_pipeline_success()
{
	setup_mock_srm();
	setup_mock_bdii();
	int i1;
#if USE_MOCK
	define_se_endpoints = calloc(sizeof(char*), 4); // set the response of the MDS layer for endpoint
	for(i1=0;i1 <3; ++i1)
		define_se_endpoints[i1]= strdup(TEST_SRM_DPM_FULLENDPOINT_URL);
	define_se_types= calloc(sizeof(char*), 4);
	char* types[] = { "srm_v1", "srm_v2", "srm_v1"};
	for(i1=0;i1 <3; ++i1)
		define_se_types[i1]= strdup(types[i1]);	


	will_respond(srm_mock_srm_context_init, 0, want_non_null(context), want_string(srm_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL));
	defined_get_output = calloc(sizeof(struct srmv2_pinfilestatus),3);
	defined_get_output[0].turl= strdup(TEST_SRM_TURL_EXAMPLE1);
	defined_get_output[1].status= ENOENT;
	defined_get_output[1].explanation = strdup("err msg");
	defined_get_output[2].turl = strdup(TEST_SRM_TURL_EXAMPLE1);
	will_respond(srm_mock_srm_prepare_to_get, 3, want_non_null(context), want_non_null(input), want_non_null(output));
	always_return(srm_mock_srm_srmv2_filestatus_delete,0);
	always_return(srm_mock_srm_srmv2_pinfilestatus_delete,0);
#endif
	GError* tmp_err=NULL;
	gfal_handle handle  = gfal_initG(&tmp_err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");
	if(handle==NULL)
		return;
	gfal_srmv2_opt opts;
	gfal_srm_opt_initG(&opts, handle);
	gfal_plugins_instance(handle, NULL);
	gfal_srm_result* resu=NULL;
	char* surls[] = {TEST_SRM_VALID_SURL_EXAMPLE1, TEST_SRM_INVALID_SURL_EXAMPLE2, TEST_SRM_VALID_SURL_EXAMPLE1, NULL};
	int ret = gfal_srm_getTURLS(&opts, surls, &resu, &tmp_err);
	assert_true_with_message(ret ==g_strv_length(surls) && resu != NULL && tmp_err == NULL, " must be a successfull request");
	gfal_check_GError(&tmp_err);
	assert_true_with_message(resu[0].err_code == 0 && *(resu[0].err_str)== '\0' && strings_are_equal(resu[0].turl, TEST_SRM_TURL_EXAMPLE1), 
				" must be a valid turl, maybe the turl has changed %d %ld %s",resu[0].err_code, resu[0].err_str, resu[0].turl);	

	assert_true_with_message(resu[1].err_code == ENOENT && *(resu[1].err_str)!= '\0' && *(resu[1].turl) == '\0', 
				" must be a invalid turl 2 ");	
	assert_true_with_message(resu[2].err_code == 0 && *(resu[2].err_str)== '\0' && strings_are_equal(resu[2].turl, TEST_SRM_TURL_EXAMPLE1), 
				" must be a valid turl 3, maybe the turl has changed ");	
	free(resu);
	gfal_handle_freeG(handle);
}


void gfal2_test_srm_get_checksum()
{
	//setup_mock_srm();	
	/*char checksum[2048]; --> disable tmp 
	char checksumtype[2048];
	GError* tmp_err=NULL;
	
	gfal_handle handle  = gfal_initG(&tmp_err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");
	if(handle==NULL)
		return;
	gfal_srmv2_opt opts;
	gfal_srm_opt_initG(&opts, handle);
	gfal_plugins_instance(handle, NULL);
	
	int ret = gfal_srm_cheksumG(&opts, TEST_SRM_ONLY_READ_ACCESS, checksum, 2048, checksumtype, 2048, &tmp_err);
	assert_true_with_message(ret ==0 && tmp_err== NULL, " must be a valid response %d %d ", ret, tmp_err);
	assert_true_with_message( strcmp(checksum, TEST_SRM_CHECKSUM_VALID)==0, "must be a valis checksum %s %s",checksum, TEST_SRM_CHECKSUM_VALID);
	assert_true_with_message( strcmp(checksumtype, TEST_SRM_CHKTYPE_VALID)==0, "must be a valis checksum type %s %s",checksumtype, TEST_SRM_CHKTYPE_VALID);

	g_clear_error(&tmp_err);
	ret = gfal_srm_cheksumG(&opts, TEST_SRML_OPENDIR_ENOENT, checksum, 2048, checksumtype, 2048, &tmp_err); // test a non existing dir
	assert_true_with_message(ret !=0 && tmp_err!= NULL && ((tmp_err)?tmp_err->code:0) == ENOENT, " must be a no existing dir %d %d", ret,((tmp_err)?tmp_err->code:0) );	
	g_clear_error(&tmp_err);*/
}
