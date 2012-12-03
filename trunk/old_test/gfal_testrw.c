/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * @(#)$RCSfile: gfal_testrw.c,v $ $Revision: 1.2 $ $Date: 2008/05/08 13:16:36 $ CERN Jean-Philippe Baud
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "gfal_api.h"
#define BLKLEN 65536

main(int argc, char **argv)
{
	int fd;
	int i;
	char ibuf[BLKLEN];
	char obuf[BLKLEN];
	int rc;

	if (argc != 2) {
		fprintf (stderr, "usage: %s filename\n", argv[0]);
		exit (1);
	}

	for (i = 0; i < BLKLEN; i++)
		obuf[i] = i;

	printf ("creating file %s\n", argv[1]);
	if ((fd = gfal_open (argv[1], O_WRONLY|O_CREAT, 0644)) < 0) {
		perror ("gfal_open");
		exit (1);
	}
	printf ("open successful, fd = %d\n", fd);

	if ((rc = gfal_write (fd, obuf, BLKLEN)) != BLKLEN) {
		if (rc < 0)
			perror ("gfal_write");
		else
			fprintf (stderr, "gfal_write returns %d\n", rc);
		(void) gfal_close (fd);
		exit (1);
	}
	printf ("write successful\n");

	if ((rc = gfal_close (fd)) < 0) {
		perror ("gfal_close");
		exit (1);
	}
	printf ("close successful\n");

	printf ("reading back %s\n", argv[1]);
	if ((fd = gfal_open (argv[1], O_RDONLY, 0)) < 0) {
		perror ("gfal_open");
		exit (1);
	}
	printf ("open successful, fd = %d\n", fd);

	if ((rc = gfal_read (fd, ibuf, BLKLEN)) != BLKLEN) {
		if (rc < 0)
			perror ("gfal_read");
		else
			fprintf (stderr, "gfal_read returns %d\n", rc);
		(void) gfal_close (fd);
		exit (1);
	}
	printf ("read successful\n");

	if ((rc = gfal_close (fd)) < 0) {
		perror ("gfal_close");
		exit (1);
	}
	printf ("close successful\n");

	for (i = 0; i < BLKLEN; i++) {
		if (ibuf[i] != obuf[i]) {
			fprintf (stderr, "compare failed at offset %d\n", i);
			exit (1);
		}
	}
	printf ("compare successful\n");
	exit (0);
}
