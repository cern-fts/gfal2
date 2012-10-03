/**
 * Compile command : gcc gfal_test_del_nonex.c `pkg-config --libs --cflags gfal2`
 */


#include <fcntl.h>
#include <stdio.h>
#include <gfal_api.h>
#include <stdlib.h>
#include "gfal_lib_test.h"

#define BLKLEN 65536

int main(int argc, char **argv)
{
	int err;

    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE);

    char buff[2048];

    if (argc != 2) {
        fprintf (stderr, "usage: %s valid_dir\n", argv[0]);
        exit (1);
    }

    // create a enoent file
    g_strlcpy(buff, argv[1], 2048);
    g_strlcat(buff, "/", 2048);
    g_strlcat(buff, "testfileunlink_enoent", 2048);




    printf ("unlinking (deleting) the file: %s\n", buff);
    err = gfal_unlink (buff);
	gfal_posix_check_error();
	err = gfal_posix_code_error();
	if (err != ENOENT) { 
		exit (1);
	} else {
		exit(0);
	}
}
