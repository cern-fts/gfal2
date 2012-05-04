/**
 * Compile command : gcc gfal_test_mkdir_full.c `pkg-config --libs --cflags gfal2` `pkg-config --libs --cflags glib-2.0`
 */

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <math.h>
#include "gfal_api.h"

int main(int argc, char **argv)
{
	char *rootdir;
	char url[2048];
	int res = 0;

	if (argc != 2) {
		fprintf (stderr, "usage: %s [basedir_url] \n", argv[0]);
		exit (1);
	}

	
	rootdir = argv[1];
	snprintf(url, 2048, "%s/test_dir_%u_%u", rootdir, (unsigned int) g_random_int(), (unsigned int) g_random_int ());
	
	printf (" verify that the directory does not exist  .... \n");
	struct stat st;
	res = gfal_stat(url, &st);
	printf(" res %d errno %d errcode %d errstring \n", res , errno, gfal_posix_code_error());
	gfal_posix_print_error();
	g_assert( res != 0 && errno == ENOENT && gfal_posix_code_error() == ENOENT);
	errno = 0;
	gfal_posix_clear_error();
	
	
	printf ("Creating directory  %s ...\n", url);
	if ( (res =gfal_mkdir (url, 0777)) < 0) {
		gfal_posix_check_error();
	}
	g_assert( res == 0);
	g_assert( errno == 0);
	
	printf (" verify that the directory exists  .... \n");
	res = gfal_stat(url, &st);
	printf(" stat result : %d %d %d",res, errno, gfal_posix_code_error() );
	g_assert( res == 0 );
	g_assert(errno == 0);
	g_assert(gfal_posix_code_error() ==0);
	g_assert( st.st_mode & S_IFDIR );
	
	
	// should return eexist
	printf (" verify that the directory  one more call exists  too  .... \n");	
	res =gfal_mkdir (url, 0777);
	g_assert( res != 0);
	g_assert( errno == EEXIST);
	g_assert(gfal_posix_code_error() == EEXIST);	
	errno = 0;
	gfal_posix_clear_error();
	

	printf ("All is ok.\n");
	exit (0);
}

