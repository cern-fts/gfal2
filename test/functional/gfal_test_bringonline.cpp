#include <gtest/gtest.h>

#include <gfal_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <utils/exceptions/gerror_to_cpp.h>
#include <transfer/gfal_transfer.h>

#include <common/gfal_lib_test.h>
#include <common/gfal_gtest_asserts.h>


class BringonlineTest: public testing::Test {
public:
    static const char* root;

    char surl[2048];
    gfal2_context_t handle;

    BringonlineTest() {
        GError *error = NULL;
        handle =  gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
    }

    virtual ~BringonlineTest() {
        gfal2_context_free(handle);
    }

    virtual void SetUp() {
        generate_random_uri(root, "bringonline", surl, 2048);

        RecordProperty("Surl", surl);

        GError* error = NULL;
        int ret = generate_file_if_not_exists(handle, surl, "file:///etc/hosts", &error);
        EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    }

    virtual void TearDown() {
        gfal_unlink(surl);
    }
};

const char* BringonlineTest::root;


// Synchronous call, one single file
TEST_F(BringonlineTest, SingleBringOnlineSync)
{
    GError* error = NULL;
    char token[64];
    int ret;
    ret = gfal2_bring_online(handle, surl, 10, 28800, token, sizeof(token), 0, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    ASSERT_EQ(1, ret);
}


// Asynchronous call, one single file
TEST_F(BringonlineTest, SingleBringOnlineAsync)
{
    GError* error = NULL;
    char token[64] = {0};
    int ret;
    ret = gfal2_bring_online(handle, surl, 10, 28800, token, sizeof(token), 1, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    if (ret == 0) {
        ASSERT_NE(0, token[0]);
        printf("Poll\n");
        ret = gfal2_bring_online_poll(handle, surl, token, &error);
    }
}

// Synchronous call, two files (one does not exist)
TEST_F(BringonlineTest, TwoBringOnlineSync)
{
    GError* error[2] = {NULL, NULL};
    char token[64] = {0};
    int ret;

    char not_exist[2048];
    generate_random_uri(root, "bringonline_enoent", not_exist, sizeof(not_exist));

    char* surls[] = {
            not_exist,
            surl
    };

    ret = gfal2_bring_online_list(handle, 2, surls, 10, 28800, token, sizeof(token), 0, error);
    ASSERT_EQ(1, ret);

    ASSERT_PRED_FORMAT3(AssertGfalErrno, -1, error[0], ENOENT);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, 1, error[1]);
}

// Asynchronous call, two files (one does not exist)
TEST_F(BringonlineTest, TwoBringOnlineAsync)
{
    GError* error[2] = {NULL, NULL};
    char token[64] = {0};
    int ret;

    char not_exist[2048];
    generate_random_uri(root, "bringonline_enoent", not_exist, sizeof(not_exist));

    char* surls[] = {
            not_exist,
            surl
    };

    ret = gfal2_bring_online_list(handle, 2, surls, 10, 28800, token, sizeof(token), 1, error);
    if (ret == 0) {
        ASSERT_NE(0, token[0]);
        printf("Poll\n");
        ret = gfal2_bring_online_poll_list(handle, 2, surls, token, error);
    }

    ASSERT_EQ(1, ret);

    ASSERT_PRED_FORMAT3(AssertGfalErrno, -1, error[0], ENOENT);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, 1, error[1]);
}

// Synchronous call, release
TEST_F(BringonlineTest, SingleReleaseSync)
{
    GError* error = NULL;
    char token[64];
    int ret;
    ret = gfal2_bring_online(handle, surl, 10, 28800, token, sizeof(token), 0, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    ASSERT_EQ(1, ret);

    if (token[0]) {
        printf("Release\n");
        ret = gfal2_release_file(handle, surl, token, &error);
        ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    }
}

// Asynchronous call, two files, abort
TEST_F(BringonlineTest, TwoAbort)
{
    GError* error[2] = {NULL, NULL};
    char token[64] = {0};
    int ret;

    char not_exist[2048];
    generate_random_uri(root, "bringonline_enoent", not_exist, sizeof(not_exist));

    char* surls[] = {
            not_exist,
            surl
    };

    ret = gfal2_bring_online_list(handle, 2, surls, 10, 28800, token, sizeof(token), 1, error);
    if (ret == 0 && token[0]) {
        ret = gfal2_abort_files(handle, 2, surls, token, error);
        ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error[0]);
        ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error[1]);

        ret = gfal2_bring_online_poll_list(handle, 2, surls, token, error);
        ASSERT_EQ(1, ret);
        ASSERT_PRED_FORMAT3(AssertGfalErrno, -1, error[0], ECANCELED);
        ASSERT_PRED_FORMAT3(AssertGfalErrno, -1, error[1], ECANCELED);
    }
}

// Poll with an invalid token
TEST_F(BringonlineTest, InvalidPoll)
{
    GError* error[2] = {NULL, NULL};
    int ret;

    char* surls[] = {
            surl,
            surl
    };

    ret = gfal2_bring_online_poll_list(handle, 2, surls, "1234-5678-badabad", error);
    ASSERT_LT(ret, 0);
    ASSERT_TRUE(error[0]->code == EBADR || error[0]->code == EIO);
    ASSERT_TRUE(error[1]->code == EBADR || error[1]->code == EIO);
}

// Release an invalid token
TEST_F(BringonlineTest, InvalidRelease)
{
    GError* error = NULL;
    int ret;

    ret = gfal2_release_file(handle, surl, "1234-5678-badabad", &error);
    ASSERT_PRED_FORMAT3(AssertGfalErrno, ret, error, EBADR);
}


int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 2) {
        printf("Missing base urls\n");
        printf("\t%s [options] srm://host/base/path/\n", argv[0]);
        return 1;
    }

    BringonlineTest::root = argv[1];

    return RUN_ALL_TESTS();
}
