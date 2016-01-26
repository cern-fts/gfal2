#include <gtest/gtest.h>
#include <fcntl.h>
#include <stdio.h>
#include <gfal_api.h>
#include <stdlib.h>
#include <common/gfal_lib_test.h>
#include <common/gfal_gtest_asserts.h>
#include <utils/exceptions/gerror_to_cpp.h>


class RenameTest: public testing::Test {
public:
    static const char* root;

    char origin[2048];
    char destination[2048];

    gfal2_context_t context;

    RenameTest() {
        GError *error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
    }

    virtual ~RenameTest() {
        gfal2_context_free(context);
    }

    virtual void SetUp() {
        generate_random_uri(root, "rename_test", origin, sizeof(origin));
        generate_random_uri(root, "rename_test", destination, sizeof(destination));
    }

    virtual void ForceRm(const char *surl) {
        GError* error = NULL;
        if (gfal2_rmdir(context, surl, &error) != 0) {
            g_clear_error(&error);
            gfal2_unlink(context, surl, &error);
        }
        g_clear_error(&error);
    }

    virtual void TearDown() {
        ForceRm(origin);
        ForceRm(destination);
    }
};
const char* RenameTest::root;


// Rename a file to a different name, destination does not exist
TEST_F(RenameTest, SimpleRename)
{
    GError* error = NULL;

    int ret = generate_file_if_not_exists(context, origin, "file:///etc/hosts", &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfal2_rename(context, origin, destination, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    struct stat buf;
    ret = gfal2_stat(context, origin, &buf, &error);
    EXPECT_PRED_FORMAT3(AssertGfalErrno, ret, error, ENOENT);
    g_clear_error(&error);

    ret = gfal2_stat(context, destination, &buf, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
}

// Rename a file, but the new name already exists and is a directory
TEST_F(RenameTest, RenameToExisting)
{
    if (strncmp(origin, "srm://", 6) == 0 || strncmp(origin, "davs", 4) == 0) {
        SKIP_TEST(RenameToExisting);
        return;
    }

    GError* error = NULL;

    int ret = generate_file_if_not_exists(context, origin, "file:///etc/hosts", &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfal2_mkdir(context, destination, 0775, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfal2_rename(context, origin, destination, &error);
    EXPECT_PRED_FORMAT3(AssertGfalErrno, ret, error, EISDIR);
}


int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 2) {
        printf("Missing base url\n");
        printf("\t%s [options] srm://host/base/path/\n", argv[0]);
        return 1;
    }

    RenameTest::root = argv[1];

//    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE | GFAL_VERBOSE_DEBUG);

    return RUN_ALL_TESTS();
}
