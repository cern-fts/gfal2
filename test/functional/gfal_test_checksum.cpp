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
#include <exceptions/gfalcoreexception.hpp>
#include <utils/exceptions/gerror_to_cpp.h>
#include <utils/checksums/checksums.h>


// Given an algorithm, returns a pre-calculated content with the hash in the requested
// algorithm
static int get_precomputed(const char* algorithm, const char** content,
        const char** checksum)
{
    static const char *message = "THIS IS A STATIC MESSAGE USED FOR CHECKSUMING";
    static struct {
        const char* algorithm;
        const char* hash;
    } precomputed[] = {
        { "MD5", "d0fb2562f39431dbe78c140581ca06b8" },
        { "SHA1", "87c813e21babb87879588dc6009770f3c2b5d2f6" },
        { "ADLER32", "111a0c0c" },
        { "CRC32", "3971548410" },
        { NULL, NULL }
    };

    *content = message;
    size_t i;
    for (i = 0; precomputed[i].algorithm != NULL; ++i) {
        if (strcasecmp(algorithm, precomputed[i].algorithm) == 0) {
            *checksum = precomputed[i].hash;
            return 0;
        }
    }

    return -1;
}


class ChecksumTest: public testing::Test {
public:
    static const char* root;
    static const char* algorithm;

    gfal2_context_t context;
    char surl[2048];

    const char* precomputed_content;
    const char* precomputed_hash;


    ChecksumTest() {
        GError *error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
    }

    virtual ~ChecksumTest() {
        gfal2_context_free(context);
    }

    virtual void SetUp() {
        generate_random_uri(root, "test_checksum", surl, sizeof(surl));
        GError* error = NULL;

        if (get_precomputed(algorithm, &precomputed_content, &precomputed_hash) != 0) {
            throw Gfal::CoreException(g_quark_from_static_string("TestChecksum"),
                    EINVAL, "Could not get the pre-calculated checksum");
        }


        int fd = gfal2_open(context, surl, O_CREAT | O_TRUNC | O_WRONLY, &error);
        Gfal::gerror_to_cpp(&error);

        size_t len = strlen(precomputed_content);
        gfal2_write(context, fd, precomputed_content, len, &error);
        Gfal::gerror_to_cpp(&error);

        gfal2_close(context, fd, &error);
        Gfal::gerror_to_cpp(&error);
    }

    virtual void TearDown() {
        GError* error = NULL;
        gfal2_unlink(context, surl, &error);
        g_clear_error(&error);
    }
};
const char* ChecksumTest::root;
const char* ChecksumTest::algorithm;


TEST_F(ChecksumTest, ValidChecksum)
{
    GError* error = NULL;
    char buffer[512];

    int ret = gfal2_checksum(context, surl, algorithm, 0, 0, buffer, sizeof(buffer), &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfal_compare_checksums(precomputed_hash, buffer, sizeof(buffer));
    EXPECT_EQ(0, ret);
}


int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 3) {
        printf("Missing base url\n");
        printf("usage: %s root_dir [MD5|ADLER32|SHA1|CRC32]\n", argv[0]);
        return 1;
    }

    ChecksumTest::root = argv[1];
    ChecksumTest::algorithm = argv[2];

    return RUN_ALL_TESTS();
}
