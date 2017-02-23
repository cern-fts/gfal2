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

#include <gfal_api.h>
#include <gtest/gtest.h>
#include <common/gfal_gtest_asserts.h>
#include <utils/exceptions/gerror_to_cpp.h>


class ConfigFixture: public testing::Test {
protected:
    gfal2_context_t context;

public:
    ConfigFixture() {
        GError* error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
    }

    ~ConfigFixture() {
        gfal2_context_free(context);
    }
};

// Regression test for DMC-833
TEST_F(ConfigFixture, ClientData)
{
    GError *error = NULL;
    int ret = 0;

    ret = gfal2_add_client_info(context, "TEST", "VALUE", &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfal2_add_client_info(context, "TEST2", "VALUE2", &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfal2_add_client_info(context, "TEST", "REPLACED", &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfal2_get_client_info_count(context, &error);
    EXPECT_EQ(ret, 2);

    const char *value;
    ret = gfal2_get_client_info_value(context, "TEST", &value, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    EXPECT_STRCASEEQ(value, "REPLACED");
}


TEST_F(ConfigFixture, String)
{
    GError *error = NULL;
    int ret = 0;

    ret = gfal2_set_opt_string(context, "GROUP1", "KEY2", "MYVALUE3", &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    gchar *value = gfal2_get_opt_string(context, "GROUP1", "KEY2", &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    EXPECT_EQ(0, strncmp(value, "MYVALUE3", 8));
    g_free(value);
}


TEST_F(ConfigFixture, Integer)
{
    GError *error = NULL;
    int ret = 0;

    ret = gfal2_set_opt_integer(context, "GROUP1", "KEY2", 43215, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    int value = gfal2_get_opt_integer(context, "GROUP1", "KEY2", &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    EXPECT_EQ(43215, value);
}
