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
#include <common/gfal_gtest_asserts.h>


class StatTest: public testing::Test {
public:
	static const char* root;

	gfal2_context_t context;

	StatTest() {
		GError *error = NULL;
		context = gfal2_context_new(&error);
		Gfal::gerror_to_cpp(&error);
	}

	virtual ~StatTest() {
		gfal2_context_free(context);
	}
};
const char* StatTest::root;


TEST_F(StatTest, SimpleStat)
{
    GError *error = NULL;
    struct stat statbuf;
    int ret;

    ret = gfal2_stat(context, root, &statbuf, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    std::cout << "stat successful" << std::endl;
    std::cout << "mode = " << std::oct << statbuf.st_mode << std::dec << std::endl;
    std::cout << "nlink = " << statbuf.st_nlink << std::endl;
    std::cout << "uid = " << statbuf.st_uid << std::endl;
    std::cout << "gid = " << statbuf.st_gid << std::endl;
    std::cout << "size = " << statbuf.st_size << std::endl;
}


int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 1) {
        printf("Missing base url\n");
        printf("\t%s [options] srm://host/base/path/\n", argv[0]);
        return 1;
    }

    StatTest::root = argv[1];

    return RUN_ALL_TESTS();
}
