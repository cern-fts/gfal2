#include <gtest/gtest.h>

#include <gfal_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <utils/exceptions/gerror_to_cpp.h>
#include <transfer/gfal_transfer.h>

#include <common/gfal_lib_test.h>
#include <common/gfal_gtest_asserts.h>


void transfer_callback(const gfalt_event_t e, gpointer user_data)
{
    puts(e->description);
}


class CopyBulk: public testing::Test {
public:
    static const char* source_root;
    static const char* destination_root;

    static const size_t NBPAIRS = 5;

    char *sources[NBPAIRS];
    char *destinations[NBPAIRS];

    gfal2_context_t handle;
    gfalt_params_t params;

    CopyBulk() {
        GError *error = NULL;
        handle =  gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
        params = gfalt_params_handle_new(NULL);
        gfalt_set_checksum_check(params, TRUE, NULL);
        gfalt_set_event_callback(params, transfer_callback, NULL);

        for (size_t i = 0; i < NBPAIRS; ++i) {
            sources[i] = new char[2048];
            destinations[i] = new char[2048];
        }
    }

    virtual ~CopyBulk() {
        gfal2_context_free(handle);
        gfalt_params_handle_delete(params, NULL);

        for (size_t i = 0; i < NBPAIRS; ++i) {
            delete [] sources[i];
            delete [] destinations[i];
        }
    }

    virtual void SetUp() {
        char source_base[2048];
        char dest_base[2048];

        generate_random_uri(source_root, "copyfile_bulk_user_source", source_base, 2048);
        generate_random_uri(destination_root, "copyfile_bulk_user_destination", dest_base, 2048);

        for (size_t i = 0; i < NBPAIRS; ++i) {
            snprintf(sources[i], 2048, "%s_%zu", source_base, i);
            snprintf(destinations[i], 2048, "%s_%zu", dest_base, i);

            GError* error = NULL;
            int ret = generate_file_if_not_exists(handle, sources[i], "file:///etc/hosts", &error);
            EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
        }
    }

    virtual void TearDown() {
        for (size_t i = 0; i < NBPAIRS; ++i) {
            gfal_unlink(sources[i]);
            gfal_unlink(destinations[i]);
        }
    }
};

const char* CopyBulk::source_root;
const char* CopyBulk::destination_root;


TEST_F(CopyBulk, CopyBulk)
{
    GError* op_error = NULL;
    GError** file_errors = NULL;
    int ret = 0;

    ret = gfalt_copy_bulk(handle, params, NBPAIRS, sources, destinations, NULL, &op_error, &file_errors);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, op_error);

    if (file_errors) {
        for (size_t i = 0; i < NBPAIRS; ++i) {
            if (file_errors[i]) {
                EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, file_errors[i]);
                g_error_free(file_errors[i]);
            }
        }
        g_free(file_errors);
    }

    if (op_error)
        g_error_free(op_error);

    // Do not trust! Make sure they do exist
    if (ret == 0) {
        struct stat st;
        GError* tmp_err = NULL;
        for (size_t i = 0; i < NBPAIRS; ++i) {
            ret = gfal2_stat(handle, destinations[i], &st, &tmp_err);
            EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, tmp_err);
            if (tmp_err)
                g_error_free(tmp_err);
            tmp_err = NULL;
        }
    }
}
/*
TEST_F(CopyBulk, CopyBulkSomeFail)
{
    // Remove source for even transfers, they should fail
    int removed = 0;
    for (size_t i = 0; i < NBPAIRS; ++i) {
        if (i % 2 == 0) {
            GError* tmp_err = NULL;
            gfal2_unlink(handle, sources[i], &tmp_err);
            if (tmp_err)
                g_error_free(tmp_err);
            ++removed;
        }
    }

    GError* op_error = NULL;
    GError** file_errors = NULL;
    int ret = 0;

    ret = gfalt_copy_bulk(handle, params, NBPAIRS, sources, destinations, NULL, &op_error, &file_errors);
    ASSERT_EQ(-removed, ret);
    ASSERT_NE((void*)NULL, file_errors);

    if (file_errors) {
        for (size_t i = 0; i < NBPAIRS; ++i) {
            if (i % 2 == 0) {
                EXPECT_PRED_FORMAT3(AssertGfalErrno, ret, file_errors[i], ENOENT);
            }
            else {
                EXPECT_PRED_FORMAT2(AssertGfalSuccess, 0, file_errors[i]);
            }
            if (file_errors[i])
                g_error_free(file_errors[i]);
        }
        g_free(file_errors);
    }

    if (op_error)
        g_error_free(op_error);
}
*/

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 3) {
        printf("Missing source and destination base urls\n");
        printf("\t%s [options] srm://host/base/path/ srm://destination/base/path/\n", argv[0]);
        return 1;
    }

    CopyBulk::source_root = argv[1];
    CopyBulk::destination_root = argv[2];

    for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "-v") == 0)
            gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE | GFAL_VERBOSE_DEBUG);
    }

    return RUN_ALL_TESTS();
}
