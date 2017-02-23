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

#define NNESTED 10


class ReadDirTest: public testing::Test {
public:
    static const char* root;
    char surl[2048];
    char nested[NNESTED][2048];
    gfal2_context_t context;

    ReadDirTest() {
        GError *error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
    }

    virtual ~ReadDirTest() {
        gfal2_context_free(context);
    }

    virtual void SetUp() {
        GError* error = NULL;
        generate_random_uri(root, "readdir_test", surl, sizeof(surl));
        int ret = gfal2_mkdir(context, surl, 0777, &error);
        EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

        // Populate
        for (int i = 0; i < NNESTED; ++i) {
            snprintf(nested[i], sizeof(nested[i]), "%s/nested_elem_%d", surl, i);
            ret = gfal2_mkdir(context, nested[i], 0777, &error);
            EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
            g_clear_error(&error);
        }
    }

    virtual void TearDown() {
        GError* error = NULL;
        int i, ret;
        for (i = 0; i < NNESTED; ++i) {
            ret = gfal2_rmdir(context, nested[i], &error);
            EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
            g_clear_error(&error);
        }
        gfal2_rmdir(context, surl, &error);
        EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
        g_clear_error(&error);
    }
};
const char* ReadDirTest::root;


TEST_F(ReadDirTest, ReadDirENOENT)
{
    char enoent_dir[2048];
    generate_random_uri(surl, "readdir_enoent", enoent_dir, sizeof(enoent_dir));

    GError* error = NULL;
    DIR* dir = gfal2_opendir(context, enoent_dir, &error);
    EXPECT_PRED_FORMAT3(AssertGfalErrno, (dir == NULL)?-1:0, error, ENOENT);
}


TEST_F(ReadDirTest, ReadDir)
{
    GError* error = NULL;
    DIR* dir = gfal2_opendir(context, surl, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, (dir == NULL)?-1:0, error);

    int count = 0;
    struct dirent* d = NULL;
    while ((d = gfal2_readdir(context, dir, &error)) != NULL) {
        // Just in case the plugin returns . and .. as entries
        if (strcmp(".", d->d_name) != 0 && strcmp("..", d->d_name) != 0)
            ++count;
    }
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    EXPECT_EQ(NNESTED, count);

    int ret = gfal2_closedir(context, dir, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
}


TEST_F(ReadDirTest, ReadDirpp)
{
    GError* error = NULL;
    DIR* dir = gfal2_opendir(context, surl, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, dir != NULL, error);

    int count = 0;
    struct dirent* d = NULL;
    struct stat st;
    while ((d = gfal2_readdirpp(context, dir, &st, &error)) != NULL) {
        // Just in case the plugin returns . and .. as entries
        if (strcmp(".", d->d_name) != 0 && strcmp("..", d->d_name) != 0)
            ++count;
        EXPECT_TRUE(S_ISDIR(st.st_mode));
    }
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    EXPECT_EQ(NNESTED, count);

    int ret = gfal2_closedir(context, dir, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
}


int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 2) {
        printf("Missing base url\n");
        printf("\t%s [options] srm://host/base/path/\n", argv[0]);
        return 1;
    }

    ReadDirTest::root = argv[1];

//    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE | GFAL_VERBOSE_DEBUG);

    return RUN_ALL_TESTS();
}
