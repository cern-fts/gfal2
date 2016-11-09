/**
 * Compile command : gcc -o gfalt_copyfile gfalt_copyfile.c `pkg-config --libs --cflags gfal_transfer`
 */
#include <gtest/gtest.h>

#include <gfal_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <utils/exceptions/gerror_to_cpp.h>
#include <transfer/gfal_transfer.h>

#include <common/gfal_lib_test.h>
#include <common/gfal_gtest_asserts.h>


class CopyTestReplace: public testing::Test {
public:
    static const char* source_root;
    static const char* destination_root;

    char source[2048];
    char destination[2048];
    gfal2_context_t handle;
    gfalt_params_t params;

    CopyTestReplace() {
        GError *error = NULL;
        handle =  gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
        params = gfalt_params_handle_new(NULL);
    }

    virtual ~CopyTestReplace() {
        gfal2_context_free(handle);
        gfalt_params_handle_delete(params, NULL);
    }

    virtual void SetUp() {
        generate_random_uri(source_root, "copyfile_replace_source", source, 2048);
        generate_random_uri(destination_root, "copyfile_replace", destination, 2048);

        RecordProperty("Source", source);
        RecordProperty("Destination", source);

        GError* error = NULL;
        int ret = generate_file_if_not_exists(handle, source, "file:///etc/hosts", &error);
        EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    }

    virtual void TearDown() {
        gfal_unlink(source);
        gfal_unlink(destination);
    }
};

const char* CopyTestReplace::source_root;
const char* CopyTestReplace::destination_root;


TEST_F(CopyTestReplace, CopyReplaceOverwriteNotSet)
{
    GError* error = NULL;
    int ret = 0;

    ret = generate_file_if_not_exists(handle, destination, "file:///etc/hosts", &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfalt_copy_file(handle, NULL, source, destination, &error);
    EXPECT_PRED_FORMAT3(AssertGfalErrno, ret, error, EEXIST);
}


TEST_F(CopyTestReplace, CopyReplaceOverwriteSet)
{
    GError* error = NULL;
    int ret = 0;

    ret = generate_file_if_not_exists(handle, destination, "file:///etc/hosts", &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfalt_set_replace_existing_file(params, TRUE, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfalt_copy_file(handle, params, source, destination, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
}


TEST_F(CopyTestReplace, CopyReplaceOverwriteSet2)
{
    GError* error = NULL;
    int ret = 0;

    struct stat st;
    ret = gfal2_stat(handle, destination, &st, &error);
    EXPECT_PRED_FORMAT3(AssertGfalErrno, ret, error, ENOENT);

    g_error_free(error); error = NULL;

    ret = gfalt_set_replace_existing_file(params, TRUE, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfalt_copy_file(handle, params, source, destination, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
}


int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 3) {
        printf("Missing source and destination base urls\n");
        printf("\t%s [options] srm://host/base/path/ srm://destination/base/path/\n", argv[0]);
        return 1;
    }

    CopyTestReplace::source_root = argv[1];
    CopyTestReplace::destination_root = argv[2];

    // gfal2_log_set_level(G_LOG_LEVEL_DEBUG);

    return RUN_ALL_TESTS();
}
