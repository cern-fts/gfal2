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

#include <execinfo.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <transfer/gfal_transfer.h>
#include <unistd.h>

#include "gfal_lib_test.h"

char* generate_random_uri(const char* uri_dir, const char* prefix, char* buff,
        size_t s_buff)
{
    snprintf(buff, s_buff, "%s/%s_%d%ld%ld", uri_dir, prefix, (int) getpid(),
            (long) time(NULL ), (long) rand());
    return buff;
}

char * generate_random_string_content(size_t size)
{
    char * res = malloc(size * sizeof(char));
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
    if (gfal_stat(surl, &st) == 0)
        return 0;

    return gfalt_copy_file(handle, NULL, src, surl, error);
}


/* Everything that links agains this will, automatically,
 * setup the segfault handler
 */
#define MAX_STACK_DEPTH 25

static void getFileAndLine(void* addr, const char* sname,
        char* buffer, size_t bufsize)
{
    char fnameBuffer[512];
    // Extract file object from the symbol name
    strncpy(fnameBuffer, sname, sizeof(fnameBuffer));
    char *p = strchr(fnameBuffer, '(');
    if (p)
        *p = '\0';

    // Build the command
    char command[1024];
    snprintf(command, sizeof(command), "addr2line -e '%s' 0x%lx", fnameBuffer, (long)addr);

    // Run it
    buffer[0] = '\0';
    FILE *proc = popen(command, "r");
    fread(buffer, 1, bufsize, proc);
    pclose(proc);

    if (buffer[0] == '?') {
        buffer[0] = '\n';
        buffer[1] = '\0';
    }
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
