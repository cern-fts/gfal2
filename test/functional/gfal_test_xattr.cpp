#include <gtest/gtest.h>
#include <gfal_api.h>
#include <common/gfal_lib_test.h>
#include <common/gfal_gtest_asserts.h>
#include <utils/exceptions/gerror_to_cpp.h>
#include <list>


class XAttrTest: public testing::Test {
public:
    static const char* root;

    char surl[2048];
    gfal2_context_t context;

    XAttrTest() {
        GError *error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
    }

    ~XAttrTest() {
        gfal2_context_free(context);

    }

    void SetUp() {
        GError *error = NULL;
        generate_random_uri(root, "xattr_test", surl, sizeof(surl));
        int ret = generate_file_if_not_exists(context, surl, "file:///etc/hosts", &error);
        EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    }

    void TearDown() {
        GError *error = NULL;
        gfal2_unlink(context, surl, &error);
    }
};

const char *XAttrTest::root;


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


TEST_F(XAttrTest, SrmStatus)
{
    if (strncmp(root, "srm://", 6) != 0) {
        SKIP_TEST(SrmType);
        return;
    }

    GError *error = NULL;
    char buffer[1024];

    ssize_t ret = gfal2_getxattr(context, surl, GFAL_XATTR_STATUS, buffer, sizeof(buffer), &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    EXPECT_GT(ret, 0);
    EXPECT_GT(strlen(buffer), 0);

    ret = gfal2_listxattr(context, surl, buffer, sizeof(buffer), &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    EXPECT_GT(ret, 0);

    bool found = false;
    int i = 0;
    while (i < ret) {
        if (strncmp(buffer + i, GFAL_XATTR_STATUS, sizeof(GFAL_XATTR_STATUS)) == 0) {
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
        SKIP_TEST(SrmType);
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


int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 2) {
        printf("Missing base url\n");
        printf("\t%s [options] srm://host/base/path/\n", argv[0]);
        return 1;
    }

    XAttrTest::root = argv[1];

    return RUN_ALL_TESTS();
}
