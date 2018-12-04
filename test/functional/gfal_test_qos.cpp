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
    static const char* file;

    gfal2_context_t context;
    gfalt_params_t params;

    QosTest() {
        GError *error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
    }

    virtual ~QosTest() {
        gfal2_context_free(context);
    }

    virtual void SetUp() {
    	unsetenv("X509_USER_PROXY");
    	unsetenv("X509_USER_CERT");
    	unsetenv("X509_USER_KEY");

    	int ret;
    	GError *error = NULL;

    	Gfal::gerror_to_cpp(&error);
    	params = gfalt_params_handle_new(&error);
    	Gfal::gerror_to_cpp(&error);
    	// Clear automatic setup
    	gfal2_remove_opt(context, "X509", "CERT", NULL);
    	gfal2_remove_opt(context, "X509", "KEY", NULL);
    	// Set configured values
    }

    virtual void TearDown() {
        GError *error = NULL;
    }
};

const char *QosTest::root = NULL;
const char *QosTest::file = NULL;


TEST_F(QosTest, TestQosClasses)
{
	GError *err = NULL;
	const char* result = gfal2_qos_check_classes(context, root, "dataobject", &err);
	if (result != NULL) {
		std::string str(result);
		std::cout << str << std::endl;
	}
	EXPECT_EQ(NULL, err);
}

TEST_F(QosTest, TestCheckFileQos)
{
	GError *err = NULL;
        std::string url = std::string(root) + std::string(file);
	const char* result = gfal2_check_file_qos(context,url.c_str(), &err);
	if (result != NULL) {
		std::string str(result);
		std::cout << str << std::endl;
	}
	EXPECT_EQ(NULL, err);
}

TEST_F(QosTest, TestCheckQoSTransitions)
{
	GError *err = NULL;
        std::string url = std::string(root) + std::string("/cdmi_capabilities/dataobject/disk");
	const char* result = gfal2_check_available_qos_transitions(context, url.c_str(), &err);
	if (result != NULL) {
		std::string str(result);
		std::cout << str << std::endl;
	}
	EXPECT_EQ(NULL, err);
}

TEST_F(QosTest, TestCheckTargetQoSOfFile)
{
	GError *err = NULL;
         std::string url = std::string(root) + std::string(file);
	const char* result = gfal2_check_target_qos(context, url.c_str(), &err);
	if (result != NULL) {
		std::string str(result);
		std::cout << str << std::endl;
	}
	EXPECT_EQ(NULL, err);
}

TEST_F(QosTest, TestChangeQosOfFile)
{
	GError *err = NULL;
        std::string url = std::string(root) + std::string(file);
	int result = gfal2_change_object_qos(context, url.c_str(), "/cdmi_capabilities/dataobject/tape/", &err);
	std::cout << result << std::endl;
	EXPECT_EQ(NULL, err);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 3) {
        printf("Missing parameters \n");
        printf("\t%s [host] [file]\n", argv[0]);
        return -1;
    }
    QosTest::root = argv[1];
    QosTest::file = argv[2];
    return RUN_ALL_TESTS();
}
