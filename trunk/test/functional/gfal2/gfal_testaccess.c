/**
 * Compile command : gcc gfal_teststat.c `pkg-config --libs --cflags gfal2`
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <gfal_api.h>

int main(int argc, char **argv)
{
	int flags = R_OK;

	if (argc != 3) {
		fprintf (stderr, "usage: %s filename flags\n", argv[0]);
		exit (1);
	}
	if (gfal_access (argv[1], flags) < 0) {
		gfal_posix_check_error();
		exit (1);
	}
	printf ("access successfull\n");
	return 0;
}
