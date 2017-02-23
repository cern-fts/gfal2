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


class RmDirTest: public testing::Test {
public:
    static const char* root;

    char surl[2048];
    char surl_nested[2048];
    gfal2_context_t context;

    RmDirTest() {
        GError *error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
    }

    virtual ~RmDirTest() {
        gfal2_context_free(context);
    }

    virtual void SetUp() {
        generate_random_uri(root, "test_rmdir", surl, sizeof(surl));
        generate_random_uri(surl, "test_rmdir_nested", surl_nested, sizeof(surl_nested));
    }

    virtual void TearDown() {
        GError* error = NULL;
        gfal2_unlink(context, surl, &error);
        g_clear_error(&error);
        gfal2_chmod(context, surl, 0777, &error);
        g_clear_error(&error);
        gfal2_rmdir(context, surl, &error);
        g_clear_error(&error);
        gfal2_rmdir(context, surl_nested, &error);
        g_clear_error(&error);
    }
};
const char* RmDirTest::root = NULL;


static int is_dav(const char* surl)
{
    return strncmp(surl, "dav:", 4) == 0 ||
           strncmp(surl, "davs:", 5) == 0 ||
           strncmp(surl, "http:", 5) == 0 ||
           strncmp(surl, "https:", 6) == 0 ||
           strncmp(surl, "dav+3rd:", 8) == 0 ||
           strncmp(surl, "davs+3rd:", 9) == 0 ||
           strncmp(surl, "http+3rd:", 9) == 0 ||
           strncmp(surl, "https+3rd:", 9) == 0;
}


TEST_F(RmDirTest, RmDirENOENT)
{
    GError* error = NULL;
    int ret = gfal2_rmdir(context, surl, &error);
    EXPECT_PRED_FORMAT3(AssertGfalErrno, ret, error, ENOENT);
}


TEST_F(RmDirTest, RmDirExists)
{
    struct stat st;
    GError* error = NULL;

    int ret = gfal2_mkdir(context, surl, 0777, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    // Trigger a stat, this will activate the cache (i.e. srm)
    // Regression for DMC-584
    ret = gfal2_stat(context, surl, &st, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);


    ret = gfal2_rmdir(context, surl, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    // Make sure it is not there!
    ret = gfal2_stat(context, surl, &st, &error);
    EXPECT_PRED_FORMAT3(AssertGfalErrno, ret, error, ENOENT);
}


TEST_F(RmDirTest, RmDirExists2)
{
    struct stat st;
    GError* error = NULL;

    g_strlcat(surl, "/", sizeof(surl));

    // Same thing but with trailing slash

    int ret = gfal2_mkdir(context, surl, 0777, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    // Trigger a stat, this will activate the cache (i.e. srm)
    // Regression for DMC-584
    ret = gfal2_stat(context, surl, &st, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);


    ret = gfal2_rmdir(context, surl, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    // Make sure it is not there!
    ret = gfal2_stat(context, surl, &st, &error);
    EXPECT_PRED_FORMAT3(AssertGfalErrno, ret, error, ENOENT);
}


TEST_F(RmDirTest, RmDirNestedNotEmpty)
{
    // dav removes recursively, so skip
    if (is_dav(surl)) {
        SKIP_TEST(RmDirNestedNotEmpty);
        return;
    }

    GError* error = NULL;

    int ret = gfal2_mkdir(context, surl, 0777, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfal2_mkdir(context, surl_nested, 0777, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfal2_rmdir(context, surl, &error);
    EXPECT_PRED_FORMAT3(AssertGfalErrno, ret, error, ENOTEMPTY);
}


TEST_F(RmDirTest, RmDirNestedEACCESS)
{
    if (is_dav(surl) || strncmp("file:", surl, 5) == 0 || strncmp("root:", surl, 5) == 0) {
        SKIP_TEST(RmDirNestedEACCESS);
        return;
    }

    GError* error = NULL;

    int ret = gfal2_mkdir(context, surl, 0777, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfal2_mkdir(context, surl_nested, 0777, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfal2_chmod(context, surl, 0000, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfal2_rmdir(context, surl_nested, &error);
    EXPECT_PRED_FORMAT3(AssertGfalErrno, ret, error, EACCES);
}


TEST_F(RmDirTest, RmDirNestedENOTDIR)
{
    // Skip for LFC
    if (strncmp(surl, "lfc:/", 5) == 0 || strncmp(surl, "lfn:/", 5) == 0) {
        SKIP_TEST(RmDirNestedENOTDIR);
        return;
    }

    GError *error = NULL;

    int ret = generate_file_if_not_exists(context, surl, "file:///etc/hosts", &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfal2_rmdir(context, surl, &error);
    EXPECT_PRED_FORMAT3(AssertGfalErrno, ret, error, ENOTDIR);
}


int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 2) {
        printf("Missing base url\n");
        printf("\t%s [options] srm://host/base/path/\n", argv[0]);
        return 1;
    }

    RmDirTest::root = argv[1];

//    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE | GFAL_VERBOSE_DEBUG);

    return RUN_ALL_TESTS();
}
