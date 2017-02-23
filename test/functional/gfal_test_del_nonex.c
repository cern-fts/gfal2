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

#include <stdio.h>
#include <gfal_api.h>

int main(int argc, char **argv)
{
    int err;

    gfal2_log_set_level(G_LOG_LEVEL_DEBUG);

    char buff[2048];

    if (argc != 2) {
        fprintf(stderr, "usage: %s valid_dir\n", argv[0]);
        exit(1);
    }

    // create a enoent file
    g_strlcpy(buff, argv[1], 2048);
    g_strlcat(buff, "/", 2048);
    g_strlcat(buff, "testfileunlink_enoent", 2048);

    printf("unlinking (deleting) the file: %s\n", buff);
    err = gfal_unlink(buff);
    err = gfal_posix_code_error();
    if (err != ENOENT) {
        return 1;
    }
    else {
        return 0;
    }
}
