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

#include <gtest/gtest.h>
#include <fcntl.h>
#include <stdio.h>
#include <gfal_api.h>
#include <stdlib.h>
#include <common/gfal_lib_test.h>
#include <common/gfal_gtest_asserts.h>
#include <utils/exceptions/gerror_to_cpp.h>


class AccessTest: public testing::Test {
public:
    static const char* root;

    char surl[2048];
    gfal2_context_t context;

    AccessTest() {
        GError *error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
    }

    virtual ~AccessTest() {
        gfal2_context_free(context);
    }

    virtual void SetUp() {
        GError* error = NULL;
        generate_random_uri(root, "access_test", surl, sizeof(surl));
        int ret = gfal2_mkdir(context, surl, 0775, &error);
        EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    }

    virtual void TearDown() {
        GError* error = NULL;
        gfal2_chmod(context, surl, 0775, &error);
        gfal2_rmdir(context, surl, &error);
        g_clear_error(&error);
    }
};
const char* AccessTest::root;


TEST_F(AccessTest, SimpleAccess)
{
    GError *error = NULL;
    int ret = gfal2_access(context, surl, R_OK, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    // If running as root, and the surl is file://, it will always be possible to do anything
    if (strncmp(surl, "file://", 7) != 0 || geteuid() != 0) {
        ret = gfal2_chmod(context, surl, 0440, &error);
        EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

        ret = gfal2_access(context, surl, W_OK, &error);
        EXPECT_PRED_FORMAT3(AssertGfalErrno, ret, error, EACCES);
    }
}


int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 2) {
        printf("Missing base url and/or modes\n");
        printf("\t%s [options] srm://host/base/path/\n", argv[0]);
        return 1;
    }

    AccessTest::root = argv[1];

    return RUN_ALL_TESTS();
}
