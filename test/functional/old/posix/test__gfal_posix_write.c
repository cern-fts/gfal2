
/* unit test for posix open func */

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <cgreen/cgreen.h>

#include "../../src/common/gfal_prototypes.h"
#include "../../src/common/gfal_types.h"
#include "../../src/common/gfal_constants.h"
#include "../../src/common/gfal_common_errverbose.h"
#include "../mock/gfal_rfio_mock_test.h"
#include "../unit_test_constants.h"
#include "test__gfal_posix_read.h"
#include "test__gfal_posix_open.h"
#include "../common/gfal__test_common_srm.h"
#include "gfal_posix_api.h"





void test_mock_write_posix_srm(const char * url, const char* filename){
#if USE_MOCK


	strcpy(defined_buff_write, filename);
	defined_buff_write_size = strlen(filename)+1;
	will_respond(rfio_mock_write, strlen(filename), want_non_null(buff), want_non_null(size), want_non_null(fd));
	char* turls[]= { "rfio://mocked_turl_nothing", NULL };
	char* surls[] = { (char*)url, NULL };
	int  status[] = { 0,0 };
	test_mock_srm_open_write_valid(surls, turls, status);
#endif
}

void test_mock_pwrite_posix_srm(const char * url, const char* filename, off_t off){
#if USE_MOCK


	strcpy(defined_buff_write, filename);
	defined_buff_write_size = strlen(filename)+1;
	will_respond(rfio_mock_lseek, off, want_non_null(fd), want(offset, off), want(whence, SEEK_SET));
	will_respond(rfio_mock_write, strlen(filename), want_non_null(buff), want_non_null(size), want_non_null(fd));
	char* turls[]= { "rfio://mocked_turl_nothing", NULL };
	char* surls[] = { (char*)url, NULL };
	int  status[] = { 0,0 };
	test_mock_srm_open_write_valid(surls, turls, status);
#endif
}



void test_generic_write_simple(char* url_exist, const char* filename){
	char buff[2048];
	int ret = -1;
	errno=0;
	strcpy(buff, TEST_SRM_FILE_CONTENT);
	//g_printerr("filename write : %s ",url_exist);
	int fd = gfal_open(url_exist, O_WRONLY | O_CREAT, 0644);
	assert_true_with_message(fd >0 && gfal_posix_code_error()==0 && errno==0, " must be a valid open for write %d %d %d", ret, gfal_posix_code_error(), errno);
	gfal_posix_check_error();
	
	ret = gfal_write(fd, buff, strlen(TEST_SRM_FILE_CONTENT)+1);
	assert_true_with_message(ret >0 && gfal_posix_code_error()==0 && errno==0, " must be a valid write %d %d %d", ret, gfal_posix_code_error(), errno);
	ret = gfal_close(fd);
	assert_true_with_message(fd !=0 && ret==0 && gfal_posix_code_error()==0 && errno==0, " must be a valid close %d %d %d", ret, gfal_posix_code_error(), errno);	

}


void test_generic_pwrite_simple(char* url_exist, const char* filename){
	char buff[2048];
	int ret = -1;
	errno=0;
	strcpy(buff, TEST_SRM_FILE_CONTENT);
	//g_printerr("filename write : %s ",url_exist);
	int fd = gfal_open(url_exist, O_WRONLY | O_CREAT, 0644);
	assert_true_with_message(fd >0 && gfal_posix_code_error()==0 && errno==0, " must be a valid open for write %d %d %d", ret, gfal_posix_code_error(), errno);
	gfal_posix_check_error();
	
	ret = gfal_pwrite(fd, buff, strlen(TEST_SRM_FILE_CONTENT)+1, 0);
	assert_true_with_message(ret >0 && gfal_posix_code_error()==0 && errno==0, " must be a valid write %d %d %d", ret, gfal_posix_code_error(), errno);
	ret = gfal_close(fd);
	assert_true_with_message(fd !=0 && ret==0 && gfal_posix_code_error()==0 && errno==0, " must be a valid close %d %d %d", ret, gfal_posix_code_error(), errno);	

}



void gfal2_test_write_posix_local_simple()
{

	test_generic_write_simple(TEST_LOCAL_WRITE_VALID, TEST_LOCAL_READ_CONTENT);
	test_generic_read_simple(TEST_LOCAL_WRITE_VALID, TEST_LOCAL_READ_CONTENT);
	system(TEST_LOCAL_WRITE_CREATE_COMMAND);
}


void gfal2_test_pwrite_posix_local_simple()
{

	test_generic_pwrite_simple(TEST_LOCAL_WRITE_VALID, TEST_LOCAL_READ_CONTENT);
	test_generic_pread_simple(TEST_LOCAL_WRITE_VALID, TEST_LOCAL_READ_CONTENT);
	system(TEST_LOCAL_WRITE_CREATE_COMMAND);
}


void gfal2_test_write_posix_srm_simple()
{
	setup_mock_srm();
	char buff_name[2048];
	time_t t = time(NULL);
	int i = rand();
	snprintf(buff_name, 2048, "%stest_%ld_%d", TEST_SRM_DPM_ENDPOINT_PREFIX, (long) t, (int) i);
	test_mock_write_posix_srm(buff_name, TEST_LOCAL_READ_CONTENT);
	test_generic_write_simple(buff_name, TEST_LOCAL_READ_CONTENT);
	test_mock_read_posix_srm(TEST_LOCAL_READ_CONTENT);
	test_generic_read_simple(buff_name, TEST_LOCAL_READ_CONTENT);
}


void gfal2_test_pwrite_posix_srm_simple()
{
	setup_mock_srm();
	char buff_name[2048];
	time_t t = time(NULL);
	int i = rand();
	snprintf(buff_name, 2048, "%stest_%ld_%d", TEST_SRM_DPM_ENDPOINT_PREFIX, (long) t, (int) i);
	test_mock_pwrite_posix_srm(buff_name, TEST_LOCAL_READ_CONTENT,0 );
	test_generic_pwrite_simple(buff_name, TEST_LOCAL_READ_CONTENT);
	test_mock_pread_posix_srm(TEST_LOCAL_READ_CONTENT, 0);
	test_generic_pread_simple(buff_name, TEST_LOCAL_READ_CONTENT);
}


void gfal2_test_write_posix_lfc_simple()
{
	/*test_mock_write_posix_lfc(TEST_LFC_OPEN_WRITE, TEST_SRM_FILE_CONTENT); --> Not able to write on the lfc for the moment
	test_generic_write_simple(TEST_LFC_OPEN_WRITE, TEST_SRM_FILE_CONTENT);
	test_mock_read_posix_lfc(TEST_LFC_OPEN_WRITE,TEST_SRM_FILE_CONTENT);
	test_generic_read_simple(TEST_LFC_OPEN_WRITE, TEST_SRM_FILE_CONTENT);*/
}

void test_write_posix_guid_simple(){
	/*test_mock_write_posix_lfc(TEST_LFC_OPEN_WRITE, TEST_SRM_FILE_CONTENT); --> Not able to write on the lfc with guid for the moment
	test_generic_write_simple(TEST_LFC_OPEN_WRITE, TEST_SRM_FILE_CONTENT);
	test_mock_read_posix_lfc(TEST_LFC_OPEN_WRITE,TEST_SRM_FILE_CONTENT);
	test_generic_read_simple(TEST_LFC_OPEN_WRITE, TEST_SRM_FILE_CONTENT);*/
	
}






