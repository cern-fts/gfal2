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
 * @(#)$RCSfile: gfal_testget.c,v $ $Revision: 1.7 $ $Date: 2009/04/08 14:21:34 $ CERN Jean-Philippe Baud
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include "gfal_api.h"
#define DEFPOLLINT 10

main(argc, argv)
int argc;
char **argv;
{
	gfal_request req = NULL;
	gfal_internal gobj = NULL;
	gfal_filestatus *filestatuses = NULL;
	int sav_errno = 0, n = 0, i = 0, nberrors = 0;
	static char *protos[] = {"rfio", "dcap", "gsiftp"};

	if (argc < 2) {
		fprintf (stderr, "usage: %s SURLs\n", argv[0]);
		exit (1);
	}

	gfal_set_verbose (0);

	if ((req = gfal_request_new ()) == NULL)
		exit (1);
	req->nbfiles = argc - 1;
	req->surls = argv + 1;
	req->protocols = protos;

	if (gfal_init (req, &gobj, NULL, 0) < 0) {
		sav_errno = errno;
		free (req);
		errno = sav_errno;
		perror (argv[0]);
		exit (1);
	}
	free (req);

	if (gfal_turlsfromsurls (gobj, NULL, 0) < 0) {
		sav_errno = errno;
		gfal_internal_free (gobj);
		errno = sav_errno;
		perror (argv[0]);
		exit (1);
	}

	if ((n = gfal_get_results (gobj, &filestatuses)) < 0) {
		sav_errno = errno;
		gfal_internal_free (gobj);
		errno = sav_errno;
		perror (argv[0]);
		exit (1);
	}

	if (filestatuses == NULL) {
		fprintf (stderr, "%s: Internal error (memory corruption?)\n", argv[0]);
		exit (1);
	}

	for (i = 0; i < n; ++i) {
		if (filestatuses[i].status == 0)
			printf("SURL %s Ready - TURL: %s\n", filestatuses[i].surl, filestatuses[i].turl);
		else {
			++nberrors;
			if (filestatuses[i].explanation)
				printf("SURL %s Failed:\n%s\n", filestatuses[i].surl, filestatuses[i].explanation);
			else
				printf("SURL %s Failed:\n%s\n", filestatuses[i].surl, strerror (filestatuses[i].status));
		}
	}

	gfal_internal_free (gobj);
	exit (nberrors > 0 ? 1 : 0);
}
