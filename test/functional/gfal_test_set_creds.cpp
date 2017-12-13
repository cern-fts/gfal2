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

/**
 * Regression test for DMC-529
 * Set the credentials via configuration, and make sure that it does work!
 */
#include <gtest/gtest.h>
#include <gfal_api.h>
#include <utils/exceptions/gerror_to_cpp.h>
#include <common/gfal_gtest_asserts.h>


class CredsTest: public testing::Test {
public:
    static const char* root;
    static const char* proxy;

    gfal2_context_t context;

    CredsTest() {
        GError *error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
    }

    virtual ~CredsTest() {
        gfal2_context_free(context);
    }
};
const char* CredsTest::root;
const char* CredsTest::proxy;


TEST_F(CredsTest, SetCreds)
{
    gfal2_set_opt_string(context, "X509", "CERT", proxy, NULL);
    gfal2_set_opt_string(context, "X509", "KEY", proxy, NULL);

    GError *error = NULL;
    struct stat statbuf;
    int ret;

    ret = gfal2_stat(context, root, &statbuf, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    if (ret == 0) {
        std::cout << "stat successful" << std::endl;
        std::cout << "mode = " << std::oct << statbuf.st_mode << std::dec << std::endl;
        std::cout << "nlink = " << statbuf.st_nlink << std::endl;
        std::cout << "uid = " << statbuf.st_uid << std::endl;
        std::cout << "gid = " << statbuf.st_gid << std::endl;
        std::cout << "size = " << statbuf.st_size << std::endl;
    }
}


int main(int argc, char **argv)
{
    if (getenv("X509_USER_CERT") ||
        getenv("X509_USER_KEY") ||
        getenv("X509_USER_PROXY")) {
        std::cerr << "Unset X509_USER_* environment variables before calling" << std::endl;
        return 1;
    }

    testing::InitGoogleTest(&argc, argv);

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " proxy surl" << std::endl;
        return 1;
    }

    CredsTest::proxy = argv[1];
    CredsTest::root = argv[2];
    
    return RUN_ALL_TESTS();
}
