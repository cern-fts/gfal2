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

int main(int argc, char **argv)
{
	char *file;
	char buff[2048];
	char buff2[2048];

	if (argc < 2) {
		fprintf (stderr, "usage: %s dir_base \n", argv[0]);
		exit (1);
	}
	
	gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_DEBUG | GFAL_VERBOSE_VERBOSE);
		
	srand(time(NULL));
	file = argv[1];

	snprintf(buff, 2048, "%s/fsdsdfkmklfsdmklmkl_enoent", file);
	printf(" try to list not existing dir .... %s", buff);
	DIR* p;
	if( (p = gfal_opendir(buff)) != NULL){
		g_assert_not_reached();	
	}
	g_assert( gfal_posix_code_error() == ENOENT);
	gfal_posix_clear_error();
	gfal_closedir(p); // must pass without problem even if useless
	gfal_posix_clear_error();
	
	snprintf(buff, 2048, "%s/testlistdir_%ld", file, time(NULL));
	printf(" create a new dir for content listing .... %s\n", buff);
	if(gfal_mkdir(buff, 0777) !=0){
		gfal_posix_check_error();
		g_assert_not_reached();
	}
	printf(" create content of the new dir .... \n");		
	int i, n = (rand()%20)+2;
	for(i=0; i < n; ++i){
		snprintf(buff2, 2048, "%s/elem_%d",buff,i);
		printf("     create one elem in the new dir %s \n", buff2);		
		if(gfal_mkdir(buff2, 0777) !=0){
			gfal_posix_check_error();
			g_assert_not_reached();
		}
	}
	
	if((p = gfal_opendir(buff) ) == NULL){
		gfal_posix_check_error();		
		g_assert_not_reached();	
	}
	
	int count=0;
	struct dirent* d;
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
	g_assert( gfal_posix_code_error() == 0);
	
	gfal_closedir(p); // do it again for crash test
	

	printf ("All is ok.\n");
	exit (0);
}
