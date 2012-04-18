/**
 * Compile command : gcc gfal_testread.c `pkg-config --libs --cflags gfal2`
 */


#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <gfal_api.h>
#define BLKLEN 65536

main(int argc, char **argv)
{
	int fd;
	char ibuf[BLKLEN];
	int rc;

	if (argc != 2) {
		fprintf (stderr, "usage: %s filename\n", argv[0]);
		exit (1);
	}

	printf ("opening %s\n", argv[1]);
	if ((fd = gfal_open (argv[1], O_RDONLY, 0)) < 0) {
		gfal_posix_check_error();
		exit (1);
	}
	printf ("open successful, fd = %d (errno = %d)\n", fd, errno);

	if ((rc = gfal_read (fd, ibuf, BLKLEN)) < 0) {
		gfal_posix_check_error();
		(void) gfal_close (fd);
		exit (1);
	}
	printf ("read successful (errno = %d)\n", errno);

	if ((gfal_close (fd)) < 0) {
		gfal_posix_check_error();
		exit (1);
	}
	printf ("close successful\n");
	exit (0);
}
