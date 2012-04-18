/**
 * Compile command : gcc gfal_testdir.c `pkg-config --libs --cflags gfal2`
 */

#include <stdio.h>
#include <stdlib.h>
#include "gfal_api.h"

main(int argc, char **argv)
{
	struct dirent *d;
	DIR *dir;

	if (argc != 2) {
		fprintf (stderr, "usage: %s filename\n", argv[0]);
		exit (1);
	}

	if ((dir = gfal_opendir (argv[1])) == NULL) {
		perror ("gfal_opendir");
		exit (1);
	}


	while ((d = gfal_readdir (dir))) {
		printf ("%s\n", d->d_name);
	}

	if (gfal_closedir (dir) < 0) {
		perror ("gfal_closedir");
		exit (1);
	}
	exit (0);
}
