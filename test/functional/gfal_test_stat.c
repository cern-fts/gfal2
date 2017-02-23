/*
 * Copyright (c) CERN 2013-2017
 *
 * Copyright (c) Members of the EMI Collaboration. 2010-2013
 *  See  http://www.eu-emi.eu/partners for details on the copyright
 *  holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
