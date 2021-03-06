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
#include <fcntl.h>
#include <stdio.h>
#include <gfal_api.h>
#include <stdlib.h>
#include <common/gfal_lib_test.h>
#include <common/gfal_gtest_asserts.h>
#include <utils/exceptions/gerror_to_cpp.h>

#define BLKLEN 65536


class RwFullTest: public testing::Test {
public:
    static const char* root;
    static int size;

    char surl[2048];
    gfal2_context_t context;

    char *original_data;

    RwFullTest() {
        GError *error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);

        original_data = new char[size];
        int i;
        for (i = 0; i < size; ++i)
            original_data[i] = i;
    }

    virtual ~RwFullTest() {
        gfal2_context_free(context);
        delete [] original_data;
    }

    virtual void SetUp() {
        generate_random_uri(root, "test_rw_full", surl, sizeof(surl));
    }

    virtual void TearDown() {
        GError* error = NULL;
        gfal2_unlink(context, surl, &error);
        g_clear_error(&error);
    }
};
const char* RwFullTest::root;
int RwFullTest::size = 0;


TEST_F(RwFullTest, OpenENOENT)
{
    GError* error = NULL;
    int ret = gfal2_open(context, surl, O_RDONLY, &error);
    EXPECT_PRED_FORMAT3(AssertGfalErrno, ret, error, ENOENT);
}


TEST_F(RwFullTest, WriteAndRead)
{
    GError* error = NULL;
    int fd = gfal2_open(context, surl, O_WRONLY | O_CREAT, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, fd, error);

    int ret = gfal2_write(context, fd, original_data, size, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfal2_close(context, fd, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    fd = gfal2_open(context, surl, O_RDONLY, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, fd, error);

    char buffer[size];
    ret = gfal2_read(context, fd, buffer, size, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, fd, error);

    ret = gfal2_close(context, fd, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    EXPECT_EQ(0, memcmp(original_data, buffer, size));
}


int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc != 3) {
        printf("Missing base url and/or size\n");
        printf("\t%s [options] srm://host/base/path/ 1024\n", argv[0]);
        return 1;
    }

    RwFullTest::root = argv[1];
    RwFullTest::size = atol(argv[2]);

    return RUN_ALL_TESTS();
}
