/*
 * Copyright (c) CERN 2017
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
#include <exceptions/gerror_to_cpp.h>
#include <common/gfal_lib_test.h>
#include <common/gfal_gtest_asserts.h>


class CopyDoubleCredTest: public testing::Test {
public:
    static const char *source_root, *destination_root;
    static const char *source_proxy, *destination_proxy;

    gfal2_context_t handle;
    gfalt_params_t params;
    gfal2_cred_t *source_cred, *dest_cred;

    char source[2048];
    char destination[2048];

    CopyDoubleCredTest() {
        source_cred = gfal2_cred_new(GFAL_CRED_X509_CERT, source_proxy);
        dest_cred = gfal2_cred_new(GFAL_CRED_X509_CERT, destination_proxy);
    }

    virtual ~CopyDoubleCredTest() {
        gfal2_cred_free(source_cred);
        gfal2_cred_free(dest_cred);
    }

    void SetUp() {
        unsetenv("X509_USER_PROXY");
        unsetenv("X509_USER_CERT");
        unsetenv("X509_USER_KEY");

        int ret;
        GError *error = NULL;

        handle = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
        params = gfalt_params_handle_new(&error);
        Gfal::gerror_to_cpp(&error);

        ret = gfal2_cred_clean(handle, &error);
        ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

        // Set configured values
        ret = gfal2_cred_set(handle, source_root, source_cred, &error);
        ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
        ret = gfal2_cred_set(handle, destination_root, dest_cred, &error);
        ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

        {
            setenv("X509_USER_PROXY", source_proxy, 1);
            generate_random_uri(source_root, "copyfile_doublecred_source", source, 2048);
            ret = generate_file_if_not_exists(handle, source, "file:///etc/hosts", &error);
            ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
            unsetenv("X509_USER_PROXY");
        }

        generate_random_uri(destination_root, "copyfile_doublecred_dest", destination, 2048);

        // Just make sure the environment is well prepared
        ASSERT_EQ((char*)NULL, getenv("X509_USER_CERT"));
        ASSERT_EQ((char*)NULL, getenv("X509_USER_KEY"));
        ASSERT_EQ((char*)NULL, getenv("X509_USER_PROXY"));

        ASSERT_EQ((gchar*)NULL, gfal2_get_opt_string_with_default(handle, "X509", "CERT", NULL));
        ASSERT_EQ((gchar*)NULL, gfal2_get_opt_string_with_default(handle, "X509", "KEY", NULL));
    }

    void TearDown() {
        gfalt_params_handle_delete(params, NULL);
        gfal2_context_free(handle);

        {
            setenv("X509_USER_PROXY", source_proxy, 1);
            gfal_unlink(source);
            gfal_posix_clear_error();
            unsetenv("X509_USER_PROXY");
        }
        {
            setenv("X509_USER_PROXY", destination_proxy, 1);
            gfal_unlink(destination);
            gfal_posix_clear_error();
            unsetenv("X509_USER_PROXY");
        }
    }
};


const char *CopyDoubleCredTest::source_root = NULL;
const char *CopyDoubleCredTest::destination_root = NULL;
const char *CopyDoubleCredTest::source_proxy = NULL;
const char *CopyDoubleCredTest::destination_proxy = NULL;


TEST_F(CopyDoubleCredTest, Stat)
{
    GError *error = NULL;
    int ret;

    ret = gfal2_cred_set(handle, source_root, source_cred, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    struct stat buf;
    ret = gfal2_stat(handle, source, &buf, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ASSERT_TRUE(S_ISREG(buf.st_mode));
}


TEST_F(CopyDoubleCredTest, Simple)
{
    GError *error = NULL;
    int ret;

    ret = gfal2_cred_set(handle, source_root, source_cred, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    ret = gfal2_cred_set(handle, destination_root, dest_cred, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfalt_copy_file(handle, params, source, destination, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
}


int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 3 || (argc > 3 && argc != 5)) {
        printf("Usage: %s source_root destination [proxy-for-source_root proxy-for-destination]\n", argv[0]);
        return 1;
    }

    CopyDoubleCredTest::source_root = argv[1];
    CopyDoubleCredTest::destination_root = argv[2];

    if (argc > 3) {
        CopyDoubleCredTest::source_proxy = argv[3];
        CopyDoubleCredTest::destination_proxy = argv[4];
    }
    else {
        CopyDoubleCredTest::source_proxy = getenv("X509_USER_PROXY");
        CopyDoubleCredTest::destination_proxy =  getenv("X509_USER_PROXY");
    }

    return RUN_ALL_TESTS();
}
