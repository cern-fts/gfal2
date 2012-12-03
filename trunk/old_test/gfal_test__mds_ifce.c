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
#include "gfal_internals.h"
#include <assert.h>

#include "gfal_testsuite.h"

#define __CALL_TEST(test) \
    printf("\nTest group: %s\n", #test); \
    res = (test); \
    if (!res) \
        return res;

/* Fixtures They depend on the test environemnt!!! TODO: make them configurable. */
static char* host = NULL;
#define errbufsz 1024
static char errbuf[errbufsz];

static char* _test__get_se_types_and_endpoints_good_case()
{
    char **se_types = NULL;
    char **se_endpoints = NULL;
    int ret = -1;

    ret = get_se_types_and_endpoints (host, &se_types, &se_endpoints, errbuf, errbufsz);
    GFAL_TEST_EQUAL(0, ret);
    GFAL_TEST_EQUAL_STRING("srm_v1", *se_types);
    GFAL_TEST_EQUAL_STRING("srm_v2", *(se_types + 1));
    GFAL_TEST_EQUAL_STRING("httpg://lxbra1910.cern.ch:8443/srm/managerv1", *se_endpoints);
    GFAL_TEST_EQUAL_STRING("httpg://lxbra1910.cern.ch:8446/srm/managerv2", *(se_endpoints + 1));
}

char * gfal_test__mds_ifce()
{
    char* res = NULL;
    host=getenv("SE_ENDPOINT_DPM");
    //GFAL_TEST_ASSERT(host != NULL);

    // TODO: test temporarily disabled. Will ebe enabled when we integrate is_interface
    //__CALL_TEST(_test__get_se_types_and_endpoints_good_case());
    return res;
}

#undef __CALL_TEST

