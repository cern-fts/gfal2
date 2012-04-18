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
 * @(#)$RCSfile: gfal_testunlink.c,v $ $Release$ $Date: 2008/05/08 13:16:36 $ CERN Jean-Philippe Baud
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "gfal_api.h"

main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf (stderr, "usage: %s filename\n", argv[0]);
		exit (1);
	}
	if (gfal_unlink (argv[1]) < 0) {
		perror ("gfal_unlink");
		exit (1);
	}
	exit (0);
}
