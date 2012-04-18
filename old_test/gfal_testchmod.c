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
 * @(#)$RCSfile: gfal_testchmod.c,v $ $Revision: 1.3 $ $Date: 2008/05/08 13:16:36 $ CERN Remi Mollon
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "gfal_api.h"

main(int argc, char **argv)
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
		perror ("gfal_access");
	}

	printf ("Changing mode of '%s' to %o...\n", file, mode);
	if (gfal_chmod (file, mode) < 0) {
		error = 1;
		perror ("gfal_chmod");
	}

	if (error) exit (1);

	printf ("All is ok.\n");
	exit (0);
}
