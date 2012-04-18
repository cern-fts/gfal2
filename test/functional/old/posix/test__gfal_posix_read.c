
/* unit test for posix open func */

#include <stdio.h>
#include <errno.h>
#include <cgreen/cgreen.h>
#include <common/gfal_constants.h>
#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>

#include <posix/gfal_posix_api.h>

#include "../mock/gfal_rfio_mock_test.h"
#include "../unit_test_constants.h"
#include "test__gfal_posix_open.h"



void test_mock_read_posix_srm(const char* content){
#if USE_MOCK
	char* tab[]= { TEST_SRM_VALID_SURL_EXAMPLE1, NULL };	
	char* tab_turl[] = { TEST_SRM_TURL_EXAMPLE1, NULL };
	int res[] = { 0, 0 };
	strcpy(defined_buff_read, content);
	defined_buff_read_size = strlen(content)+1;
	will_respond(rfio_mock_read, strlen(content), want_non_null(buff), want_non_null(size), want_non_null(fd));
	test_mock_srm_open_valid(tab, tab_turl, res);
#endif
}

void test_mock_pread_posix_srm(const char* content, off_t off){
#if USE_MOCK
	char* tab[]= { TEST_SRM_VALID_SURL_EXAMPLE1, NULL };	
	char* tab_turl[] = { TEST_SRM_TURL_EXAMPLE1, NULL };
	int res[] = { 0, 0 };
	strcpy(defined_buff_read, content);
	defined_buff_read_size = strlen(content)+1;
	will_respond(rfio_mock_read, strlen(content), want_non_null(buff), want_non_null(size), want_non_null(fd));
	will_respond(rfio_mock_lseek, off, want_non_null(fd), want(offset, off), want(whence, SEEK_SET));
	test_mock_srm_open_valid(tab, tab_turl, res);
#endif
}

void test_mock_read_posix_lfc(const char * url, const char* filename){
#if USE_MOCK
	strcpy(defined_buff_read, filename);
	defined_buff_read_size = strlen(filename)+1;
	will_respond(rfio_mock_read, strlen(filename), want_non_null(buff), want_non_null(size), want_non_null(fd));
	test_mock_lfc_open_valid(url);
#endif
}

void test_mock_read_posix_guid(const char* guid1, const char* filename){
#if USE_MOCK
	strcpy(defined_buff_read, filename);
	defined_buff_read_size = strlen(filename)+1;	
	will_respond(rfio_mock_read, strlen(filename), want_non_null(buff), want_non_null(size), want_non_null(fd));
	test_mock_guid_open_valid(guid1);
#endif
	
}

void test_generic_read_simple(char* url_exist, const char* filename){
	char buff[2048];
	int ret = -1;
	errno=0;
	int fd = gfal_open(url_exist, O_RDONLY, 555);
	assert_true_with_message(fd >0 && gfal_posix_code_error()==0 && errno==0, " must be a valid open for read %d %d %d", ret, gfal_posix_code_error(), errno);
	gfal_posix_check_error();
	
	ret = gfal_read(fd, buff, strlen(TEST_SRM_FILE_CONTENT)+1);
	assert_true_with_message(ret >0 && gfal_posix_code_error()==0 && errno==0, " must be a valid read %d %d %d", ret, gfal_posix_code_error(), errno);
	assert_true_with_message(strcmp(filename, buff)==0, " must be the content of the file");
	gfal_posix_check_error();
	ret = gfal_close(fd);
	assert_true_with_message(fd !=0 && ret==0 && gfal_posix_code_error()==0 && errno==0, " must be a valid close %d %d %d", ret, gfal_posix_code_error(), errno);
	gfal_posix_check_error();	
	ret = gfal_close(fd);
	assert_true_with_message( ret==-1 && gfal_posix_code_error()==EBADF && errno==EBADF, " must be a bad descriptor %d %d %d", ret, gfal_posix_code_error(), errno);

}

// same but for vector read
void test_generic_pread_simple(char* url_exist, const char* filename){
	char buff[2048];
	int ret = -1;
	errno=0;
	int fd = gfal_open(url_exist, O_RDONLY, 555);
	assert_true_with_message(fd >0 && gfal_posix_code_error()==0 && errno==0, " must be a valid open for pread %d %d %d", ret, gfal_posix_code_error(), errno);
	gfal_posix_check_error();
	
	ret = gfal_pread(fd, buff, strlen(TEST_SRM_FILE_CONTENT)+1, 0);
	assert_true_with_message(ret >0 && gfal_posix_code_error()==0 && errno==0, " must be a valid pread %d %d %d", ret, gfal_posix_code_error(), errno);
	assert_true_with_message(strcmp(filename, buff)==0, " must be the content of the file");
	gfal_posix_check_error();
	ret = gfal_close(fd);
	assert_true_with_message(fd !=0 && ret==0 && gfal_posix_code_error()==0 && errno==0, " must be a valid close %d %d %d", ret, gfal_posix_code_error(), errno);
	gfal_posix_check_error();	
	ret = gfal_close(fd);
	assert_true_with_message( ret==-1 && gfal_posix_code_error()==EBADF && errno==EBADF, " must be a bad descriptor %d %d %d", ret, gfal_posix_code_error(), errno);

}



void gfal2_test_read_posix_local_simple()
{
	system(TEST_LOCAL_OPEN_CREATE_COMMAND);
	test_generic_read_simple(TEST_LOCAL_OPEN_EXIST, TEST_LOCAL_READ_CONTENT);

}

void gfal2_test_pread_posix_local_simple()
{
	system(TEST_LOCAL_OPEN_CREATE_COMMAND);
	test_generic_pread_simple(TEST_LOCAL_OPEN_EXIST, TEST_LOCAL_READ_CONTENT);

}

void gfal2_test_read_posix_srm_simple(){
	test_mock_read_posix_srm(TEST_SRM_FILE_CONTENT);
	test_generic_read_simple(TEST_SRM_ONLY_READ_HELLO, TEST_SRM_FILE_CONTENT);	
}

void gfal2_test_pread_posix_srm_simple(){
	test_mock_read_posix_srm(TEST_SRM_FILE_CONTENT);
	test_generic_pread_simple(TEST_SRM_ONLY_READ_HELLO, TEST_SRM_FILE_CONTENT);	
}


void gfal2_test_read_posix_lfc_simple()
{
	test_mock_read_posix_lfc(TEST_LFC_ONLY_READ_ACCESS, TEST_SRM_FILE_CONTENT);
	test_generic_read_simple(TEST_LFC_ONLY_READ_ACCESS, TEST_SRM_FILE_CONTENT);
}

void gfal2_test_pread_posix_lfc_simple()
{
	test_mock_read_posix_lfc(TEST_LFC_ONLY_READ_ACCESS, TEST_SRM_FILE_CONTENT);
	test_generic_pread_simple(TEST_LFC_ONLY_READ_ACCESS, TEST_SRM_FILE_CONTENT);
}




void gfal2_test_read_posix_guid_simple(){
	test_mock_read_posix_guid(TEST_GUID_ONLY_READ_ACCESS, TEST_SRM_FILE_CONTENT);
	test_generic_read_simple(TEST_GUID_ONLY_READ_ACCESS, TEST_SRM_FILE_CONTENT);		
}


void gfal2_test_pread_posix_guid_simple(){
	test_mock_read_posix_guid(TEST_GUID_ONLY_READ_ACCESS, TEST_SRM_FILE_CONTENT);
	test_generic_pread_simple(TEST_GUID_ONLY_READ_ACCESS, TEST_SRM_FILE_CONTENT);	
	
}






