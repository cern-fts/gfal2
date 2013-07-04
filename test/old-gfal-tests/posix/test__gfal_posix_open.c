
/* unit test for posix open func */

#include <stdio.h>
#include <cgreen/cgreen.h>
#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_constants.h>
#include "../unit_test_constants.h"
#include <posix/gfal_posix_api.h>
#include <posix/gfal_posix_internal.h>
#include <errno.h>
#include "../mock/gfal_lfc_mock_test.h"
#include "../mock/gfal_srm_mock_test.h"
#include "../mock/gfal_mds_mock_test.h"
#include "../common/rfio/test__gfal_rfio_plugin.h"
#include "../common/gfal__test_common_srm.h"
#include "../common/gfal__test_plugin.h"
#include "test__gfal_posix_open.h"




void test_mock_srm_open_valid(char** tab, char** tab_turl, int* res){
#if USE_MOCK
	test_rfio_mock_all();
	setup_mock_srm();
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
	
	define_mock_endpoints(TEST_SRM_DPM_FULLENDPOINT_URL); // mock the mds for the srm endpoitn resolution


	will_respond(srm_mock_srm_context_init, 0, want_non_null(context), want_string(srm_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL));
	define_mock_srmv2_pinfilestatus(1, tab, NULL, tab_turl, res);
	will_respond(srm_mock_srm_prepare_to_get, 1, want_non_null(context), want_non_null(input), want_non_null(output));

	will_respond(rfio_mock_open, 15, want_non_null(path));
	will_respond(rfio_mock_close, 0, want(fd, 15));
#endif
}

void test_mock_srm_open_write_valid(char** tab, char** tab_turl, int* res){
#if USE_MOCK
	test_rfio_mock_all();
	setup_mock_srm();
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
	int status[] = { 0,0 };
	char* turls[] = { "rfio://mockedturl", NULL };
	char* surls[] = { "srm://mockedsurl", NULL };
	
	define_mock_endpoints(TEST_SRM_DPM_FULLENDPOINT_URL); // mock the mds for the srm endpoitn resolution


	will_respond(srm_mock_srm_context_init, 0, want_non_null(context), want_string(srm_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL));
	define_mock_srmv2_putoutput(1, tab, NULL, tab_turl, res);
	will_respond(srm_mock_srm_prepare_to_put, 1, want_non_null(context), want_non_null(input), want_non_null(output));

	will_respond(rfio_mock_open, 15, want_non_null(path));

	define_put_done(1, surls, NULL, turls, status);
	will_respond(srm_mock_srm_put_done,1, want_non_null(context));
	will_respond(rfio_mock_close, 0, want(fd, 15));
#endif
}

void test_mock_srm_open_invalid(char** tab, char** tab_exp, int* res){
#if USE_MOCK
	test_rfio_mock_all();
	setup_mock_srm();
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
	
	define_mock_endpoints(TEST_SRM_DPM_FULLENDPOINT_URL); // mock the mds for the srm endpoitn resolution


	will_respond(srm_mock_srm_context_init, 0, want_non_null(context), want_string(srm_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL));
	define_mock_srmv2_pinfilestatus(1, tab, tab_exp, NULL , res);
	will_respond(srm_mock_srm_prepare_to_get, 1, want_non_null(context), want_non_null(input), want_non_null(output));
#endif
}


void test_mock_lfc_open_valid(const char* lfc_url){ 
#if USE_MOCK
	char* tab[]= { TEST_SRM_VALID_SURL_EXAMPLE1, NULL };	
	char* tab_turl[] = { TEST_SRM_TURL_EXAMPLE1, NULL };
	int res[] = { 0, 0 };
	test_mock_srm_open_valid(tab, tab_turl, res);
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);

	if( gfal_check_GError(&mock_err))
		return;	

	define_mock_filereplica(1, tab);
	will_respond(lfc_mock_getreplica, 0, want_string(path, ((char*)lfc_url)+4), want_non_null(nbentries), want_non_null(rep_entries));	

#endif
}

void test_mock_lfc_open_enoent(const char* lfc_url){
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	setup_mock_srm();
	if( gfal_check_GError(&mock_err))
		return;	
	will_respond(lfc_mock_getreplica, ENOENT, want_string(path, ((char*)lfc_url)+4), want_non_null(nbentries), want_non_null(rep_entries));	
	
#endif	
}

void test_mock_lfc_open_eacces(const char* lfc_url){
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	setup_mock_srm();
	if( gfal_check_GError(&mock_err))
		return;	
	will_respond(lfc_mock_getreplica, EACCES, want_string(path, ((char*)lfc_url)+4), want_non_null(nbentries), want_non_null(rep_entries));	
	
#endif	
}

void test_mock_guid_open_valid(const char * guid1){
#if USE_MOCK
	int i1;
	test_mock_lfc_open_valid(TEST_LFC_OPEN_EXIST);
	define_linkinfos= calloc(sizeof(struct lfc_linkinfo),3);
	define_numberlinkinfos=3;
	for(i1=0; i1< define_numberlinkinfos; ++i1)
		g_strlcpy(define_linkinfos[i1].path, TEST_LFC_OPEN_EXIST+4, 2048);
	will_respond(lfc_mock_getlinks, 0, want_string(guid, ((char*)guid1)+5), want(path, NULL), want_non_null(nbentries), want_non_null(linkinfos));	
#endif	
}

void test_mock_guid_open_invalid(const char * guid1){
#if USE_MOCK
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
		
	will_respond(lfc_mock_getlinks, ENOENT, want_string(guid, ((char*)guid1)+5), want(path, NULL), want_non_null(nbentries), want_non_null(linkinfos));	
#endif	
}

void gfal2_test_open_posix_all_simple()
{
	int ret = gfal_open(NULL, O_RDONLY, 555);
	assert_true_with_message( ret < 0 && gfal_posix_code_error() == EFAULT && errno==EFAULT, " must be a EFAULT response");
	gfal_posix_clear_error();
	
}

static void test_generic_open_simple(char* url_exist, char* url_noent, char* url_noaccess){
	int ret = -1;
	int fd;
	if(url_exist){
		fd = gfal_open(url_exist, O_RDONLY, 555);
		assert_true_with_message(fd >0 && gfal_posix_code_error()==0 && errno==0, " must be a valid open %d %d %d", fd, gfal_posix_code_error(), errno);
		gfal_posix_check_error();
		ret = gfal_close(fd);
		assert_true_with_message(fd !=0 && ret==0 && gfal_posix_code_error()==0 && errno==0, " must be a valid close %d %d %d", ret, gfal_posix_code_error(), errno);
		gfal_posix_check_error();	
		ret = gfal_close(fd);
		assert_true_with_message( ret==-1 && gfal_posix_code_error()==EBADF && errno==EBADF, " must be a bad descriptor %d %d %d", ret, gfal_posix_code_error(), errno);
	}
	
	if(url_noent){
		gfal_posix_clear_error();
		fd = gfal_open(url_noent, O_RDONLY, 555);
		assert_true_with_message( fd <=0 && gfal_posix_code_error()==ENOENT && errno==ENOENT, " must be a non existing file %d %d %d", ret, gfal_posix_code_error(), errno);
	}
	
	if(url_noaccess){	
		gfal_posix_clear_error();
		fd = gfal_open(url_noaccess, O_RDONLY, 555);
		assert_true_with_message( fd <=0 && gfal_posix_code_error()==EACCES && errno==EACCES, " must be a non accessible file %d %d %d", fd, gfal_posix_code_error(), errno);
		gfal_posix_clear_error();	
	}	
	
}


void gfal2_test_open_posix_local_simple()
{
	system(TEST_LOCAL_OPEN_CREATE_COMMAND);
	
	test_generic_open_simple(TEST_LOCAL_OPEN_EXIST, TEST_LOCAL_OPEN_NOEXIST, TEST_LOCAL_OPEN_NOACCESS);

}



void gfal2_test_open_posix_lfc_simple()
{
	test_mock_lfc_open_valid(TEST_LFC_OPEN_EXIST);
	test_mock_lfc_open_enoent(TEST_LFC_OPEN_NOEXIST);
	test_mock_lfc_open_eacces(TEST_LFC_OPEN_NOACCESS);
	test_generic_open_simple(TEST_LFC_OPEN_EXIST, TEST_LFC_OPEN_NOEXIST, TEST_LFC_OPEN_NOACCESS);

}



void gfal2_test_open_posix_srm_simple()
{
	char* tab[]= { TEST_SRM_VALID_SURL_EXAMPLE1, NULL };	
	char* tab_turl[] = { TEST_SRM_TURL_EXAMPLE1, NULL };
	int res[] = { 0, 0 };
	int res2[] = { ENOENT, 0 };
	char* exp[] = { "mock enoent", NULL };
	test_mock_srm_open_valid(tab, tab_turl, res);
	test_generic_open_simple(TEST_SRM_OPEN_EXIST, NULL, NULL);
	test_mock_srm_open_invalid(tab, exp, res2);
	test_generic_open_simple(NULL, TEST_SRM_OPEN_NOEXIST,NULL);
	
}



void gfal2_test_open_posix_guid_simple()
{
	test_mock_guid_open_valid(TEST_GUID_OPEN_EXIST);
	test_generic_open_simple(TEST_GUID_OPEN_EXIST, NULL, NULL);
	test_mock_guid_open_invalid(TEST_GUID_OPEN_NONEXIST);
	test_generic_open_simple(NULL, TEST_GUID_NOEXIST_ACCESS, NULL);
}
