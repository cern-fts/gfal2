/**
 * Compile command : gcc gfal_testcreatedir.c `pkg-config --libs --cflags gfal2`
 */

#include <stdio.h>
#include <stdlib.h>
#include "gfal_api.h"

int main(int argc, char **argv)
{
	char *rootdir;

	if (argc != 2) {
		fprintf (stderr, "usage: %s rootdir rights \n", argv[0]);
		exit (1);
	}

	rootdir = argv[1];

	printf ("Creating directory %s ...\n", rootdir);
	if (gfal_mkdir (rootdir, 0777) < 0) {
		gfal_posix_check_error();
		exit (1);
	}



	printf ("All is ok.\n");
	exit (0);
}
