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
 * @(#)$RCSfile: gfal_testcreatdir.c,v $ $Revision: 1.2 $ $Date: 2008/05/08 13:16:36 $ CERN Remi Mollon
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
		perror ("gfal_mkdir");
		exit (1);
	}

	printf ("Renaming directory 'olddir' to 'newdir'...\n");
	if (gfal_rename (olddir, newdir) < 0) {
		perror ("gfal_rename");
		exit (1);
	}

	printf ("Removing directory 'newdir'...\n");
	if (gfal_rmdir (newdir) < 0) {
		perror ("gfal_rmdir");
		exit (1);
	}

	printf ("All is ok.\n");
	exit (0);
}
