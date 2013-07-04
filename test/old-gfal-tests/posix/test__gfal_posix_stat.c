
/* unit test for posix access func */


#include <cgreen/cgreen.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>


#include "../../src/common/gfal_prototypes.h"
#include "../../src/common/gfal_types.h"
#include "../../src/common/gfal_constants.h"
#include "../unit_test_constants.h"
#include "gfal_posix_api.h"
#include "gfal_posix_internal.h"


#include "../common/gfal__test_common_srm.h"
#include "../common/gfal__test_plugin.h"
#include "../mock/gfal_mds_mock_test.h"
#include "../mock/gfal_lfc_mock_test.h"
#include "../mock/gfal_srm_mock_test.h"

void create_srm_stat_env_mock(){
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	setup_mock_srm();
	if( gfal_check_GError(&mock_err))
		return;

	define_mock_endpoints(TEST_SRM_DPM_FULLENDPOINT_URL);
	define_mock_stat_file_valid(TEST_GFAL_SRM_FILE_STAT_OK, TEST_GFAL_SRM_FILE_STAT_MODE_VALUE, TEST_GFAL_SRM_FILE_STAT_UID_VALUE, TEST_GFAL_SRM_FILE_STAT_GID_VALUE);

	will_respond(srm_mock_srm_context_init, 0, want_non_null(context), want_string(srm_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL));
	will_respond(srm_mock_srm_ls, 0, want_non_null(context), want_non_null(inut), want_non_null(output));	

#endif		
	
	
}

void create_srm_stat_env_mock_noent(){
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	setup_mock_srm();
	if( gfal_check_GError(&mock_err))
		return;

	define_mock_endpoints(TEST_SRM_DPM_FULLENDPOINT_URL);
	define_mock_stat_file_error(TEST_GFAL_SRM_FILE_STAT_NONEXIST, ENOENT, "epic fail");

	will_respond(srm_mock_srm_context_init, 0, want_non_null(context), want_string(srm_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL));
	will_respond(srm_mock_srm_ls, 0, want_non_null(context), want_non_null(inut), want_non_null(output));	

#endif		
	
	
}


void gfal2_test__gfal_posix_stat_lfc()
{
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	if( gfal_check_GError(&mock_err))
		return;

	define_mock_filestatg(TEST_GFAL_LFC_FILE_STAT_MODE_VALUE, TEST_GFAL_LFC_FILE_STAT_GID_VALUE, TEST_GFAL_LFC_FILE_STAT_UID_VALUE);
	will_respond(lfc_mock_statg, 0, want_string(path, TEST_GFAL_LFC_FILE_STAT_OK+5), want_non_null(linkinfos));
	will_respond(lfc_mock_statg, ENOENT, want_string(path, TEST_GFAL_LFC_FILE_STAT_NONEXIST+5), want_non_null(linkinfos));
	always_return(lfc_mock_getlinks, EINVAL);
#endif
	struct stat buff;
	memset(&buff,0, sizeof(struct stat));
	int res = gfal_stat(TEST_GFAL_LFC_FILE_STAT_OK, &buff);
	assert_true_with_message(res ==0 && errno==0 && gfal_posix_code_error()==0, " must be a success");
	
	assert_true_with_message( buff.st_mode == TEST_GFAL_LFC_FILE_STAT_MODE_VALUE &&
		buff.st_uid==TEST_GFAL_LFC_FILE_STAT_UID_VALUE &&
		buff.st_gid==TEST_GFAL_LFC_FILE_STAT_GID_VALUE,
		" this is not the correct value for the lfc stat mode %o, uid %d, gid %d, size %d", 
								buff.st_mode, buff.st_uid, buff.st_gid, buff.st_size);
	
	gfal_posix_clear_error();
	res = gfal_stat(TEST_GFAL_LFC_FILE_STAT_NONEXIST, &buff);
	assert_true_with_message( res == -1 && gfal_posix_code_error() == ENOENT && errno== ENOENT, " must be an invalid stat");
	gfal_posix_clear_error();	
}





void gfal2_test__gfal_posix_stat_guid()
{
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	if( gfal_check_GError(&mock_err))
		return;

	char* tab[]={ "/dtea/testmock", NULL};
	define_mock_linkinfos(1, tab);
	define_mock_filestatg(TEST_GFAL_LFC_FILE_STAT_MODE_VALUE, TEST_GFAL_LFC_FILE_STAT_GID_VALUE, TEST_GFAL_LFC_FILE_STAT_UID_VALUE);
	will_respond(lfc_mock_getlinks, 0, want_string(guid, TEST_GFAL_GUID_FILE_STAT_OK+5), want(path, NULL), want_non_null(nbentries), want_non_null(linkinfos));	
	will_respond(lfc_mock_statg, 0, want_string(path, tab[0]), want_non_null(linkinfos));
	will_respond(lfc_mock_getlinks, ENOENT, want_string(guid, TEST_GUID_NOEXIST_ACCESS+5), want(path, NULL), want_non_null(nbentries), want_non_null(linkinfos));	
	always_return(lfc_mock_getlinks, EINVAL);
#endif
	struct stat buff;
	memset(&buff,0, sizeof(struct stat));
	int res = gfal_stat(TEST_GFAL_GUID_FILE_STAT_OK, &buff);
	assert_true_with_message(res ==0 && errno==0 && gfal_posix_code_error()==0, " must be a success");
	
	assert_true_with_message( buff.st_mode == TEST_GFAL_LFC_FILE_STAT_MODE_VALUE &&
		buff.st_uid==TEST_GFAL_LFC_FILE_STAT_UID_VALUE &&
		buff.st_gid==TEST_GFAL_LFC_FILE_STAT_GID_VALUE,
		" this is not the correct value for the lfc stat mode %o, uid %d, gid %d, size %d", 
								buff.st_mode, buff.st_uid, buff.st_gid, buff.st_size);
	
	res = gfal_stat(TEST_GUID_NOEXIST_ACCESS, &buff);
	assert_true_with_message( res == -1 && gfal_posix_code_error() == ENOENT && errno== ENOENT, " must be an invalid stat %d %d %d", res, gfal_posix_code_error(), errno);
	gfal_posix_clear_error();	
}



void gfal2_test__gfal_posix_stat_local()
{
	struct stat buff;
	memset(&buff,0, sizeof(struct stat));
	
	// create a fil of the given size
	system(TEST_GFAL_LOCAL_STAT_COMMAND);
	errno=0;
	int res = gfal_stat(TEST_GFAL_LOCAL_STAT_OK, &buff);
	assert_true_with_message(res ==0 && errno==0 && gfal_posix_code_error()==0, " must be a success");
	
	assert_true_with_message( buff.st_mode == TEST_GFAL_LOCAL_FILE_STAT_MODE_VALUE,
		" this is not the correct value for the local stat mode %o, uid %d, gid %d, size %d", 
								buff.st_mode, buff.st_uid, buff.st_gid, buff.st_size);
	
	res = gfal_stat(TEST_GFAL_LOCAL_STAT_NONEXIST, &buff);
	assert_true_with_message( res == -1 && gfal_posix_code_error() == ENOENT && errno== ENOENT, " must be an invalid stat");
	gfal_posix_clear_error();			
}



void gfal2_test__gfal_posix_stat_srm()
{
	struct stat buff;
	memset(&buff,0, sizeof(struct stat));
#if USE_MOCK
	create_srm_stat_env_mock();	

#endif	
	int res = gfal_stat(TEST_GFAL_SRM_FILE_STAT_OK, &buff);
	assert_true_with_message(res ==0 && errno==0 && gfal_posix_code_error()==0, " must be a success %d %d %d", res, errno, gfal_posix_code_error());
	
	assert_true_with_message(buff.st_mode == TEST_GFAL_SRM_FILE_STAT_MODE_VALUE 
		&& buff.st_uid == TEST_GFAL_SRM_FILE_STAT_UID_VALUE
		&& buff.st_gid == TEST_GFAL_SRM_FILE_STAT_GID_VALUE,
		" this is not the correct value for the srm stat mode %o, uid %d, gid %d, size %d", 
								buff.st_mode, buff.st_uid, buff.st_gid, buff.st_size);
	gfal_posix_check_error();

#if USE_MOCK
	create_srm_stat_env_mock_noent();
#endif	
	res = gfal_stat(TEST_GFAL_SRM_FILE_STAT_NONEXIST, &buff);
	assert_true_with_message( res == -1 && gfal_posix_code_error() == ENOENT && errno== ENOENT, " must be an invalid stat");
	gfal_posix_clear_error();	
	
}




void gfal2_test__gfal_posix_lstat_lfc()
{
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	if( gfal_check_GError(&mock_err))
		return;

	char* tab[]={ "/dtea/testmock", NULL};
	define_mock_filelstat(TEST_GFAL_LFC_FILE_STAT_MODE_VALUE, TEST_GFAL_LFC_FILE_STAT_GID_VALUE, TEST_GFAL_LFC_FILE_STAT_UID_VALUE);
	will_respond(lfc_mock_lstatg, 0, want_string(path, tab[0]), want_non_null(linkinfos));
#endif
	struct stat buff;
	memset(&buff,0, sizeof(struct stat));
	int res = gfal_lstat(TEST_GFAL_LFC_FILE_STAT_OK, &buff);
	assert_true_with_message(res ==0 && errno==0 && gfal_posix_code_error()==0, " must be a success stat %d %d %d", res, errno, gfal_posix_code_error());
	
	assert_true_with_message( buff.st_mode == TEST_GFAL_LFC_FILE_STAT_MODE_VALUE &&
		buff.st_uid==TEST_GFAL_LFC_FILE_STAT_UID_VALUE &&
		buff.st_gid==TEST_GFAL_LFC_FILE_STAT_GID_VALUE,
		" this is not the correct value for the lfc stat mode %o, uid %d, gid %d, size %d", 
								buff.st_mode, buff.st_uid, buff.st_gid, buff.st_size);
	gfal_posix_clear_error();
	
#if USE_MOCK
	define_mock_filelstat(TEST_GFAL_LFC_LINK_STAT_MODE_VALUE, TEST_GFAL_LFC_LINK_STAT_GID_VALUE, TEST_GFAL_LFC_LINK_STAT_UID_VALUE);
	will_respond(lfc_mock_lstatg, 0, want_string(path, tab[0]), want_non_null(linkinfos));
	will_respond(lfc_mock_lstatg, ENOENT, want_string(path, tab[0]), want_non_null(linkinfos));
#endif	

	res = gfal_lstat(TEST_GFAL_LFC_LINK_STAT_OK, &buff);
	assert_true_with_message(res ==0 && errno==0 && gfal_posix_code_error()==0, " must be a success lstat %d %d %d", res, errno, gfal_posix_code_error());
	
	assert_true_with_message( buff.st_mode == TEST_GFAL_LFC_LINK_STAT_MODE_VALUE &&
		buff.st_uid==TEST_GFAL_LFC_LINK_STAT_UID_VALUE &&
		buff.st_gid==TEST_GFAL_LFC_LINK_STAT_GID_VALUE,
		" this is not the correct value for the lfc stat mode %o, uid %d, gid %d, size %d", 
								buff.st_mode, buff.st_uid, buff.st_gid, buff.st_size);
	
	res = gfal_lstat(TEST_GFAL_LFC_FILE_STAT_NONEXIST, &buff);
	assert_true_with_message( res == -1 && gfal_posix_code_error() == ENOENT && errno== ENOENT, " must be an invalid stat %d %d %d ", res, gfal_posix_code_error(), errno);
	gfal_posix_clear_error();		
}




void gfal2_test__gfal_posix_lstat_guid()
{
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_plugins_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	if( gfal_check_GError(&mock_err))
		return;

	char* tab[]={ "/dtea/testmock", NULL};
	define_mock_linkinfos(1, tab);
	define_mock_filelstat(TEST_GFAL_LFC_FILE_STAT_MODE_VALUE, TEST_GFAL_LFC_FILE_STAT_GID_VALUE, TEST_GFAL_LFC_FILE_STAT_UID_VALUE);
	will_respond(lfc_mock_getlinks, 0, want_string(guid, TEST_GFAL_GUID_FILE_STAT_OK+5), want(path, NULL), want_non_null(nbentries), want_non_null(linkinfos));	
	will_respond(lfc_mock_lstatg, 0, want_string(path, tab[0]), want_non_null(linkinfos));
	will_respond(lfc_mock_getlinks, ENOENT, want_string(guid, TEST_GUID_NOEXIST_ACCESS+5), want(path, NULL), want_non_null(nbentries), want_non_null(linkinfos));	
	always_return(lfc_mock_getlinks, EINVAL);
#endif
	struct stat buff;
	memset(&buff,0, sizeof(struct stat));
	int res = gfal_lstat(TEST_GFAL_GUID_FILE_STAT_OK, &buff);
	assert_true_with_message(res ==0 && errno==0 && gfal_posix_code_error()==0, " must be a success %d %d %d", res, errno, gfal_posix_code_error());
	
	assert_true_with_message( buff.st_mode == TEST_GFAL_LFC_FILE_STAT_MODE_VALUE &&
		buff.st_uid==TEST_GFAL_LFC_FILE_STAT_UID_VALUE &&
		buff.st_gid==TEST_GFAL_LFC_FILE_STAT_GID_VALUE,
		" this is not the correct value for the lfc stat mode %o, uid %d, gid %d, size %d", 
								buff.st_mode, buff.st_uid, buff.st_gid, buff.st_size);
	
	gfal_posix_clear_error();
	
	res = gfal_lstat(TEST_GUID_NOEXIST_ACCESS, &buff);
	assert_true_with_message( res == -1 && gfal_posix_code_error() == ENOENT && errno== ENOENT, " must be an invalid stat %d %d %d ", res, gfal_posix_code_error(), errno);
	gfal_posix_clear_error();	
}



void gfal2_test__gfal_posix_lstat_local()
{
	errno =0;
	struct stat buff;
	memset(&buff,0, sizeof(struct stat));
	system(TEST_GFAL_LOCAL_STAT_COMMAND);
	system(TEST_GFAL_LOCAL_LINK_COMMAND);
	int res = gfal_lstat(TEST_GFAL_LOCAL_STAT_OK, &buff);
	assert_true_with_message(res ==0 && errno==0 && gfal_posix_code_error()==0, " must be a success %d %d %d", res, errno, gfal_posix_code_error());
	gfal_posix_clear_error();
	
	
	res = gfal_lstat(TEST_GFAL_LOCAL_LINK_OK, &buff);
	assert_true_with_message(res ==0 && errno==0 && gfal_posix_code_error()==0, " must be a success %d %d %d", res, errno, gfal_posix_code_error());
	gfal_posix_clear_error();
	
	res = gfal_lstat(TEST_GFAL_LOCAL_STAT_NONEXIST, &buff);
	assert_true_with_message( res == -1 && gfal_posix_code_error() == ENOENT && errno== ENOENT, " must be an invalid stat");
	gfal_posix_clear_error();	
	gfal_posix_clear_error();	
}




void gfal2_test__gfal_posix_lstat_srm()
{
	struct stat buff;
	memset(&buff,0, sizeof(struct stat));
#if USE_MOCK
	create_srm_stat_env_mock();	

#endif	
	int res = gfal_lstat(TEST_GFAL_SRM_FILE_STAT_OK, &buff);
	assert_true_with_message(res ==0 && errno==0 && gfal_posix_code_error()==0, " must be a success %d %d %d", res, errno, gfal_posix_code_error());
	
	assert_true_with_message(buff.st_mode == TEST_GFAL_SRM_FILE_STAT_MODE_VALUE 
		&& buff.st_uid == TEST_GFAL_SRM_FILE_STAT_UID_VALUE
		&& buff.st_gid == TEST_GFAL_SRM_FILE_STAT_GID_VALUE,
		"this is not the correct value for the srm stat mode %o, uid %d, gid %d, size %d", 
								buff.st_mode, buff.st_uid, buff.st_gid, buff.st_size);
	gfal_posix_check_error();

#if USE_MOCK
	create_srm_stat_env_mock_noent();
#endif	
	res = gfal_lstat(TEST_GFAL_SRM_FILE_STAT_NONEXIST, &buff);
	assert_true_with_message( res == -1 && gfal_posix_code_error() == ENOENT && errno== ENOENT, " must be an invalid lstat");
	gfal_posix_clear_error();	
}
