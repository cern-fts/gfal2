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


class CopyTestTimeout: public testing::Test {
public:
    static const char* source_root;
    static const char* destination_root;

    char source[2048];
    char destination[2048];
    gfal2_context_t handle;
    gfalt_params_t params;
    bool isThirdPartyCopy, mustBeThirdPartyCopy;

    CopyTestTimeout() {
        GError *error = NULL;
        handle =  gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
        params = gfalt_params_handle_new(NULL);
        gfalt_add_event_callback(params, &event_callback, this, NULL, NULL);
        mustBeThirdPartyCopy = expect_third_party_copy(source_root, destination_root);
    }

    virtual ~CopyTestTimeout() {
        gfal2_context_free(handle);
        gfalt_params_handle_delete(params, NULL);
    }

    virtual void SetUp() {
        isThirdPartyCopy = false;
        generate_random_uri(source_root, "copyfile_timeout_source", source, 2048);
        generate_random_uri(destination_root, "copyfile_timeout", destination, 2048);

        RecordProperty("Source", source);
        RecordProperty("Destination", source);

        GError* error = NULL;
        int ret = generate_file_if_not_exists(handle, source, "file:///etc/hosts", &error);
        EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    }

    virtual void TearDown() {
        GError *error = NULL;
        gfal2_unlink(handle, source, &error);
        g_clear_error(&error);
        gfal2_unlink(handle, destination, &error);
        g_clear_error(&error);
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
    CopyTestTimeout *copyTest = static_cast<CopyTestTimeout*>(user_data);
    if (e->stage == GFAL_EVENT_TRANSFER_TYPE) {
        copyTest->isThirdPartyCopy = (strncmp(e->description, "3rd", 3) == 0);
        gfal2_log(G_LOG_LEVEL_INFO, "Third party copy");
    }
}


const char* CopyTestTimeout::source_root;
const char* CopyTestTimeout::destination_root;


TEST_F(CopyTestTimeout, CopyTimeout)
{
    GError* error = NULL;
    int ret = 0;

    ret = gfalt_set_timeout(params, 1, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfalt_copy_file(handle, params, source, destination, &error);
    ASSERT_PRED_FORMAT3(AssertGfalErrno, ret, error, ETIMEDOUT);
    VerifyThirdPartyCopy();
}


int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 2) {
        printf("Missing source and destination base urls\n");
        printf("\t%s [options] srm://host/base/path/ srm://destination/base/path/\n", argv[0]);
        return 1;
    }

    CopyTestTimeout::source_root = argv[1];
    CopyTestTimeout::destination_root = argv[2];

    // gfal2_log_set_level(G_LOG_LEVEL_DEBUG);

    return RUN_ALL_TESTS();
}
