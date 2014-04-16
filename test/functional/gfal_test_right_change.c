/**
 * Compile command : gcc gfal_test_right_change.c `pkg-config --libs --cflags gfal2`
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <glib.h>
#include <gfal_api.h>
#include <common/gfal_lib_test.h>

int main(int argc, char **argv)
{
	int i;
	mode_t mode;
	char *file;
	struct stat initial_stat, current_stat;
    char buff[2048];
	
	if (argc < 3) {
		fprintf (stderr, "usage: %s file mode1 mode2 mode3 mode4\n", argv[0]);
		exit (1);
	}

	file = argv[1];
	gfal_set_verbose(GFAL_VERBOSE_TRACE);

    generate_random_uri(file, "right_change_test_generated", buff, 2048);

    printf("Creating directory for right check %s \n", buff);
    if( gfal_mkdir(buff, 0755) < 0){
        gfal_posix_check_error();
        g_assert_not_reached();
    }

	for(i=2; i < argc; ++i){
		printf ("Checking initial right of %s  ...\n", buff);
        if (gfal_stat (buff, &initial_stat) < 0) {
			gfal_posix_check_error();
			g_assert_not_reached();
		}
	    printf ("initial right  0%o ...\n", initial_stat.st_mode);		

		mode = (mode_t) strtol (argv[i], NULL, 8);
		if (errno > 0) {
			perror ("strtol");
			exit (1);
		}
		printf ("Changing mode of '%s' to %o ...\n", buff, mode);
		
        if (gfal_chmod (buff, mode) < 0) {
			gfal_posix_check_error();
			g_assert_not_reached();
		}

        if (gfal_stat (buff, &current_stat) < 0) {
			gfal_posix_check_error();
			g_assert_not_reached();
		}		
	    printf ("checking the new right  0%o ...\n", current_stat.st_mode);	
	    
	    g_assert(mode == (current_stat.st_mode & 0777));
	    
	    printf(" rolling back the change ....");  
        if (gfal_chmod (buff, initial_stat.st_mode) < 0) {
			gfal_posix_check_error();
			g_assert_not_reached();			
		}
		
        if (gfal_stat (buff, &current_stat) < 0) {
			gfal_posix_check_error();
			g_assert_not_reached();
		}			
		printf ("checking the rollback  0%o ...\n", current_stat.st_mode);	
		g_assert(current_stat.st_mode == initial_stat.st_mode);		    
	}

	g_assert(gfal_rmdir(buff) == 0);

	printf ("All is ok.\n");
	return 0;
}
