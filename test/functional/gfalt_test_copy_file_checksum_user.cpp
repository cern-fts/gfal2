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

class CopyTestUserChecksum: public testing::Test {
public:
    static const char* source_root;
    static const char* destination_root;

    char source[2048];
    char destination[2048];
    gfal2_context_t handle;
    gfalt_params_t params;

    CopyTestUserChecksum() {
        GError *error = NULL;
        handle =  gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
        params = gfalt_params_handle_new(NULL);
        gfalt_set_checksum_check(params, TRUE, NULL);
    }

    virtual ~CopyTestUserChecksum() {
        gfal2_context_free(handle);
        gfalt_params_handle_delete(params, NULL);
    }

    virtual void SetUp() {
        generate_random_uri(source_root, "copyfile_checksum_user_source", source, 2048);
        generate_random_uri(destination_root, "copyfile_checksum_user", destination, 2048);

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

const char* CopyTestUserChecksum::source_root;
const char* CopyTestUserChecksum::destination_root;


TEST_F(CopyTestUserChecksum, CopyRightUserChecksum)
{
    GError* error = NULL;
    int ret = 0;

    char checksum_source[2048];
    ret = gfal2_checksum(handle, source, "ADLER32", 0, 0, checksum_source, sizeof(checksum_source), &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    gfalt_set_user_defined_checksum(params, "ADLER32", checksum_source, NULL);

    ret = gfalt_copy_file(handle, params, source, destination, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
}


TEST_F(CopyTestUserChecksum, CopyBadChecksum)
{
    GError* error = NULL;
    int ret = 0;

    gfalt_set_user_defined_checksum(params, "ADLER32", "aaaaaaa", NULL);

    ret = gfalt_copy_file(handle, params, source, destination, &error);
    EXPECT_PRED_FORMAT3(AssertGfalErrno, ret, error, EIO);
}


int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 2) {
        printf("Missing source and destination base urls\n");
        printf("\t%s [options] srm://host/base/path/ srm://destination/base/path/\n", argv[0]);
        return 1;
    }

    CopyTestUserChecksum::source_root = argv[1];
    CopyTestUserChecksum::destination_root = argv[2];

//    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE | GFAL_VERBOSE_DEBUG);

    return RUN_ALL_TESTS();
}
