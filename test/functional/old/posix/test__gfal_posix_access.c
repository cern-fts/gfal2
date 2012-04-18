
/* unit test for posix access func */


#include <cgreen/cgreen.h>
#include <errno.h>
#include <stdio.h>

#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_common_plugin.h>


#include <common/lfc/lfc_ifce_ng.h>
#include <posix/gfal_posix_api.h>
#include <posix/gfal_posix_internal.h>

#include "../unit_test_constants.h"


#include "../mock/gfal_lfc_mock_test.h"

#include "../common/gfal__test_common_srm.h"
#include "../common/gfal__test_plugin.h"




void gfal2_test_access_posix_guid_exist()
{
	int i1;
	// test exist guid
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	if( gfal_check_GError(&mock_err))
		return;

	define_linkinfos= calloc(sizeof(struct lfc_linkinfo),3);
	define_numberlinkinfos=3;
	for(i1=0; i1< define_numberlinkinfos; ++i1)
		g_strlcpy(define_linkinfos[i1].path, "/dteam/test/imagine", 2048);
	will_respond(lfc_mock_getlinks, 0, want_string(guid, TEST_GUID_VALID_ACCESS+5), want(path, NULL), want_non_null(nbentries), want_non_null(linkinfos));	
	will_respond(lfc_mock_access, 0, want_string(path, "/dteam/test/imagine"), want(mode, F_OK));
	will_respond(lfc_mock_getlinks, ENOENT, want_string(guid, TEST_GUID_NOEXIST_ACCESS+5), want(path, NULL), want_non_null(nbentries), want_non_null(linkinfos));		
	always_return(lfc_mock_getlinks, EINVAL);
#endif
	int ret;
	gfal_posix_clear_error();
	errno =0;
	ret = gfal_access(TEST_GUID_VALID_ACCESS, F_OK);
	assert_true_with_message(ret==0 && gfal_posix_code_error()==0 && errno == 0, " must be a valid access to the guid %d %d %d",ret, errno, gfal_posix_code_error());
	gfal_posix_check_error();
	gfal_posix_clear_error();
	ret = gfal_access(TEST_GUID_NOEXIST_ACCESS, F_OK);
	assert_true_with_message(ret == -1 && errno == ENOENT && gfal_posix_code_error()== ENOENT, "must be a non exist guid %d %d %d", ret, errno, gfal_posix_code_error());
	gfal_posix_clear_error();
	ret = gfal_access("google.com", F_OK);
	assert_true_with_message(ret == -1 && errno == EPROTONOSUPPORT && gfal_posix_code_error() == EPROTONOSUPPORT, " must be a syntax error");
	gfal_posix_clear_error();

}



void gfal2_test_access_posix_guid_read()
{
	int i1;
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	if( gfal_check_GError(&mock_err))
		return;

	define_linkinfos= calloc(sizeof(struct lfc_linkinfo),3);
	define_numberlinkinfos=3;
	for(i1=0; i1< define_numberlinkinfos; ++i1)
		g_strlcpy(define_linkinfos[i1].path, "/dteam/test/imagine", 2048);
	will_respond(lfc_mock_getlinks, 0, want_string(guid, TEST_GUID_ONLY_READ_ACCESS+5), want(path, NULL), want_non_null(nbentries), want_non_null(linkinfos));	
	will_respond(lfc_mock_access, 0, want_string(path, "/dteam/test/imagine"), want(mode, R_OK));
	will_respond(lfc_mock_getlinks, ENOENT, want_string(guid, TEST_GUID_NOEXIST_ACCESS+5), want(path, NULL), want_non_null(nbentries), want_non_null(linkinfos));	
	will_respond(lfc_mock_getlinks, EACCES, want_string(guid, TEST_GUID_NO_READ_ACCESS+5), want(path, NULL), want_non_null(nbentries), want_non_null(linkinfos));			
	always_return(lfc_mock_getlinks, EINVAL);
#endif
	// test exist guid
	int ret;

	ret = gfal_access(TEST_GUID_ONLY_READ_ACCESS, R_OK);
	assert_true_with_message(ret==0 && gfal_posix_code_error()==0 && errno == 0, " must be a valid access to the guid %d %d %d",ret, errno, gfal_posix_code_error());
	gfal_posix_check_error();
	ret = gfal_access(TEST_GUID_NOEXIST_ACCESS, R_OK);
	assert_true_with_message(ret == -1 && gfal_posix_code_error() == ENOENT && errno == ENOENT, " must be a non existing guid %d %d %d",ret, errno, gfal_posix_code_error());
	gfal_posix_clear_error();
	ret = gfal_access(TEST_GUID_NO_READ_ACCESS, R_OK);
	assert_true_with_message( ret == -1 && errno == EACCES && gfal_posix_code_error() == EACCES, "must be an unaccessible file %d %d %d",ret, errno, gfal_posix_code_error());
	gfal_posix_clear_error();
	
}



void gfal2_test_access_posix_guid_write()
{
	// test exist guid
	int ret,i1;
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	if( gfal_check_GError(&mock_err))
		return;

	define_linkinfos= calloc(sizeof(struct lfc_linkinfo),3);
	define_numberlinkinfos=3;
	for(i1=0; i1< define_numberlinkinfos; ++i1)
		g_strlcpy(define_linkinfos[i1].path, "/dteam/test/imagine", 2048);
	will_respond(lfc_mock_getlinks, 0, want_string(guid, TEST_GUID_WRITE_ACCESS+5), want(path, NULL), want_non_null(nbentries), want_non_null(linkinfos));	
	will_respond(lfc_mock_access, 0, want_string(path, "/dteam/test/imagine"), want(mode, W_OK));
	will_respond(lfc_mock_getlinks, ENOENT, want_string(guid, TEST_GUID_NOEXIST_ACCESS+5), want(path, NULL), want_non_null(nbentries), want_non_null(linkinfos));	
	will_respond(lfc_mock_getlinks, EACCES, want_string(guid, TEST_GUID_NO_WRITE_ACCESS+5), want(path, NULL), want_non_null(nbentries), want_non_null(linkinfos));			
	always_return(lfc_mock_getlinks, EINVAL);
#endif

	ret = gfal_access(TEST_GUID_WRITE_ACCESS, W_OK);
	assert_true_with_message(ret == 0 && errno ==0 && gfal_posix_code_error() == 0 , " must be a valid check access");
	gfal_posix_check_error();
	ret = gfal_access(TEST_GUID_NOEXIST_ACCESS, W_OK);
	assert_true_with_message(ret == -1 && errno ==ENOENT && gfal_posix_code_error() == ENOENT , " must be a non-existing  file");
	gfal_posix_clear_error();
	ret = gfal_access(TEST_GUID_NO_WRITE_ACCESS, W_OK);
	assert_true_with_message(ret == -1 && errno ==EACCES && gfal_posix_code_error() == EACCES , " must be a valid check access");
	gfal_posix_clear_error();	
}



void gfal2_test_access_posix_lfn_exist()
{
	// test exist guid
	int ret;
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	if( gfal_check_GError(&mock_err))
		return;

	will_respond(lfc_mock_access, 0, want_string(path, TEST_LFC_ONLY_READ_ACCESS+4), want(mode, F_OK));
	will_respond(lfc_mock_access, ENOENT, want_string(path, TEST_LFC_NOEXIST_ACCESS+4), want(mode, F_OK));
	always_return(lfc_mock_getlinks, EINVAL);
#endif
	ret = gfal_access(TEST_LFC_ONLY_READ_ACCESS, F_OK);
	assert_true_with_message(ret == 0 && errno ==0 && gfal_posix_code_error() == 0 , " must be a valid check access");
	gfal_posix_check_error();
	ret = gfal_access(TEST_LFC_NOEXIST_ACCESS, F_OK);
	assert_true_with_message(ret == -1 && errno ==ENOENT && gfal_posix_code_error() == ENOENT , " must be a non-existing  file");
	gfal_posix_clear_error();
	ret = gfal_access("google.com", F_OK);
	assert_true_with_message(ret == -1 && errno ==EPROTONOSUPPORT && gfal_posix_code_error() == EPROTONOSUPPORT , " must be a valid check access");
	gfal_posix_clear_error();
}



void gfal2_test_access_posix_lfn_read()
{
	// test exist guid
	int ret;
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	if( gfal_check_GError(&mock_err))
		return;

	will_respond(lfc_mock_access, 0, want_string(path, TEST_LFC_ONLY_READ_ACCESS+4), want(mode, R_OK));
	will_respond(lfc_mock_access, ENOENT, want_string(path, TEST_LFC_NOEXIST_ACCESS+4), want(mode, R_OK));
	will_respond(lfc_mock_access, EACCES, want_string(path, TEST_LFC_NO_READ_ACCESS+4), want(mode, R_OK));
	always_return(lfc_mock_getlinks, EINVAL);
#endif
	ret = gfal_access(TEST_LFC_ONLY_READ_ACCESS, R_OK);
	assert_true_with_message(ret == 0 && errno ==0 && gfal_posix_code_error() == 0 , " must be a valid check access");
	gfal_posix_check_error();
	ret = gfal_access(TEST_LFC_NOEXIST_ACCESS, R_OK);
	assert_true_with_message(ret == -1 && errno ==ENOENT && gfal_posix_code_error() == ENOENT , " must be a non-existing  file");
	gfal_posix_clear_error();
	ret = gfal_access(TEST_LFC_NO_READ_ACCESS, R_OK);
	assert_true_with_message(ret == -1 && errno ==EACCES && gfal_posix_code_error() == EACCES , " must be a valid check access");
	gfal_posix_clear_error();
	
}



void gfal2_test_access_posix_lfn_write()
{
	// test exist guid
	int ret;
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	if( gfal_check_GError(&mock_err))
		return;

	will_respond(lfc_mock_access, 0, want_string(path, TEST_LFC_WRITE_ACCESS+4), want(mode, W_OK));
	will_respond(lfc_mock_access, ENOENT, want_string(path, TEST_LFC_NOEXIST_ACCESS+4), want(mode, W_OK));
	will_respond(lfc_mock_access, EACCES, want_string(path, TEST_LFC_NO_WRITE_ACCESS+4), want(mode, W_OK));
	always_return(lfc_mock_getlinks, EINVAL);
#endif
	ret = gfal_access(TEST_LFC_WRITE_ACCESS, W_OK);
	assert_true_with_message(ret == 0 && errno ==0 && gfal_posix_code_error() == 0 , " must be a valid check access");
	gfal_posix_check_error();
	ret = gfal_access(TEST_LFC_NOEXIST_ACCESS, W_OK);
	assert_true_with_message(ret == -1 && errno ==ENOENT && gfal_posix_code_error() == ENOENT , " must be a non-existing  file");
	gfal_posix_clear_error();
	ret = gfal_access(TEST_LFC_NO_WRITE_ACCESS, W_OK);
	assert_true_with_message(ret == -1 && errno ==EACCES && gfal_posix_code_error() == EACCES , " must be a valid check access");
	gfal_posix_clear_error();
	
}




void gfal2_test_access_posix_srm_exist()
{
	// test exist guid
	int ret;
#if USE_MOCK
	mock_srm_access_right_response(TEST_SRM_ONLY_READ_ACCESS);
#endif
	ret = gfal_access(TEST_SRM_ONLY_READ_ACCESS, F_OK);
	assert_true_with_message(ret == 0 && errno ==0 && gfal_posix_code_error() == 0 , " must be a valid check access %d %d %d", ret, errno, gfal_posix_code_error());
	gfal_posix_check_error();
#if USE_MOCK
	mock_srm_access_error_response(TEST_SRM_NOEXIST_ACCESS,ENOENT);
#endif
	ret = gfal_access(TEST_SRM_NOEXIST_ACCESS, F_OK);
	assert_true_with_message(ret == -1 && errno ==ENOENT && gfal_posix_code_error() == ENOENT , " must be a non-existing  file");
	gfal_posix_clear_error();
	ret = gfal_access("google.com", F_OK);
	assert_true_with_message(ret == -1 && errno ==EPROTONOSUPPORT && gfal_posix_code_error() == EPROTONOSUPPORT , " must be a valid check access");
	gfal_posix_clear_error();
}




void gfal2_test_access_posix_srm_read()
{
	// test exist guid
	int ret;
#if USE_MOCK
	mock_srm_access_right_response(TEST_SRM_ONLY_READ_ACCESS);
#endif
	ret = gfal_access(TEST_SRM_ONLY_READ_ACCESS, R_OK);
	assert_true_with_message(ret == 0 && errno ==0 && gfal_posix_code_error() == 0 , " must be a valid check access %d %d %d", ret, errno, gfal_posix_code_error());
	gfal_posix_check_error();
#if USE_MOCK
	mock_srm_access_error_response(TEST_SRM_NOEXIST_ACCESS,ENOENT);
#endif
	ret = gfal_access(TEST_SRM_NOEXIST_ACCESS, R_OK);
	assert_true_with_message(ret == -1 && errno ==ENOENT && gfal_posix_code_error() == ENOENT , " must be a non-existing  file");
	gfal_posix_clear_error();
#if USE_MOCK
	mock_srm_access_error_response(TEST_SRM_NOEXIST_ACCESS,EACCES);
#endif
	ret = gfal_access(TEST_SRM_NO_READ_ACCESS, R_OK);
	assert_true_with_message(ret == -1 && errno ==EACCES && gfal_posix_code_error() == EACCES , " must be a valid check access");
	gfal_posix_clear_error();
}



void gfal2_test_access_posix_srm_write()
{
	// test exist guid
	int ret;
#if USE_MOCK
	mock_srm_access_right_response(TEST_SRM_WRITE_ACCESS);
#endif
	ret = gfal_access(TEST_SRM_WRITE_ACCESS, W_OK);
	assert_true_with_message(ret == 0 && errno ==0 && gfal_posix_code_error() == 0 , " must be a valid check access %d %d %d", ret, errno, gfal_posix_code_error());
	gfal_posix_check_error();
#if USE_MOCK
	mock_srm_access_error_response(TEST_SRM_NOEXIST_ACCESS,ENOENT);
#endif
	ret = gfal_access(TEST_SRM_NOEXIST_ACCESS, W_OK);
	assert_true_with_message(ret == -1 && errno ==ENOENT && gfal_posix_code_error() == ENOENT , " must be a non-existing  file");
	gfal_posix_clear_error();
#if USE_MOCK
	mock_srm_access_error_response(TEST_SRM_NO_WRITE_ACCESS,EACCES);
#endif
	ret = gfal_access(TEST_SRM_NO_WRITE_ACCESS, W_OK);
	assert_true_with_message(ret == -1 && errno ==EACCES && gfal_posix_code_error() == EACCES , " must be a valid check access");
	gfal_posix_clear_error();
}
