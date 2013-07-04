/**
 * Compile command : gcc gfal_testchmod.c `pkg-config --libs --cflags gfal2`
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "gfal_api.h"

int main(int argc, char **argv)
{
	int mode, error = 0;
	char *file;

	if (argc != 3) {
		fprintf (stderr, "usage: %s file mode\n", argv[0]);
		exit (1);
	}

	file = argv[1];
	mode = strtol (argv[2], NULL, 8);
	if (errno > 0) {
		perror ("strtol");
		exit (1);
	}

	printf ("Checking RW access to '%s'...\n",file);
	if (gfal_access (file, R_OK|W_OK) < 0) {
		error = 1;
		gfal_posix_check_error();
	}

	printf ("Changing mode of '%s' to %o...\n", file, mode);
	if (gfal_chmod (file, mode) < 0) {
		error = 1;
		gfal_posix_check_error();
	}

	if (error) exit (1);

	printf ("All is ok.\n");
	exit (0);
}
