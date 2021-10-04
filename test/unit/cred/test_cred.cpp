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

#include <gfal_api.h>
#include <gtest/gtest.h>
#include "common/gfal_gtest_asserts.h"

class CredTest: public testing::Test {
protected:
    gfal2_context_t context;
    gfal2_cred_t *x509, *user, *x509_2;
    gfal2_cred_t *token, *token_2;

public:
    CredTest() {
        x509 = gfal2_cred_new(GFAL_CRED_X509_CERT, "ABCDE");
        x509_2 = gfal2_cred_new(GFAL_CRED_X509_CERT, "12345");
        user = gfal2_cred_new(GFAL_CRED_USER, "user");
        token = gfal2_cred_new(GFAL_CRED_BEARER, "mytoken");
        token_2 = gfal2_cred_new(GFAL_CRED_BEARER, "mytoken_2");
    }

    virtual ~CredTest() {
        gfal2_cred_free(x509);
        gfal2_cred_free(x509_2);
        gfal2_cred_free(user);
        gfal2_cred_free(token);
        gfal2_cred_free(token_2);
    }

    virtual void SetUp() {
        GError *error = NULL;
        context = gfal2_context_new(&error);
        ASSERT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
        int ret = gfal2_cred_clean(context, &error);
        ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    }

    virtual void TearDown() {
        gfal2_context_free(context);
    }
};


TEST_F(CredTest, dup)
{
    gfal2_cred_t *copy = gfal2_cred_dup(x509);
    ASSERT_NE(copy, (void*)NULL);
    ASSERT_NE(copy->type, x509->type);
    ASSERT_NE(copy->value, x509->value);
    ASSERT_STREQ(copy->type, x509->type);
    ASSERT_STREQ(copy->value, x509->value);
    gfal2_cred_free(copy);
}


TEST_F(CredTest, set_get)
{
    const char *original_baseurl = "gsiftp://host.com/path";

    GError *error = NULL;
    int ret = gfal2_cred_set(context, original_baseurl, x509, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    const char *baseurl = NULL;
    char *resp = gfal2_cred_get(context, GFAL_CRED_X509_CERT, "gsiftp://host.com/path/subdir/file", &baseurl, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    ASSERT_NE(resp, (void*)NULL);
    ASSERT_STREQ(resp, x509->value);
    ASSERT_STREQ(original_baseurl, baseurl);
    g_free(resp);
}


TEST_F(CredTest, compat)
{
    GError *error = NULL;
    int ret;

    ret = gfal2_set_opt_string(context, "X509", "CERT", "/path/to/my/cert", &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    ret = gfal2_set_opt_string(context, "X509", "KEY", "/path/to/my/key", &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    const char *baseurl = NULL;
    char *resp = gfal2_cred_get(context, GFAL_CRED_X509_CERT, "gsiftp://host.com/path/subdir/file", &baseurl, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    ASSERT_NE(resp, (void*)NULL);
    ASSERT_STREQ(resp, "/path/to/my/cert");
    ASSERT_STREQ("", baseurl);
    g_free(resp);

    resp = gfal2_cred_get(context, GFAL_CRED_X509_KEY, "gsiftp://host.com/path/subdir/file", &baseurl, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    ASSERT_NE(resp, (void*)NULL);
    ASSERT_STREQ(resp, "/path/to/my/key");
    ASSERT_STREQ("", baseurl);
    g_free(resp);
}


TEST_F(CredTest, set_get_longer_prefix)
{
    const char *short_base = "gsiftp://host.com/path";
    const char *long_base = "gsiftp://host.com/path/subdir";

    GError *error = NULL;
    int ret = gfal2_cred_set(context, short_base, x509, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfal2_cred_set(context, long_base, user, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfal2_cred_set(context, long_base, x509_2, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    const char *baseurl = NULL;
    char *resp = gfal2_cred_get(context, GFAL_CRED_X509_CERT, "gsiftp://host.com/path/subdir/file", &baseurl, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    ASSERT_NE(resp, (void*)NULL);
    ASSERT_STREQ(resp, x509_2->value);
    ASSERT_STREQ(long_base, baseurl);
    g_free(resp);
}


static void callback(const char *url_prefix, const gfal2_cred_t *cred, void *user_data)
{
    char **value = (char**)user_data;
    *value = cred->value;
}


TEST_F(CredTest, foreach)
{
    GError *error = NULL;
    int ret = gfal2_cred_set(context, "gsiftp://host.com/path", x509, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    char *value;
    gfal2_cred_foreach(context, callback, &value);
    ASSERT_STREQ(value, x509->value);
}


TEST_F(CredTest, copy)
{
    GError *error = NULL;
    int ret = gfal2_cred_set(context, "gsiftp://host.com/path", x509, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    gfal2_context_t new_context = gfal2_context_new(&error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, 0, error);

    ret = gfal2_cred_copy(new_context, context, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    const char *baseurl = NULL;
    char *resp = gfal2_cred_get(new_context, GFAL_CRED_X509_CERT, "gsiftp://host.com/path/subdir/file", &baseurl, &error);
    ASSERT_NE(resp, (void*)NULL);
    ASSERT_STREQ(resp, x509->value);
    g_free(resp);

    gfal2_context_free(new_context);
}

TEST_F(CredTest, del)
{
    const char* short_base = "https://host.com/path";
    const char* long_base = "https://host.com/path/subpath";
    const char* full_path = "https://host.com/path/subpath/file";
    GError* error = NULL;

    int ret = gfal2_cred_set(context, short_base, token, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfal2_cred_set(context, long_base, token_2, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    const char* baseurl = NULL;
    char* resp = gfal2_cred_get(context, GFAL_CRED_BEARER, full_path, &baseurl, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    ASSERT_NE(resp, (void*) NULL);
    ASSERT_STREQ(resp, token_2->value);
    ASSERT_STREQ(long_base, baseurl);
    g_free(resp);

    ret = gfal2_cred_del(context, GFAL_CRED_BEARER, long_base, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    resp = gfal2_cred_get(context, GFAL_CRED_BEARER, full_path, &baseurl, &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    ASSERT_NE(resp, (void*) NULL);
    ASSERT_STREQ(resp, token->value);
    ASSERT_STREQ(short_base, baseurl);
    g_free(resp);

    ret = gfal2_cred_del(context, GFAL_CRED_BEARER, long_base, &error);
    ASSERT_EQ(ret, -1);

    ret = gfal2_cred_del(context, GFAL_CRED_BEARER, full_path, &error);
    ASSERT_EQ(ret, -1);
}
