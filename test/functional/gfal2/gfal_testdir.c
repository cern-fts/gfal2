/**
 * Compile command : gcc gfal_testdir.c `pkg-config --libs --cflags gfal2`
 */

#include <stdio.h>
#include <stdlib.h>
#include "gfal_api.h"
#include "gfal_posix_internal.h"

int main(int argc, char **argv)
{
	struct dirent *d;
	DIR *dir;

	if (argc != 2) {
		fprintf (stderr, "usage: %s filename\n", argv[0]);
        return 1;
	}

	gfal_set_verbose(10);

	if ((dir = gfal_opendir (argv[1])) == NULL) {
		perror ("gfal_opendir");
		fprintf(stderr, "%s\n", (*gfal_posix_get_last_error())->message);
        return 1;
	}

    
	while ((d = gfal_readdir (dir))) {
		printf ("%s\n", d->d_name);
	}

	if (gfal_closedir (dir) < 0) {
		perror ("gfal_closedir");
        return 1;
	}
    return 0;
}
