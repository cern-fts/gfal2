
/* unit test for posix access func */


#include <cgreen/cgreen.h>
#include <string.h>
#include <unistd.h>
#include "../../src/common/gfal_prototypes.h"
#include "../../src/common/gfal_types.h"
#include "../../src/common/gfal_constants.h"
#include "../unit_test_constants.h"
#include <stdio.h>
#include "gfal_posix_api.h"
#include <errno.h>


void test__gfal_posix_rename_plugin()
{
	int res = gfal_access(TEST_LFC_RENAME_VALID_SRC, F_OK);
	if(res !=0){
		assert_true_with_message(FALSE, "source file not present on the lfc");
		gfal_posix_release_error();
		return;
	}
	res = gfal_rename(TEST_LFC_RENAME_VALID_SRC, TEST_LFC_RENAME_VALID_DEST);
	if(res !=0){
		assert_true_with_message(FALSE, " this move file must be a success");
		gfal_posix_release_error();
		return;
	}
	res = gfal_access(TEST_LFC_RENAME_VALID_DEST, F_OK);
	if(res != 0){
		assert_true_with_message(FALSE, "dest file must be present if corrctly moved");
		gfal_posix_release_error();
		return;
	}
	res = gfal_rename( TEST_LFC_RENAME_VALID_DEST, TEST_LFC_RENAME_VALID_SRC);
	if(res !=0){
		assert_true_with_message(FALSE, " this move file must be a success");
		gfal_posix_release_error();
		return;
	}
	res = gfal_access(TEST_LFC_RENAME_VALID_SRC, F_OK);
	if(res !=0){
		assert_true_with_message(FALSE, "the soruce file must be in the init position");
		gfal_posix_release_error();
		return;
	}
}


void test__gfal_posix_move_dir_plugin()
{
	int res = gfal_access(TEST_LFC_MOVABLE_DIR_SRC, F_OK);
	if(res !=0){
		assert_true_with_message(FALSE, "source dir not present on the lfc");
		gfal_posix_release_error();
		return;
	}
	res = gfal_rename(TEST_LFC_MOVABLE_DIR_SRC, TEST_LFC_MOVABLE_DIR_DEST);
	if(res !=0){
		assert_true_with_message(FALSE, " this move dir must be a success");
		gfal_posix_release_error();
		return;
	}	
	res = gfal_access(TEST_LFC_MOVABLE_DIR_DEST, F_OK);
	if(res !=0){
		assert_true_with_message(FALSE, "dest dir not present on the lfc");
		gfal_posix_release_error();
		return;
	}	
	res = gfal_rename(TEST_LFC_MOVABLE_DIR_DEST, TEST_LFC_MOVABLE_DIR_SRC );
	if(res !=0){
		assert_true_with_message(FALSE, " this move dir must be a success");
		gfal_posix_release_error();
		return;
	}		
	res = gfal_access(TEST_LFC_MOVABLE_DIR_SRC, F_OK);
	if(res !=0){
		assert_true_with_message(FALSE, "source dir not present on the lfc after move");
		gfal_posix_release_error();
		return;
	}		
}



void test__gfal_posix_rename_url_check()
{
	int res = gfal_rename( TEST_LFC_NOEXIST_ACCESS, TEST_LFC_RENAME_VALID_DEST);
	if(res ==0 || errno != ENOENT || gfal_posix_code_error() != ENOENT){
		assert_true_with_message(FALSE, "this move dir cannot succed, src url does not exist");
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();		
	res = gfal_rename( TEST_LFC_NOEXIST_ACCESS, "google.com");
	if(res ==0 || errno != EPROTONOSUPPORT || gfal_posix_code_error() != EPROTONOSUPPORT){
		assert_true_with_message(FALSE, "unknow protocol, must fail");
		gfal_posix_release_error();
		return;
	}	
}


void test__gfal_posix_rename_local()
{
		// create local file
	const char * msg = "hello";
	char nfile[500], nfile2[500];
	strcpy(nfile, "file://");
	strcpy(nfile2, "file://");
	FILE* f = fopen(TEST_GFAL_LOCAL_FILE_RENAME_SRC, "w+");
	if(f == NULL){
		assert_true_with_message(FALSE, " file must be created");
		return;
	}
	fwrite(msg, sizeof(char), 5, f);
	fclose(f);
	strcat(nfile,TEST_GFAL_LOCAL_FILE_RENAME_SRC);
	strcat(nfile2, TEST_GFAL_LOCAL_FILE_RENAME_DEST); // create two full file url file:///

	int res = gfal_access(nfile, F_OK);
	if(res !=0){
		assert_true_with_message(FALSE, "src file not present ");
		gfal_posix_release_error();
		return;
	}	
	
	res = gfal_rename(nfile, nfile2);
	if(res !=0){
		assert_true_with_message(FALSE, "must be a valid rename");
		gfal_posix_release_error();
		return;
	}
	res = gfal_access(nfile2, F_OK);
	if(res !=0){
		assert_true_with_message(FALSE, "dst file not present ");
		gfal_posix_release_error();
		return;
	}	
	res = gfal_rename(nfile2, nfile);
	if(res !=0){
		assert_true_with_message(FALSE, "must be a valid reverse-rename");
		gfal_posix_release_error();
		return;
	}
	res = gfal_access(nfile, F_OK);
	if(res !=0){
		assert_true_with_message(FALSE, "src file is not present as initial ");
		gfal_posix_release_error();
		return;
	}			
			
	
	
}
