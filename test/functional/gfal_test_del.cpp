#include <gtest/gtest.h>
#include <fcntl.h>
#include <stdio.h>
#include <gfal_api.h>
#include <stdlib.h>
#include <common/gfal_lib_test.h>
#include <common/gfal_gtest_asserts.h>
#include <utils/exceptions/gerror_to_cpp.h>

#define BLKLEN 65536


class DeleteTest: public testing::Test {
public:
    static const char* root;
    char nested_file[2048];

    const static int N_FILES = 5;
    char* files[N_FILES];

    gfal2_context_t context;

    DeleteTest() {
        GError *error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);

        for (int i = 0; i < N_FILES; ++i) {
            files[i] = new char[2048];
        }
    }

    virtual ~DeleteTest() {
        gfal2_context_free(context);

        for (int i = 0; i < N_FILES; ++i) {
            delete [] files[i];
        }
    }

    virtual void SetUp() {
        nested_file[0] = '\0';

        int ret;
        GError* error = NULL;

        for (int i = 0; i < N_FILES; ++i) {
            generate_random_uri(root, "test_del", files[i], 2048);
            ret = generate_file_if_not_exists(context, files[i], "file:///etc/hosts", &error);
            EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
        }
    }

    virtual void TearDown() {
        if (nested_file[0])
            gfal_unlink(nested_file);
        for (int i = 0; i < N_FILES; ++i) {
            if (gfal_unlink(files[i]) < 0)
                gfal_rmdir(files[i]);
        }
    }
};

TEST_F(DeleteTest, DeleteSequential)
{
    int ret;
    GError* error = NULL;

    for (int i = 0; i < N_FILES; ++i) {
        ret = gfal2_unlink(context, files[i], &error);
        EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    }

    // Were they really removed?
    struct stat st;
    for (int i = 0; i < N_FILES; ++i) {
        GError *err = NULL;
        ret = gfal2_stat(context, files[i], &st, &err);
        EXPECT_PRED_FORMAT3(AssertGfalErrno, ret, err, ENOENT);
    }
}

TEST_F(DeleteTest, BulkDeletionOddFail)
{
    int ret;

    // Remove odd files to force an error
    for (int i = 0; i < N_FILES; ++i) {
        GError* err = NULL;
        if (i % 2) {
            ret = gfal2_unlink(context, files[i], &err);
            EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, err);
        }
    }

    GError *errors[N_FILES] = {0};
    ret = gfal2_unlink_list(context, N_FILES, files, errors);
    EXPECT_LT(ret, 0);

    for (int i = 0; i < N_FILES; ++i) {
        if (i % 2) {
            EXPECT_PRED_FORMAT3(AssertGfalErrno, -1, errors[i], ENOENT);
        }
        else {
            EXPECT_EQ(NULL, errors[i]);
        }
    }

    // Were they really removed?
    struct stat st;
    for (int i = 0; i < N_FILES; ++i) {
        GError *err = NULL;
        if (i % 2 == 0) {
            ret = gfal2_stat(context, files[i], &st, &err);
            EXPECT_PRED_FORMAT3(AssertGfalErrno, ret, err, ENOENT);
        }
    }
}

TEST_F(DeleteTest, BulkDeletionIsDir)
{
    int ret;
    GError* err = NULL;

    // Remove the first file, create  dir instead
    ret = gfal2_unlink(context, files[0], &err);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, err);
    ret = gfal2_mkdir(context, files[0], 0775, &err);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, err);

    generate_random_uri(files[0], "test_del_nested_files", nested_file, 2048);
    ret = generate_file_if_not_exists(context, nested_file, "file:///etc/hosts", &err);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, err);

    // Call unlink all
    GError *errors[N_FILES] = {0};
    ret = gfal2_unlink_list(context, N_FILES, files, errors);
    EXPECT_LT(ret, 0);

    // The first must fail
    for (int i = 0; i < N_FILES; ++i) {
        if (i == 0) {
            EXPECT_NE((void*)NULL, errors[i]);
        }
        else {
            EXPECT_EQ(NULL, errors[i]);
        }
    }

    // Were they really removed? 0 should have not!
    struct stat st;
    for (int i = 0; i < N_FILES; ++i) {
        GError *err = NULL;
        ret = gfal2_stat(context, files[i], &st, &err);
        if (i != 0) {
            EXPECT_PRED_FORMAT3(AssertGfalErrno, ret, err, ENOENT);
        }
        else {
            EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, err);
        }
    }
}

const char* DeleteTest::root;

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 2) {
        printf("Missing base url\n");
        printf("\t%s [options] srm://host/base/path/\n", argv[0]);
        return 1;
    }

    DeleteTest::root = argv[1];

//    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE | GFAL_VERBOSE_DEBUG);

    return RUN_ALL_TESTS();
}
