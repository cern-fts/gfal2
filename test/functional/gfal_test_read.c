/**
 * Compile command : gcc gfal_testread.c `pkg-config --libs --cflags gfal2`
 */


#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <gfal_api.h>

#include <common/gfal_lib_test.h>

#define BLKLEN 65536

int main(int argc, char **argv)
{
	int fd;
	char ibuf[BLKLEN];
	int rc;

	if (argc != 2) {
		fprintf (stderr, "usage: %s filename\n", argv[0]);
		exit (1);
	}

	GError* error = NULL;
	gfal2_context_t handle = gfal2_context_new(&error);
	if (!handle)
	{
	    fprintf(stderr, "Could not create the handler: %s", error->message);
        return 1;
	}
	if (generate_file_if_not_exists(handle, argv[1], "file:///etc/hosts", &error))
	{
	    fprintf(stderr, "Could not generate the source: %s", error->message);
	    return 1;
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

	printf ("buffer: \n");
	fputs(ibuf, stdout);

	if ((gfal_close (fd)) < 0) {
		gfal_posix_check_error();
		exit (1);
	}
	printf ("close successful\n");

	g_assert(gfal_unlink(argv[1]) == 0);

	return 0;
}
