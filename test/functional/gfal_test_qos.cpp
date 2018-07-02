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
#include <utils/exceptions/gerror_to_cpp.h>

#define BLKLEN 65536


class QosTest: public testing::Test {
public:
    static const char* root;

    gfal2_context_t context;

    QosTest() {
        GError *error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
    }

    virtual ~QosTest() {
        gfal2_context_free(context);
    }

    virtual void SetUp() {
        GError* error = NULL;
    }

    virtual void TearDown() {
        GError *error = NULL;
    }
};


TEST_F(QosTest, TestSimpleCase)
{
	GError *err = NULL;
	gfal2_qos_check_classes(context, "https://dcache-xdc.desy.de:6443", "dataobject", &err);
	EXPECT_EQ(NULL, err);
}

const char* QosTest::root;

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    /*if (argc < 2) {
        printf("Missing base url\n");
        printf("\t%s [options] srm://host/base/path/\n", argv[0]);
        return 1;
    }*/

    /*QosTest::root = argv[1];*/

    return RUN_ALL_TESTS();
}
