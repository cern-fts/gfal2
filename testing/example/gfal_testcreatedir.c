/**
 * Compile command : gcc gfal_testcreatedir.c `pkg-config --libs --cflags gfal2`
 */

#include <stdio.h>
#include <stdlib.h>
#include "gfal_api.h"

main(int argc, char **argv)
{
	char *rootdir;
	char olddir[1024], newdir[1024];

	if (argc != 2) {
		fprintf (stderr, "usage: %s rootdir\n", argv[0]);
		exit (1);
	}

	rootdir = argv[1];
	snprintf (olddir, 1024, "%s/olddir", rootdir);
	snprintf (newdir, 1024, "%s/newdir", rootdir);

	printf ("Creating directory 'olddir'...\n");
	if (gfal_mkdir (olddir, 0700) < 0) {
		gfal_posix_check_error();
		exit (1);
	}

	printf ("Renaming directory 'olddir' to 'newdir'...\n");
	if (gfal_rename (olddir, newdir) < 0) {
		gfal_posix_check_error();
		exit (1);
	}

	printf ("Removing directory 'newdir'...\n");
	if (gfal_rmdir (newdir) < 0) {
		gfal_posix_check_error();
		exit (1);
	}

	printf ("All is ok.\n");
	exit (0);
}
