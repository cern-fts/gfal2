/**
 * Compile command : gcc gfal_test_readdir_full.c `pkg-config --libs --cflags gfal2`
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <glib.h>
#include <string.h>
#include <gfal_api.h>

#include <common/gfal_lib_test.h>

int main(int argc, char **argv)
{
	char *file;
	char base_dir[2048];
	char nested_dir[2048];
    int count=0;
    GError * tmp_err=NULL;
    struct dirent* d;
    struct stat st;

	if (argc < 2) {
		fprintf (stderr, "usage: %s dir_base \n", argv[0]);
		exit (1);
	}
	
	gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_DEBUG | GFAL_VERBOSE_VERBOSE);
		
	srand(time(NULL));
	file = argv[1];

    generate_random_uri(file, "readdir_full_entry_enoent", base_dir, 2048);
	printf(" try to list not existing dir .... %s", base_dir);
	DIR* p;
	if( (p = gfal_opendir(base_dir)) != NULL){
		g_assert_not_reached();	
	}
	g_assert( gfal_posix_code_error() == ENOENT);
	gfal_posix_clear_error();
	gfal_closedir(p); // must pass without problem even if useless
	gfal_posix_clear_error();
	
    generate_random_uri(file, "testlistdir_", base_dir, 2048);
	printf(" create a new dir for content listing .... %s\n", base_dir);
	if(gfal_mkdir(base_dir, 0777) !=0){
		gfal_posix_check_error();
		g_assert_not_reached();
	}
	printf(" create content of the new dir .... \n");		
	int i, n = (rand()%20)+2;
    for (i = 0; i < n; ++i) {
	    snprintf(nested_dir, sizeof(nested_dir), "%s/nested_elem_%d", base_dir, i);
		printf("     create one elem in the new dir %s \n", nested_dir);
		if(gfal_mkdir(nested_dir, 0777) !=0){
			gfal_posix_check_error();
			g_assert_not_reached();
		}
	}
	
    //// simple opendir
	if((p = gfal_opendir(base_dir) ) == NULL){
		gfal_posix_check_error();		
		g_assert_not_reached();	
	}
	
	while( ( d= gfal_readdir(p))){
		g_assert( gfal_posix_code_error() == 0);
		g_assert(  (strstr(d->d_name, "elem_") != NULL) ||
				 *d->d_name == '.');
		printf(" +1 iterate %s", d->d_name);
		g_assert(strchr(d->d_name, '/') == NULL);
		count++;

	}
	printf(" count : %d", count);
	g_assert( count >= n && count < n+3); // take care of .. and . if available
	
	g_assert( gfal_closedir(p) == 0);
	g_assert( gfal_posix_code_error() == 0);

    //// simple opendir
    if((p = gfal_opendir(base_dir) ) == NULL){
        gfal_posix_check_error();
        g_assert_not_reached();
    }

    count=0;
    d= NULL;

    while( ( d= gfal_readdir(p))){
        g_assert( gfal_posix_code_error() == 0);
        g_assert(  (strstr(d->d_name,"elem_") != NULL) ||
                 *d->d_name == '.');
        printf(" +1 iterate %s", d->d_name);
        g_assert(strchr(d->d_name, '/') == NULL);
        count++;

    }
    printf(" count : %d", count);
    g_assert( count >= n && count < n+3); // take care of .. and . if available

    g_assert( gfal_closedir(p) == 0);


   //// advanced opendir pp

    if((p = gfal_opendir(base_dir) ) == NULL){
        gfal_posix_check_error();
        g_assert_not_reached();
    }

    count = 0;
    d = NULL;

    while( ( d= gfal2_readdirpp(gfal_posix_get_context(), p, &st, &tmp_err))){
        g_assert( gfal_posix_code_error() == 0);
        g_assert(tmp_err == NULL);
        g_assert(  (strstr(d->d_name,"elem_") != NULL) ||
                 *d->d_name == '.');
        printf(" +1 iterate %s", d->d_name);
        g_assert(strchr(d->d_name, '/') == NULL);
        g_assert(S_ISDIR(st.st_mode));
        count++;

    }
    printf(" count : %d\n", count);
    g_assert( tmp_err == NULL ); // take care of .. and . if available
    g_assert( count >= n && count < n+3); // take care of .. and . if available

    g_assert( gfal_closedir(p) == 0);
	
	gfal_closedir(p); // do it again for crash test


	for (i = 0; i < n; ++i) {
	    snprintf(nested_dir, sizeof(nested_dir), "%s/nested_elem_%d", base_dir, i);
	    printf("     rmdir %s \n", nested_dir);
	    g_assert(gfal_rmdir(nested_dir) == 0);
	}
	printf("     rmdir %s \n", base_dir);
	g_assert(gfal_rmdir(base_dir) == 0);

	printf ("All is ok.\n");
	exit (0);
}
