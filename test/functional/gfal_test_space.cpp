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
#include <utils/exceptions/gerror_to_cpp.h>
#include <common/gfal_gtest_asserts.h>
#include <json.h>


class SpaceTest: public testing::Test {
public:
    static const char* root;
    gfal2_context_t context;
    static enum ResponseType {
        kUnsupported,
        kSpaceTokenArray,
        kSpaceInfo,
    } responseType;
    static char responseBuffer[1024];

    SpaceTest() {
        GError *error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
    }

    virtual ~SpaceTest() {
        gfal2_context_free(context);
    }
};

const char* SpaceTest::root;
SpaceTest::ResponseType SpaceTest::responseType;
char SpaceTest::responseBuffer[1024];


TEST_F(SpaceTest, SpaceInAttrList)
{
    GError *error = NULL;
    char attrs[1024];

    int ret = gfal2_listxattr(context, root, attrs, sizeof(attrs), &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    for (int i = 0; i < ret;) {
        GTEST_LOG_(INFO) << (attrs + i);
        size_t attrlen = strlen(attrs + i);
        if (strcmp(attrs + i, GFAL_XATTR_SPACETOKEN) == 0) {
            SUCCEED();
            return;
        }
        i += attrlen + 1;
    }
    // Raise an error if we reach here
    FAIL();
}


TEST_F(SpaceTest, GetSpaceAttr)
{
    GError *error = NULL;

    int ret = gfal2_getxattr(context, root, GFAL_XATTR_SPACETOKEN, responseBuffer, sizeof(responseBuffer), &error);
    if (ret < 0 && (error->code == ENOSYS || error->code == ECOMM)) {
        GTEST_LOG_(INFO) << "Not supported by the storage: " << error->message;
        g_error_free(error);
        responseType = kUnsupported;
        SKIP_TEST(GetSpaceAttr);
        return;
    }
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    // Now, there are two possible responses: either an array with spacetoken names (SRM),
    // or just space info (xrootd)
    if (responseBuffer[0] == '[') {
        responseType = kSpaceTokenArray;
    }
    else if (responseBuffer[0] == '{') {
        responseType = kSpaceInfo;
    }
}


static void _validate_space_info(const char *response)
{
    json_object *info = json_tokener_parse(response);
    ASSERT_NE(info, (void*)NULL);
    ASSERT_TRUE(json_object_is_type(info, json_type_object));

    json_object *totalsizeObj, *unusedObj, *usedObj;
    ASSERT_TRUE(json_object_object_get_ex(info, "totalsize", &totalsizeObj));
    ASSERT_TRUE(json_object_object_get_ex(info, "unusedsize", &unusedObj));
    ASSERT_TRUE(json_object_object_get_ex(info, "usedsize", &usedObj));

    errno = 0;
    int64_t total = json_object_get_int64(totalsizeObj);
    ASSERT_EQ(errno, 0);

    errno = 0;
    int64_t unused = json_object_get_int64(unusedObj);
    ASSERT_EQ(errno, 0);

    errno = 0;
    int64_t used = json_object_get_int64(usedObj);
    ASSERT_EQ(errno, 0);

    GTEST_LOG_(INFO) << used << "/" << total << "(" << unused << ")";
    json_object_put(info);
}


TEST_F(SpaceTest, SpaceTokenList)
{
    GError *error = NULL;
    char buffer[1024];

    if (responseType != kSpaceTokenArray) {
        SKIP_TEST(SpaceTokenList);
        return;
    }

    json_object *obj = json_tokener_parse(responseBuffer);
    ASSERT_NE(obj, (void*)NULL);
    ASSERT_TRUE(json_object_is_type(obj, json_type_array));

    // Iterate and ask for the info of each token
    int arrayLen = json_object_array_length(obj);
    for (int i = 0; i < arrayLen; ++i) {
        json_object *tokenObj = json_object_array_get_idx(obj, i);
        ASSERT_TRUE(json_object_is_type(tokenObj, json_type_string));
        const char *token = json_object_get_string(tokenObj);

        char *attrName = g_strconcat(GFAL_XATTR_SPACETOKEN, ".token?", token, NULL);
        GTEST_LOG_(INFO) << attrName;

        int ret = gfal2_getxattr(context, root, attrName, buffer, sizeof(buffer), &error);
        g_free(attrName);
        ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
        _validate_space_info(buffer);
    }

    json_object_put(obj);
}


TEST_F(SpaceTest, SpaceInfo)
{
    if (responseType != kSpaceInfo) {
        SKIP_TEST(SpaceTokenList);
        return;
    }
    _validate_space_info(responseBuffer);
}


int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 2) {
        printf("Missing base url\n");
        printf("\t%s srm://host/base/path/\n", argv[0]);
        return 1;
    }

    SpaceTest::root = argv[1];

    return RUN_ALL_TESTS();
}
