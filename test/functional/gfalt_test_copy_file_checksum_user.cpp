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

#include <gfal_api.h>
#include <utils/exceptions/gerror_to_cpp.h>

#include <common/gfal_lib_test.h>
#include <common/gfal_gtest_asserts.h>


static void event_callback(const gfalt_event_t e, gpointer user_data);


class CopyTestUserChecksum: public testing::Test {
public:
    static const char* source_root;
    static const char* destination_root;

    char source[2048];
    char destination[2048];
    gfal2_context_t handle;
    gfalt_params_t params;
    bool transfer_happened;
    bool isThirdPartyCopy, mustBeThirdPartyCopy;

    CopyTestUserChecksum(): transfer_happened(false) {
        GError *error = NULL;
        handle =  gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
        params = gfalt_params_handle_new(NULL);
        gfalt_add_event_callback(params, &event_callback, this, NULL, NULL);
        mustBeThirdPartyCopy = is_same_scheme(source_root, destination_root);
    }

    virtual ~CopyTestUserChecksum() {
        gfal2_context_free(handle);
        gfalt_params_handle_delete(params, NULL);
    }

    virtual void SetUp() {
        isThirdPartyCopy = false;
        transfer_happened = false;
        gfalt_set_checksum(params, GFALT_CHECKSUM_BOTH, NULL, NULL, NULL);

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
        gfal_posix_clear_error();
        gfal_unlink(destination);
        gfal_posix_clear_error();
    }

    void VerifyThirdPartyCopy() {
        if (mustBeThirdPartyCopy) {
            EXPECT_TRUE(isThirdPartyCopy);
        }
        else {
            EXPECT_FALSE(isThirdPartyCopy);
        }
    }
};


void event_callback(const gfalt_event_t e, gpointer user_data)
{
    CopyTestUserChecksum *copyTest = static_cast<CopyTestUserChecksum*>(user_data);
    if (e->stage == GFAL_EVENT_TRANSFER_ENTER) {
        copyTest->transfer_happened = true;
    }
    if (e->stage == GFAL_EVENT_TRANSFER_TYPE) {
        copyTest->isThirdPartyCopy = (strncmp(e->description, "3rd", 3) == 0);
        gfal2_log(G_LOG_LEVEL_INFO, "Third party copy");
    }
}


const char* CopyTestUserChecksum::source_root;
const char* CopyTestUserChecksum::destination_root;


// Enabling checksum with the correct checksum, the copy must succeed
TEST_F(CopyTestUserChecksum, CopyRightUserChecksum)
{
    GError* error = NULL;
    int ret = 0;

    char checksum_source[2048];
    ret = gfal2_checksum(handle, source, "ADLER32", 0, 0, checksum_source, sizeof(checksum_source), &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    gfalt_set_checksum(params, GFALT_CHECKSUM_BOTH, "ADLER32", checksum_source, NULL);

    ret = gfalt_copy_file(handle, params, source, destination, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    VerifyThirdPartyCopy();
}

// Enabling checksum with a wrong checksum, the copy must fail
// There must be no copy, since the source would not pass the validation
TEST_F(CopyTestUserChecksum, CopyBadChecksum)
{
    GError* error = NULL;
    int ret = 0;

    gfalt_set_checksum(params, GFALT_CHECKSUM_BOTH, "ADLER32", "aaaaaaa", NULL);

    ret = gfalt_copy_file(handle, params, source, destination, &error);
    EXPECT_PRED_FORMAT3(AssertGfalErrno, ret, error, EIO);
    EXPECT_FALSE(transfer_happened);
}

// Enabling checksum for the source only with the correct checksum, the copy must succeed
TEST_F(CopyTestUserChecksum, CopyRightUserChecksumSource)
{
    GError* error = NULL;
    int ret = 0;

    char checksum_source[2048];
    ret = gfal2_checksum(handle, source, "ADLER32", 0, 0, checksum_source, sizeof(checksum_source), &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    gfalt_set_checksum(params, GFALT_CHECKSUM_SOURCE, "ADLER32", checksum_source, NULL);

    ret = gfalt_copy_file(handle, params, source, destination, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    VerifyThirdPartyCopy();
}

// Enabling checksum for the destination only with the correct checksum, the copy must succeed
TEST_F(CopyTestUserChecksum, CopyRightUserChecksumDestination)
{
    GError* error = NULL;
    int ret = 0;

    char checksum_source[2048];
    ret = gfal2_checksum(handle, source, "ADLER32", 0, 0, checksum_source, sizeof(checksum_source), &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    gfalt_set_checksum(params, GFALT_CHECKSUM_TARGET, "ADLER32", checksum_source, NULL);

    ret = gfalt_copy_file(handle, params, source, destination, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    VerifyThirdPartyCopy();
}

// Enabling checksum for the source only with a wrong checksum, the copy must fail
TEST_F(CopyTestUserChecksum, CopyRightUserChecksumSourceBad)
{
    GError* error = NULL;
    int ret = 0;

    gfalt_set_checksum(params, GFALT_CHECKSUM_SOURCE, "ADLER32", "aaaaaaa", NULL);

    ret = gfalt_copy_file(handle, params, source, destination, &error);
    EXPECT_PRED_FORMAT3(AssertGfalErrno, ret, error, EIO);
    EXPECT_FALSE(transfer_happened);
}

// Enabling checksum for the destination only with a wrong checksum, the copy must fail
// Note that the transfer must happen even if the source file does not match!
TEST_F(CopyTestUserChecksum, CopyRightUserChecksumDestinationBad)
{
    GError* error = NULL;
    int ret = 0;

    gfalt_set_checksum(params, GFALT_CHECKSUM_TARGET, "ADLER32", "aaaaaaa", NULL);

    ret = gfalt_copy_file(handle, params, source, destination, &error);
    EXPECT_PRED_FORMAT3(AssertGfalErrno, ret, error, EIO);
    EXPECT_TRUE(transfer_happened);
    VerifyThirdPartyCopy();
}

// Disabling checksum with a wrong checksum, the copy must succeed
TEST_F(CopyTestUserChecksum, CopyRightUserChecksumDisabledBad)
{
    GError* error = NULL;
    int ret = 0;

    gfalt_set_checksum(params, GFALT_CHECKSUM_NONE, "ADLER32", "aaaaaaa", NULL);

    ret = gfalt_copy_file(handle, params, source, destination, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    VerifyThirdPartyCopy();
}

// Set source or destination, with empty user defined
TEST_F(CopyTestUserChecksum, CopyBadChecksumValue)
{
    GError* error = NULL;
    int ret = gfalt_set_checksum(params, GFALT_CHECKSUM_SOURCE, "ADLER32", "", &error);
    EXPECT_PRED_FORMAT3(AssertGfalErrno, ret, error, EINVAL);
    g_clear_error(&error);

    ret = gfalt_set_checksum(params, GFALT_CHECKSUM_TARGET, "ADLER32", "", &error);
    EXPECT_PRED_FORMAT3(AssertGfalErrno, ret, error, EINVAL);
    g_clear_error(&error);

    ret = gfalt_set_checksum(params, GFALT_CHECKSUM_BOTH, "ADLER32", "", &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    g_clear_error(&error);
}



int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 3) {
        printf("Missing source and destination base urls\n");
        printf("\t%s [options] srm://host/base/path/ srm://destination/base/path/\n", argv[0]);
        return 1;
    }

    CopyTestUserChecksum::source_root = argv[1];
    CopyTestUserChecksum::destination_root = argv[2];

    // gfal2_log_set_level(G_LOG_LEVEL_DEBUG);

    return RUN_ALL_TESTS();
}
