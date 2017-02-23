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

/*
 * NOTE: This test only makes sense for LFC as a destination!
 */
#include <gtest/gtest.h>
#include <fcntl.h>
#include <stdio.h>
#include <gfal_api.h>
#include <stdlib.h>
#include <common/gfal_lib_test.h>
#include <common/gfal_gtest_asserts.h>
#include <utils/exceptions/gerror_to_cpp.h>


class RegisterTest: public testing::Test {
public:
    static const char* lfc_root;
    static const char* origin_root;

    char origin[2048];
    char origin2[2048];
    char lfc[2048];
    gfal2_context_t context;
    gfalt_params_t params;

    RegisterTest() {
        GError *error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
        params = gfalt_params_handle_new(NULL);
    }

    virtual ~RegisterTest() {
        GError *error = NULL;
        gfalt_params_handle_delete(params, &error);
        gfal2_context_free(context);
    }

    virtual void SetUp() {
        GError* error = NULL;
        generate_random_uri(lfc_root, "register_test", lfc, sizeof(origin));
        generate_random_uri(origin_root, "register_test", origin, sizeof(origin));
        generate_random_uri(origin_root, "register_test", origin2, sizeof(origin2));
        int ret = generate_file_if_not_exists(context, origin, "file:///etc/hosts", &error);
        EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
        ret = generate_file_if_not_exists(context, origin2, "file:///etc/group", &error);
        EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    }

    virtual void TearDown() {
        GError* error = NULL;
        gfal2_unlink(context, origin, &error);
        g_clear_error(&error);
        gfal2_unlink(context, origin2, &error);
        g_clear_error(&error);
        gfal2_unlink(context, lfc, &error);
        g_clear_error(&error);
    }
};

const char* RegisterTest::lfc_root;
const char* RegisterTest::origin_root;


TEST_F(RegisterTest, RegisterNewEntry)
{
    GError* error = NULL;
    int ret = gfalt_copy_file(context, params, origin, lfc, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    char replicas[2048];
    ssize_t xattr_size = gfal2_getxattr(context, lfc, GFAL_XATTR_REPLICA, replicas, sizeof(replicas), &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    EXPECT_GT(xattr_size, 0);

    EXPECT_STRCASEEQ(origin, replicas);
}


TEST_F(RegisterTest, RegisterExistingEntry)
{
    GError* error = NULL;
    int ret = gfalt_copy_file(context, params, origin, lfc, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfalt_copy_file(context, params, origin, lfc, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
}


TEST_F(RegisterTest, RegisterMismatch)
{
    GError* error = NULL;
    int ret = gfalt_copy_file(context, params, origin, lfc, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfalt_copy_file(context, params, origin2, lfc, &error);
    EXPECT_PRED_FORMAT3(AssertGfalErrno, ret, error, EINVAL);
}


TEST_F(RegisterTest, RegisterAndOpen)
{
    GError* error = NULL;
    int ret = gfalt_copy_file(context, params, origin, lfc, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfal2_open(context, lfc, O_RDONLY, &error);
    EXPECT_GE(ret, 0);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, 0, error);

    char buffer[1024] = {0};
    ssize_t read = gfal2_read(context, ret, buffer, sizeof(buffer), &error);
    EXPECT_GT(read, 0);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, 0, error);


    char buffer2[1024] = {0};
    FILE* fd = fopen("/etc/hosts", "r");
    EXPECT_NE((void*)NULL, fd);
    size_t read2 = fread(buffer2, 1, sizeof(buffer2), fd);
    fclose(fd);

    EXPECT_EQ(read, read2);
    EXPECT_STREQ(buffer, buffer2);

    ret = gfal2_close(context, ret, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
}


int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 3) {
        printf("Missing origin or lfc url\n");
        printf("\t%s srm://host/base/path/ lfc://host/base/path\n", argv[0]);
        return 1;
    }

    RegisterTest::origin_root = argv[1];
    RegisterTest::lfc_root = argv[2];

    if (argc > 3 && strncmp(argv[3], "-v", 2) == 0) {
        gfal2_log_set_level(G_LOG_LEVEL_DEBUG);
    }

    return RUN_ALL_TESTS();
}
