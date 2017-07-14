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


#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "gfal_lib_test.h"
#include <uri/gfal2_uri.h>

char* generate_random_uri(const char* uri_dir, const char* prefix, char* buff,
        size_t s_buff)
{
    struct utsname info;
    memset(&info, 0, sizeof(struct utsname));
    uname(&info);

    if (uri_dir[strlen(uri_dir)-1] != '/') {
        snprintf(buff, s_buff, "%s/%s_%s_%d%ld%ld", uri_dir, info.nodename, prefix, (int) getpid(),
            (long) time(NULL), (long) rand());
    }
    else {
        snprintf(buff, s_buff, "%s%s_%s_%d%ld%ld", uri_dir, info.nodename, prefix, (int) getpid(),
            (long) time(NULL), (long) rand());
    }

    return buff;
}

char * generate_random_string_content(size_t size)
{
    char * res = (char*)malloc(size * sizeof(char));
    size_t i = 0;
    while (i < size) {
        res[i] = (char) (((rand() % 2) ? 65 : 97) + (rand() % 26));
        i++;
    }
    return res;
}

int generate_file_if_not_exists(gfal2_context_t handle, const char* surl,
        const char* src, GError** error)
{
    struct stat st;
    if (gfal2_stat(handle, surl, &st, error) == 0) {
        return 0;
    }

    if ((*error)->code != ENOENT) {
        return -1;
    }
    g_clear_error(error);

    return gfalt_copy_file(handle, NULL, src, surl, error);
}


/* Everything that links against this will, automatically,
 * setup the segfault handler
 */
#define MAX_STACK_DEPTH 25

static void getFileAndLine(void* addr, const char* sname,
        char* buffer, size_t bufsize)
{
    char fnameBuffer[512];
    // Extract file object from the symbol name
    strncpy(fnameBuffer, sname, sizeof(fnameBuffer) - 1);
    char *p = strchr(fnameBuffer, '(');
    if (p)
        *p = '\0';

    // Build the command
    char command[1024];
    snprintf(command, sizeof(command), "addr2line -e '%s' 0x%lx", fnameBuffer, (long)addr);

    // Run it
    buffer[0] = '\0';
    FILE *proc = popen(command, "r");
    if (proc) {
        size_t nbytes = fread(buffer, 1, bufsize, proc);
        pclose(proc);
        if (nbytes <= 0) {
            strncpy(buffer, "Could not read from addr2line\n", bufsize);
        }
    }
    else {
        strncpy(buffer, "Could not execute addr2line\n", bufsize);
    }

    if (buffer[0] == '?') {
        buffer[0] = '\n';
        buffer[1] = '\0';
    }
}

int clean_file(const char* surl)
{
    if (gfal_unlink(surl) < 0) {
        if (gfal_posix_code_error() != ENOENT) {
            char errbuf[512];
            gfal_posix_strerror_r(errbuf, sizeof(errbuf));
            printf("%s\n", errbuf);
            g_assert_not_reached();
        }
    }
    return 0;
}


int expect_third_party_copy(const char *url1, const char *url2)
{
    int expect = 0;
    gfal2_uri *uri1 = gfal2_parse_uri(url1, NULL);
    gfal2_uri *uri2 = gfal2_parse_uri(url2, NULL);

    // If file is involved, surely not
    if (g_strcmp0(uri1->scheme, "file") == 0 || g_strcmp0(uri2->scheme, "file") == 0) {
        expect = 0;
        goto done_expect;
    }

    // If SRM involved, let's assume yes
    if (g_strcmp0(uri1->scheme, "srm") == 0 || g_strcmp0(uri2->scheme, "srm") == 0) {
        expect = 1;
        goto done_expect;
    }

    // If both are the same, depending on the protocol
    int is_same = (g_strcmp0(uri1->scheme, uri2->scheme) == 0);
    if (is_same) {
        expect = g_strcmp0(uri1->scheme, "gsiftp") == 0 ||
            g_strcmp0(uri1->scheme, "root") == 0 ||
            g_strcmp0(uri1->scheme, "davs") == 0;
        goto done_expect;
    }

    // Otherwise, not expected
    // Mind that there are other combinations here that could happen, and even
    // some protocols can do copies both with third party or not, depending on the storages involved
    // (i.e. davs, s3, root), so this is an incomplete heuristic only suitable for the storages normally
    // involved on the tests

done_expect:
    gfal2_free_uri(uri1);
    gfal2_free_uri(uri2);
    return expect;
}


static void dump_stack(int sig)
{
    if (sig == SIGSEGV || sig == SIGBUS || sig == SIGABRT) {
        fprintf(stderr, "FATAL ERROR!\n");
        void *array[MAX_STACK_DEPTH] = { 0 };
        char fileBuffer[1024];
        int nFrames = backtrace(array, MAX_STACK_DEPTH);
        char **symbols = backtrace_symbols(array, nFrames);
        int i;
        for (i = 0; symbols && i < nFrames; ++i) {
            if (symbols[i]) {
                getFileAndLine(array[i], symbols[i], fileBuffer, sizeof(fileBuffer));
                fprintf(stderr, "%s\n", symbols[i]);
                fprintf(stderr, "\t%s", fileBuffer);
            }
        }
        if (symbols) {
            free(symbols);
        }

        exit(1);
    }
}


__attribute__((constructor))
void setup_segfault_handler(void)
{
    signal(SIGSEGV, dump_stack);
}
