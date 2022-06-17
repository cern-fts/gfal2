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
#include <stdio.h>
#include <stdlib.h>

#include <gfal_api.h>
#include <uri/gfal2_uri.h>
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
        GError *error = NULL;
        gfal2_unlink(handle, surl, &error);
        g_clear_error(&error);
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
    ASSERT_GE(ret,0);
}


// Asynchronous call, one single file
TEST_F(BringonlineTest, SingleBringOnlineAsync)
{
    GError* error = NULL;
    char token[64] = {0};
    int ret;
    ret = gfal2_bring_online(handle, surl, 10, 28800, token, sizeof(token), TRUE, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    if (ret == 0) {
        ASSERT_NE(0, token[0]);
        while (ret == 0) {
            sleep(1);
            printf("Poll\n");
            ret = gfal2_bring_online_poll(handle, surl, token, &error);
        }
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

    ret = gfal2_bring_online_list(handle, 2, surls, 10, 28800, token, sizeof(token), FALSE, error);
    ASSERT_GE(ret,0);

    // Tape REST API returns a success even if files in the request do not exist
    if (ret == 0) {
        ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error[0]);
    }
    else {
        ASSERT_PRED_FORMAT3(AssertGfalErrno, -1, error[0], ENOENT);
    }
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

    ret = gfal2_bring_online_list(handle, 2, surls, 10, 28800, token, sizeof(token), TRUE, error);

    if (ret == 0) {
        ASSERT_NE(0, token[0]);
        while (ret == 0) {
            sleep(1);
            g_clear_error(&error[0]);
            g_clear_error(&error[1]);

            printf("Poll\n");
            ret = gfal2_bring_online_poll_list(handle, 2, surls, token, error);
            if (error[0] != NULL) {
                ASSERT_TRUE(error[0]->code == EAGAIN || error[0]->code == ENOENT || error[0]->code == ENOMSG);
                if (error[0]->code == EAGAIN) {
                    g_clear_error(&error[0]);
                }
            }
            if (error[1] != NULL) {
                ASSERT_EQ(error[1]->code, EAGAIN);
                g_clear_error(&error[1]);
            }
        }
    }

    ASSERT_GT(ret, 0);

    ASSERT_PRED_FORMAT3(AssertGfalOneOfErrno, -1, error[0], (std::list<int>{ENOENT,ENOMSG}));
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, 1, error[1]);
}

// Synchronous call, release
TEST_F(BringonlineTest, SingleReleaseSync)
{
    GError* error = NULL;
    char token[64];
    int ret;
    ret = gfal2_bring_online(handle, surl, 10, 28800, token, sizeof(token), FALSE, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    ASSERT_GE(ret,0);

    if (token[0]) {
        printf("Release\n");
        ret = gfal2_release_file(handle, surl, token, &error);
        ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    }
}

// Asynchronous call, two files, abort
// Note that Castor returns an error (EIO) when a file has been processed before the abort
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

    ret = gfal2_bring_online_list(handle, 2, surls, 10, 28800, token, sizeof(token), TRUE, error);

    if (ret == 0 && token[0]) {
        ret = gfal2_abort_files(handle, 2, surls, token, error);
        ASSERT_TRUE(error[0] == NULL || error[0]->code == ENOENT);
        ASSERT_TRUE(error[1] == NULL || error[1]->code == EIO || error[1]->code == ENOENT);

        while (ret == 0) {
            sleep(1);
            printf("Poll\n");
            g_clear_error(&error[0]);
            g_clear_error(&error[1]);
            ret = gfal2_bring_online_poll_list(handle, 2, surls, token, error);
        }

        ASSERT_TRUE(error[0]->code == ECANCELED || error[0]->code == ENOENT || error[0]->code == ENOMSG);
        ASSERT_TRUE(error[1] == NULL ||error[1]->code == ECANCELED || error[1]->code == EIO ||
                    error[1]->code == ENOENT);
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
    // With xroot you can poll with an invalid token
    if(ret < 0) {
        ASSERT_TRUE(error[0]->code == EBADR || error[0]->code == EIO || error[0]->code == EINVAL ||
                    error[0]->code == ENOMSG);
        ASSERT_TRUE(error[1]->code == EBADR || error[1]->code == EIO || error[1]->code == EINVAL ||
                    error[1]->code == ENOMSG);
    }
    else{
        ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error[0]);
        ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error[1]);
    }
}

// Release an invalid token
TEST_F(BringonlineTest, InvalidRelease)
{
    GError* error = NULL;
    int ret;

    ret = gfal2_release_file(handle, surl, "1234-5678-badabad", &error);
    // Some storages return a success even if the token does not exist
    if (ret) {
        ASSERT_PRED_FORMAT3(AssertGfalOneOfErrno, ret, error, (std::list<int>{EBADR, EHOSTUNREACH}));
    }
    else {
        ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    }
}

// Poll invalid hostname
TEST_F(BringonlineTest, InvalidHostPoll)
{
    GError* error = NULL;
    char invalid_surl[2048];
    const char* format = "%s://invalid.%sfile.test";
    int ret;

    gfal2_uri* parsed = gfal2_parse_uri(root, &error);
    ASSERT_NE(parsed, (void *) NULL);

    if (root[strlen(root) - 1] != '/') {
        format = "%s://invalid.%s/file.test";
    }

    snprintf(invalid_surl, sizeof(invalid_surl), format,
             parsed->scheme, (root + strlen(parsed->scheme) + 3));

    g_clear_error(&error);
    gfal2_free_uri(parsed);

    ret = gfal2_bring_online_poll(handle, invalid_surl, "bringonline-token", &error);

    ASSERT_EQ(-1, ret);
    ASSERT_PRED_FORMAT3(AssertGfalOneOfErrno, -1, error, (std::list<int>{ECOMM, EHOSTUNREACH}));
}

// Request with duplicated SURLs (see DMC-676)
TEST_F(BringonlineTest, DuplicatedSURLs)
{
    const int nbfiles = 100;
    GError* error[nbfiles];
    char token[64] = {0};
    int ret;

    memset(error, 0x00, sizeof(error));

    char *surls[nbfiles];
    // 0 exists
    surls[0] = surl;
    // 1 does not
    surls[1] = (char*)calloc(1, 2048);
    generate_random_uri(root, "bringonline_duplicated", surls[1], 2048);
    // all the rest are 0 or 1 duplicated
    for (int i = 1; i < nbfiles; ++i) {
        surls[i] = surls[i % 2];
    }

    ret = gfal2_bring_online_list(handle, nbfiles, surls,
            10, 28800, token, sizeof(token), TRUE, error);

    if (ret == 0) {
        ASSERT_NE(0, token[0]);
        while (ret == 0) {
            sleep(1);
            for (int i = 0; i < nbfiles; ++i) {
                g_clear_error(&error[i]);
            }

            printf("Poll\n");
            ret = gfal2_bring_online_poll_list(handle, nbfiles, surls, token, error);

            for (int i = 0; i < nbfiles; ++i) {
                if (error[i] != NULL) {
                    if (i % 2 == 0) {
                        ASSERT_EQ(EAGAIN, error[i]->code);
                    }
                    else {
                        ASSERT_TRUE(error[i]->code == EAGAIN || error[i]->code == ENOENT || error[i]->code == ENOMSG);
                    }
                    if (error[i] && error[i]->code == EAGAIN) {
                        g_clear_error(&error[i]);
                    }
                }
            }
        }
    }

    // Only the first one and duplicated should be successful
    for (int i = 0; i < nbfiles; ++i) {
        if (i % 2 == 0)
            ASSERT_PRED_FORMAT2(AssertGfalSuccess, 1, error[i]);
        else
            ASSERT_PRED_FORMAT3(AssertGfalOneOfErrno, -1, error[i], (std::list<int>{ENOENT,ENOMSG}));
    }

    free(surls[1]);
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
