/*
 * Copyright (c) CERN 2013-2021
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

class TokenTest: public testing::Test {

public:
    static const char* url;
    static const char* issuer;

    gfal2_context_t context;

    TokenTest() {
        GError* error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
    }

    virtual ~TokenTest() {
        gfal2_context_free(context);
    }
};

const char* TokenTest::url = NULL;
const char* TokenTest::issuer = NULL;

TEST_F(TokenTest, TestRetrieveNoIssuer)
{
    char buff[2048];
    GError* err = NULL;
    ssize_t result = gfal2_token_retrieve(context, url, "", true, 60, NULL, buff, 2048, &err);

    EXPECT_GT(result, 0);
    EXPECT_EQ(NULL, err);
}

TEST_F(TokenTest, TestRetrieveIssuer)
{
    char buff[2048];
    GError* err = NULL;
    ssize_t result = gfal2_token_retrieve(context, url, issuer, true, 60, NULL, buff, 2048, &err);

    EXPECT_GT(result, 0);
    EXPECT_EQ(NULL, err);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 3) {
        printf("Missing parameters: \n");
        printf("\t%s [url] [issuer]\n", argv[0]);
        return -1;
    }

    TokenTest::url = argv[1];
    TokenTest::issuer = argv[2];

    return RUN_ALL_TESTS();
}
