/**
 * Compile command : gcc gfal_teststat.c `pkg-config --libs --cflags gfal2`
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <gfal_api.h>

int main(int argc, char **argv)
{
	struct stat statbuf;

	gfal2_log_set_level(G_LOG_LEVEL_DEBUG);

	if (argc != 2) {
		fprintf (stderr, "usage: %s filename\n", argv[0]);
		return 1;
	}
	if (gfal_stat(argv[1], &statbuf) < 0) {
		char errbuf[512];
		gfal_posix_strerror_r(errbuf, sizeof(errbuf));
		printf("%s\n", errbuf);
		return 1;
	}
	printf ("stat successful\n");
	printf ("mode = %o\n", statbuf.st_mode);
	printf ("nlink = %ld\n", statbuf.st_nlink);
	printf ("uid = %d\n", statbuf.st_uid);
	printf ("gid = %d\n", statbuf.st_gid);
	printf ("size = %ld\n", statbuf.st_size);
	return 0;
}
