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

#include <list>

static void event_callback(const gfalt_event_t e, gpointer user_data);


class CopyTestMkdir: public testing::Test {
public:
    static const char* source_root;
    static const char* destination_root;

    char source[2048];
    char destination[2048];
    gfal2_context_t handle;
    gfalt_params_t params;
    bool isThirdPartyCopy, mustBeThirdPartyCopy;

    std::list<std::string> directories;

    CopyTestMkdir() {
        GError *error = NULL;
        handle =  gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
        params = gfalt_params_handle_new(NULL);
        gfalt_set_create_parent_dir(params, TRUE, NULL);
        gfalt_add_event_callback(params, &event_callback, this, NULL, NULL);
        mustBeThirdPartyCopy = expect_third_party_copy(source_root, destination_root);
    }

    virtual ~CopyTestMkdir() {
        gfal2_context_free(handle);
        gfalt_params_handle_delete(params, NULL);
    }

    virtual void SetUp() {
        isThirdPartyCopy = false;
        generate_random_uri(source_root, "copyfile_replace_source", source, 2048);

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

        std::list<std::string>::const_iterator i;
        for (i = directories.begin(); i != directories.end(); ++i) {
            gfal2_rmdir(handle, i->c_str(), &error);
            g_clear_error(&error);
        }
        directories.clear();
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
    CopyTestMkdir *copyTest = static_cast<CopyTestMkdir*>(user_data);
    if (e->stage == GFAL_EVENT_TRANSFER_TYPE) {
        copyTest->isThirdPartyCopy = (strncmp(e->description, "3rd", 3) == 0);
        gfal2_log(G_LOG_LEVEL_INFO, "Third party copy");
    }
}


const char* CopyTestMkdir::source_root;
const char* CopyTestMkdir::destination_root;


TEST_F(CopyTestMkdir, CopyNested)
{
    char first_level[2048];
    char second_level[2048];
    char third_level[2048];

    generate_random_uri(destination_root, "generate_folder", first_level, 2048);
    generate_random_uri(first_level, "generate_folder2", second_level, 2048);
    generate_random_uri(second_level, "generate_folder3", third_level, 2048);
    generate_random_uri(third_level, "generate_dest_file", destination, 2048);

    directories.push_front(first_level);
    directories.push_front(second_level);
    directories.push_front(third_level);

    GError *error = NULL;
    int ret = 0;

    ret = gfalt_copy_file(handle, params, source, destination, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    VerifyThirdPartyCopy();
}


TEST_F(CopyTestMkdir, CopyAtRoot)
{
    generate_random_uri(destination_root, "simple_file_at_root", destination, 2048);

    GError *error = NULL;
    int ret = 0;

    ret = gfalt_copy_file(handle, params, source, destination, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
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

    CopyTestMkdir::source_root = argv[1];
    CopyTestMkdir::destination_root = argv[2];

    // gfal2_log_set_level(G_LOG_LEVEL_DEBUG);

    return RUN_ALL_TESTS();
}
