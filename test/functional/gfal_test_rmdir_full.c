/**
 * Compile command : gcc gfal_test_rmdir_full.c `pkg-config --libs --cflags gfal2`
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <glib.h>
#include <gfal_api.h>

int main(int argc, char **argv)
{
	char *valid_file,* valid_dir;
	char buff[2048];
	char buff2[2048];
	
	if (argc < 3) {
		fprintf (stderr, "usage: %s validdir valid_file \n", argv[0]);
		exit (1);
	}

	valid_file = argv[2];
	valid_dir = argv[1];
	
	gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_DEBUG | GFAL_VERBOSE_VERBOSE);

	// create a enoent file 
	g_strlcpy(buff, valid_dir, 2048);
	g_strlcat(buff, "/", 2048);	
	g_strlcat(buff, "klfdsklmfdsklmfdsklmfdsklmfds_enoent", 2048);	
	

	printf ("check enoent directory  %s ...\n", buff);	
	if(gfal_rmdir(buff) == 0) { // must not exist
		gfal_posix_check_error();
		g_assert_not_reached();
	}	
	printf ("enoent code : %d  ...\n", gfal_posix_code_error());			
	g_assert(gfal_posix_code_error() == ENOENT);
	gfal_posix_clear_error();
	g_assert(gfal_posix_code_error() == 0);	
	
	snprintf(buff, 2048, "%s/%ld_testrmdir", valid_dir,time(NULL));	
	printf ("create and delete directory  %s ...\n", buff);		
	if(gfal_mkdir(buff, 0777) != 0) {
		gfal_posix_check_error();
		g_assert_not_reached();
	}		
	g_assert(gfal_posix_code_error() == 0);	
	if(gfal_rmdir(buff) != 0) { 
		gfal_posix_check_error();
		g_assert_not_reached();
	}		
	g_assert(gfal_posix_code_error() == 0);	
	
	
	snprintf(buff, 2048, "%s/%ld_testrmdir/", valid_dir, time(NULL));
	printf ("create and delete directory with slash  %s   ...\n", buff);		
	if(gfal_mkdir(buff, 0777) != 0) {
		gfal_posix_check_error();
		g_assert_not_reached();
	}		
	g_assert(gfal_posix_code_error() == 0);	
	if(gfal_rmdir(buff) != 0) {
		gfal_posix_check_error();
		g_assert_not_reached();
	}		
	g_assert(gfal_posix_code_error() == 0);		
	
	snprintf(buff, 2048, "%s/%ld_testrmdir_eaccess", valid_dir, time(NULL));
	snprintf(buff2, 2048, "%s/testdirinside", buff);
	printf ("create eaccess and enotempty dir   %s  %s ...\n", buff, buff2);
	if(gfal_mkdir(buff, 0777) != 0) { 
		gfal_posix_check_error();
		g_assert_not_reached();
	}		
	g_assert(gfal_posix_code_error() == 0);	
	

	if(gfal_mkdir(buff2, 0777) != 0) { 
		gfal_posix_check_error();
		g_assert_not_reached();
	}		
	g_assert(gfal_posix_code_error() == 0);
	
	printf ("try enotempty   %s   ...\n", buff);		
	if(gfal_rmdir(buff) == 0) {  // must ENOTDIR
		gfal_posix_check_error();
		g_assert_not_reached();
	}
	printf(" err code enotempty : %d\n", gfal_posix_code_error());
	g_assert(gfal_posix_code_error() == ENOTEMPTY);
	gfal_posix_clear_error();
	g_assert(gfal_posix_code_error() == 0);			
		
	if(gfal_chmod(buff, 0000) != 0) { 
		gfal_posix_check_error();
		g_assert_not_reached();
	}		
	g_assert(gfal_posix_code_error() == 0);		
	
	// Skip this bit if file:// is used, since the rmdir will succeed
	if (strncmp("file:", buff2, 5) != 0) {
        printf ("try eacess %s   ...\n", buff2);
        if(gfal_rmdir(buff2) == 0) {  // must eaccess
            gfal_posix_check_error();
            g_assert_not_reached();
        }
        g_assert(gfal_posix_code_error() == EACCES);
        gfal_posix_clear_error();
        g_assert(gfal_posix_code_error() == 0);
	}
	else {
	    printf("skipping eacess test for file://\n");
	}
	
		
	if(gfal_rmdir(valid_file) == 0) {  // must ENOTDIR
		gfal_posix_check_error();
		g_assert_not_reached();
	}
	printf(" enotdir err : %d\n", gfal_posix_code_error())	;
	g_assert(gfal_posix_code_error() == ENOTDIR);
	gfal_posix_clear_error();
	g_assert(gfal_posix_code_error() == 0);	
	printf ("All is ok.\n");
	exit (0);
}
