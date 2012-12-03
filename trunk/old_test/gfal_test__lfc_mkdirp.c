/*
 * Authors: Zsolt Molnar <zsolt.molnar@cern.ch, http://www.zsoltmolnar.hu>
 *
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
 *
 * Test if the logic recognizes unknown protocols of wrongly formatted
 * protocol lists in the transfetParameters property of PrepareToGet.
 */
#include "lfc_ifce.h"
#include "gfal_testsuite.h"

#include <assert.h>

#define __CALL_TEST(test) \
    printf("\nTest group: %s\n", #test); \
    res = (test); \
    if (!res) \
        return res;

/* Fixtures They depend on the test environemnt!!! TODO: make them configurable. */
static char* host = NULL;
#define errbufsz 1024
static char errbuf[errbufsz];

static char* _test__ckeck_recursive_directory_creation()
{
    return NULL;
}

char * gfal_test__lfc_mkdirp()
{
    char* res = NULL;
    __CALL_TEST(_test__ckeck_recursive_directory_creation());
    return res;
}

#undef __CALL_TEST

