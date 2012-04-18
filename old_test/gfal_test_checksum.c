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
 * @(#)$RCSfile: gfal_test_checksum.c,v $ $Revision: 1.1 $ $Date: 2009/08/26 15:40:06 $ CERN Jean-Philippe Baud
 */
#include <stdio.h>
#include <stdlib.h>
#include "gfal_api.h"

static void _gfal_test(const char* file, const int line, const int res)
{
    if (!res) {
        printf("FAILED in file '%s', line %d.\n", file, line);
        exit(-1);
    }
}

#define GFAL_TEST(res) \
    _gfal_test(__FILE__, __LINE__, (res))

static void gfal_test__parse_checksum_algname()
{
    enum gfal_cksm_type gct;
    /* Test the good cases */
    gct = gfal_parse_checksum_algname("CRC32");
    GFAL_TEST(gct == GFAL_CKSM_CRC32);
    gct = gfal_parse_checksum_algname("ADLER32");
    GFAL_TEST(gct == GFAL_CKSM_ADLER32);
    gct = gfal_parse_checksum_algname("MD5");
    GFAL_TEST(gct == GFAL_CKSM_MD5);
    gct = gfal_parse_checksum_algname("SHA1");
    GFAL_TEST(gct == GFAL_CKSM_SHA1);
    /* Test garbage */
    gct = gfal_parse_checksum_algname("garbage");
    GFAL_TEST(gct == GFAL_CKSM_NONE);
    /* Test null parameter and empty string */
    gct = gfal_parse_checksum_algname(NULL);
    GFAL_TEST(gct == GFAL_CKSM_NONE);
    gct = gfal_parse_checksum_algname("");
    GFAL_TEST(gct == GFAL_CKSM_NONE);
    /* Test abbreviations (DPM algorithm naming cases) */
    gct = gfal_parse_checksum_algname("CR");
    GFAL_TEST(gct == GFAL_CKSM_CRC32);
    gct = gfal_parse_checksum_algname("AD");
    GFAL_TEST(gct == GFAL_CKSM_ADLER32);
    gct = gfal_parse_checksum_algname("MD");
    GFAL_TEST(gct == GFAL_CKSM_MD5);
    /* Test a lowercase standard algorithm name */
    gct = gfal_parse_checksum_algname("adler32");
    GFAL_TEST(gct == GFAL_CKSM_ADLER32);
}

int main(int argc, char** argv)
{
    gfal_test__parse_checksum_algname();
    printf("PASSED.\n");
    return 0;
}
