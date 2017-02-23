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

/**
 * Regression test for DMC-529
 * Set the credentials via configuration, and make sure that it does work!
 */
#include <gfal_api.h>


int main(int argc, char** argv)
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s proxy surl\n", argv[0]);
        return 1;
    }

    const char* uproxy = argv[1];
    const char* surl = argv[2];

    if (getenv("X509_USER_CERT") ||
        getenv("X509_USER_KEY") ||
        getenv("X509_USER_PROXY")) {
        fprintf(stderr, "Unset X509_USER_* environment variables before calling");
        return 1;
    }

    GError* error = NULL;
    gfal2_context_t context = gfal2_context_new(&error);
    if (error) {
        fprintf(stderr, "%s\n", error->message);
        g_clear_error(&error);
        return 1;
    }

    gfal2_set_opt_string(context, "X509", "CERT", uproxy, NULL);
    gfal2_set_opt_string(context, "X509", "KEY", uproxy, NULL);

    struct stat st;
    int ret = gfal2_stat(context, surl, &st, &error);
    if (ret < 0) {
        fprintf(stderr, "Failed to stat: %s\n", error->message);
        g_clear_error(&error);
        return 1;
    }

    printf("Stat OK:\n\tMode:\t%04o\n\tSize:\t%zd\n", st.st_mode, st.st_size);

    gfal2_context_free(context);
    return 0;
}
