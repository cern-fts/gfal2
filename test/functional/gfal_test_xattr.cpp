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
#include <gfal_api.h>
#include <common/gfal_lib_test.h>
#include <common/gfal_gtest_asserts.h>
#include <utils/exceptions/gerror_to_cpp.h>
#include <list>


class XAttrTest: public testing::Test {
public:
    static const char* root;
    static const char* origin_root;

    char surl[2048];
    char origin[2048];
    gfal2_context_t context;

    XAttrTest() {
        GError *error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
        origin[0] = '\0';
    }

    ~XAttrTest() {
        gfal2_context_free(context);

    }

    void SetUp() {
        GError *error = NULL;
        generate_random_uri(root, "xattr_test", surl, sizeof(surl));
        int ret;
        if (strncmp(root, "lfc://", 6) != 0) {
            ret = generate_file_if_not_exists(context, surl, "file:///etc/hosts", &error);
            EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
        }
        else {
            generate_random_uri(origin_root, "xattr_test", origin, sizeof(surl));
            ret = generate_file_if_not_exists(context, origin, "file:///etc/hosts", &error);
            EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
            ret = gfalt_copy_file(context, NULL, origin, surl, &error);
        }
    }

    void TearDown() {
        GError *error = NULL;
        gfal2_unlink(context, surl, &error);
        g_clear_error(&error);
        gfal2_unlink(context, origin, &error);
        g_clear_error(&error);
    }
};

const char *XAttrTest::root;
const char *XAttrTest::origin_root;


TEST_F(XAttrTest, SrmType)
{
    if (strncmp(root, "srm://", 6) != 0) {
        SKIP_TEST(SrmType);
        return;
    }

    GError *error = NULL;
    char buffer[1024];

    ssize_t ret = gfal2_getxattr(context, surl, "srm.type", buffer, sizeof(buffer), &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    EXPECT_GT(ret, 0);
    EXPECT_GT(strlen(buffer), 0);

    ret = gfal2_listxattr(context, surl, buffer, sizeof(buffer), &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    EXPECT_GT(ret, 0);
    
    bool found = false;
    int i = 0;
    while (i < ret) {
        if (strncmp(buffer + i, "srm.type", 8) == 0) {
            found = true;
            break;
        }
        i += strlen(buffer + i) + 1;
    }
    EXPECT_TRUE(found);
}


TEST_F(XAttrTest, Status)
{

    GError *error = NULL;
    char buffer[1024];

    ssize_t ret = gfal2_getxattr(context, surl, GFAL_XATTR_STATUS, buffer, sizeof(buffer), &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    EXPECT_GT(ret, 0);
    EXPECT_GT(strlen(buffer), 0);

    bool found = false;
    int i = 0;
    while (i < ret) {
        if (strncmp(buffer + i, GFAL_XATTR_STATUS_ONLINE, sizeof(GFAL_XATTR_STATUS_ONLINE)) == 0) {
            found = true;
            break;
        }
        i += strlen(buffer + i) + 1;
    }
    EXPECT_TRUE(found);
}


TEST_F(XAttrTest, SrmReplicas)
{
    if (strncmp(root, "srm://", 6) != 0) {
        SKIP_TEST(SrmReplicas);
        return;
    }

    GError *error = NULL;
    char buffer[1024];

    ssize_t ret = gfal2_getxattr(context, surl, GFAL_XATTR_REPLICA, buffer, sizeof(buffer), &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    EXPECT_GT(ret, 0);
    EXPECT_GT(strlen(buffer), 0);

    ret = gfal2_listxattr(context, surl, buffer, sizeof(buffer), &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    EXPECT_GT(ret, 0);

    bool found = false;
    int i = 0;
    while (i < ret) {
        if (strncmp(buffer + i, GFAL_XATTR_REPLICA, sizeof(GFAL_XATTR_REPLICA)) == 0) {
            found = true;
            break;
        }
        i += strlen(buffer + i) + 1;
    }
    EXPECT_TRUE(found);
}


TEST_F(XAttrTest, LfcComment)
{
    if (strncmp(root, "lfc://", 6) != 0) {
        SKIP_TEST(LfcComment);
        return;
    }

    GError *error = NULL;
    char comment[] = "this is a comment";
    char buffer[1024];

    int ret = gfal2_setxattr(context, surl, GFAL_XATTR_COMMENT, comment, sizeof(comment), 0, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfal2_listxattr(context, surl, buffer, sizeof(buffer), &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    EXPECT_GT(ret, 0);

    bool found = false;
    int i = 0;
    while (i < ret) {
        if (strncmp(buffer + i, GFAL_XATTR_COMMENT, sizeof(GFAL_XATTR_COMMENT)) == 0) {
            found = true;
            break;
        }
        i += strlen(buffer + i) + 1;
    }
    EXPECT_TRUE(found);

    ret = gfal2_getxattr(context, surl, GFAL_XATTR_COMMENT, buffer, sizeof(buffer), &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    EXPECT_GT(ret, 0);
    EXPECT_STREQ(comment, buffer);
}


TEST_F(XAttrTest, LfcReplica)
{
    if (strncmp(root, "lfc://", 6) != 0) {
        SKIP_TEST(LfcReplica);
        return;
    }

    GError *error = NULL;
    char buffer[1024];

    int ret = gfal2_listxattr(context, surl, buffer, sizeof(buffer), &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    EXPECT_GT(ret, 0);

    bool found = false;
    int i = 0;
    while (i < ret) {
        if (strncmp(buffer + i, GFAL_XATTR_REPLICA, sizeof(GFAL_XATTR_REPLICA)) == 0) {
            found = true;
            break;
        }
        i += strlen(buffer + i) + 1;
    }
    EXPECT_TRUE(found);

    ret = gfal2_getxattr(context, surl, GFAL_XATTR_REPLICA, buffer, sizeof(buffer), &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    EXPECT_GT(ret, 0);
    EXPECT_STREQ(origin, buffer);
}


TEST_F(XAttrTest, LfcSetReplica)
{
    if (strncmp(root, "lfc://", 6) != 0) {
        SKIP_TEST(LfcSetReplica);
        return;
    }

    struct stat buf;
    stat("/etc/hosts", &buf);

    GError *error = NULL;
    char buffer[1024];
    char replica[1024];
    snprintf(replica, sizeof(replica), "mock://fake/file?size=%lld", (long long)buf.st_size);

    // Add
    snprintf(buffer, sizeof(buffer), "+%s", replica);
    int ret = gfal2_setxattr(context, surl, GFAL_XATTR_REPLICA, buffer, sizeof(buffer), 0, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, 0, error);

    ret = gfal2_getxattr(context, surl, GFAL_XATTR_REPLICA, buffer, sizeof(buffer), &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    EXPECT_EQ(ret, strlen(origin) + strlen(replica) + 2);

    // Remove
    snprintf(buffer, sizeof(buffer), "-%s", replica);
    ret = gfal2_setxattr(context, surl, GFAL_XATTR_REPLICA, buffer, sizeof(buffer), 0, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, 0, error);

    ret = gfal2_getxattr(context, surl, GFAL_XATTR_REPLICA, buffer, sizeof(buffer), &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    EXPECT_EQ(ret, strlen(origin) + 1);
}


int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 2) {
        printf("Missing base url\n");
        printf("\t%s [options] srm://host/base/path/\n", argv[0]);
        return 1;
    }

    XAttrTest::root = argv[1];
    if (strncmp(XAttrTest::root, "lfc://", 6) == 0) {
        if (argc < 3) {
            printf("Need a site URL for creating the replica!\n");
            return 1;
        }
        XAttrTest::origin_root = argv[2];
    }

    return RUN_ALL_TESTS();
}
