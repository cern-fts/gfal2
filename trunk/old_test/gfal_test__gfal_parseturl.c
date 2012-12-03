/*
 * Authors: Zsolt Molnar <zsolt.molnar@cern.ch, http://www.zsoltmolnar.hu>
 *
 * Unit tests for parseturl method in gfal.c
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
 */
#include "gfal_internals.h"
#include "gfal_testsuite.h"
#include <errno.h>

/* WARNING: the test functions may have memory leaks. Due to the nature of the
 * test application, they are not handled elaborately, do not worry if you find
 * some... They do not affect the test execution and production.*/
#define __CALL_TEST(test) \
    printf("\nTest group: %s\n", #test); \
    res = (test); \
    if (!res) \
        return res;

/**
 * Test for bug 55424: gfal file open problem(gfal_open)
 *
 * https://savannah.cern.ch/bugs/?55424
 *
 * The parseturl function crashed when the TURL was NULL. Here, we pass NULL
 * as turl, the test should not crash and must return error code / error
 * message.
 */
static char * _test_parseturl_turl_is_null_55424()
{
    static const int errbufsz = 1024;
    char errbuf[errbufsz];
    int res = 1232532;
    errbuf[0] = 0;
    errno = 0;
    /* White box test. We know that the turl is checked first... */
    res = parseturl(NULL /* turl */, NULL, 0, NULL, 0, errbuf, errbufsz);
    GFAL_TEST_EQUAL(-1, res);
    GFAL_TEST_EQUAL(EFAULT, errno);
    return NULL;
}

char * gfal_test__gfal_parseturl()
{
    char* res = NULL;
    __CALL_TEST(_test_parseturl_turl_is_null_55424());
    return NULL;
}
