/**
 * Compile command : gcc gfal_test_del_nonex.c `pkg-config --libs --cflags gfal2`
 */


#include <fcntl.h>
#include <stdio.h>
#include <gfal_api.h>
#include <stdlib.h>
#define BLKLEN 65536

int main(int argc, char **argv)
{
	int err;

//	gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE);	

	if (argc != 2) {
		fprintf (stderr, "usage: %s filename\n", argv[0]);
		exit (1);
	}

	printf ("unlinking (deleting) the file: %s\n", argv[1]);
	err = gfal_unlink (argv[1]);
	gfal_posix_check_error();
	err = gfal_posix_code_error();
	if (err != ENOENT) { 
		exit (1);
	} else {
		exit(0);
	}
}
