/*
 * Copyright (c) CERN 2022
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

#define __GFAL2_H_INSIDE__
#include <common/gfal_plugin.h>
#undef __GFAL2_H_INSIDE__

#include <davix.hpp>
#include "plugins/http/gfal_http_plugin.h"

class CustomHttpOptionsTest: public testing::Test {
public:
    CustomHttpOptionsTest() = default;
    ~CustomHttpOptionsTest() = default;

    virtual void SetUp() {
        GError* error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
        storeConfig("HTTP PLUGIN", "ENABLE_STREAM_COPY", true);
        storeConfig("HTTP PLUGIN", "ENABLE_FALLBACK_TPC_COPY", true);
    }

    virtual void TearDown() {
        gfal2_context_free(context);
        context = NULL;
    }

protected:
    gfal2_context_t context = NULL;
    const char* src = "https://eospublic.cern.ch:443/path/file.src";
    const char* dst = "https://eospps.cern.ch:443/path/file.dst";

    void storeConfig(const std::string& group, const char* key, bool value) {
        GError* error = NULL;
        gfal2_set_opt_boolean(context, group.c_str(), key, value, &error);
        ASSERT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    }

    std::string surlToConfigGroup(const char* surl) {
        Davix::Uri uri(surl);
        std::string prot = uri.getProtocol();

        if (prot.back() == 's') {
            prot.pop_back();
        }

        std::string group = prot + ":" + uri.getHost();
        std::transform(group.begin(), group.end(), group.begin(), ::toupper);
        return group;
    }
};

TEST_F(CustomHttpOptionsTest, GenericEnabled)
{
    ASSERT_TRUE(is_http_streaming_enabled(context, src, dst));

    ASSERT_TRUE(is_http_3rdcopy_fallback_enabled(context, src, dst));
}

TEST_F(CustomHttpOptionsTest, GenericDisabled)
{
    storeConfig("HTTP PLUGIN", "ENABLE_STREAM_COPY", false);
    ASSERT_FALSE(is_http_streaming_enabled(context, src, dst));

    storeConfig("HTTP PLUGIN", "ENABLE_FALLBACK_TPC_COPY", false);
    ASSERT_FALSE(is_http_3rdcopy_fallback_enabled(context, src, dst));
}

TEST_F(CustomHttpOptionsTest, SrcDisabled)
{
    storeConfig(surlToConfigGroup(src), "ENABLE_STREAM_COPY", false);
    ASSERT_FALSE(is_http_streaming_enabled(context, src, dst));

    storeConfig(surlToConfigGroup(src), "ENABLE_FALLBACK_TPC_COPY", false);
    ASSERT_FALSE(is_http_3rdcopy_fallback_enabled(context, src, dst));
}

TEST_F(CustomHttpOptionsTest, DstDisabled)
{
    storeConfig(surlToConfigGroup(dst), "ENABLE_STREAM_COPY", false);
    ASSERT_FALSE(is_http_streaming_enabled(context, src, dst));

    storeConfig(surlToConfigGroup(dst), "ENABLE_FALLBACK_TPC_COPY", false);
    ASSERT_FALSE(is_http_3rdcopy_fallback_enabled(context, src, dst));
}

TEST_F(CustomHttpOptionsTest, SrcEnabled)
{
    storeConfig(surlToConfigGroup(src), "ENABLE_STREAM_COPY", true);
    ASSERT_TRUE(is_http_streaming_enabled(context, src, dst));

    storeConfig(surlToConfigGroup(src), "ENABLE_FALLBACK_TPC_COPY", true);
    ASSERT_TRUE(is_http_3rdcopy_fallback_enabled(context, src, dst));
}

TEST_F(CustomHttpOptionsTest, DstEnabled)
{
    storeConfig(surlToConfigGroup(dst), "ENABLE_STREAM_COPY", true);
    ASSERT_TRUE(is_http_streaming_enabled(context, src, dst));

    storeConfig(surlToConfigGroup(dst), "ENABLE_FALLBACK_TPC_COPY", true);
    ASSERT_TRUE(is_http_3rdcopy_fallback_enabled(context, src, dst));
}

TEST_F(CustomHttpOptionsTest, SrcAndDstDisabled)
{
    storeConfig(surlToConfigGroup(src), "ENABLE_STREAM_COPY", false);
    storeConfig(surlToConfigGroup(dst), "ENABLE_STREAM_COPY", false);
    ASSERT_FALSE(is_http_streaming_enabled(context, src, dst));

    storeConfig(surlToConfigGroup(src), "ENABLE_FALLBACK_TPC_COPY", false);
    storeConfig(surlToConfigGroup(dst), "ENABLE_FALLBACK_TPC_COPY", false);
    ASSERT_FALSE(is_http_3rdcopy_fallback_enabled(context, src, dst));
}

TEST_F(CustomHttpOptionsTest, SrcEnabledAndDstDisabled)
{
    storeConfig(surlToConfigGroup(src), "ENABLE_STREAM_COPY", true);
    storeConfig(surlToConfigGroup(dst), "ENABLE_STREAM_COPY", false);
    ASSERT_FALSE(is_http_streaming_enabled(context, src, dst));

    storeConfig(surlToConfigGroup(src), "ENABLE_FALLBACK_TPC_COPY", true);
    storeConfig(surlToConfigGroup(dst), "ENABLE_FALLBACK_TPC_COPY", false);
    ASSERT_FALSE(is_http_3rdcopy_fallback_enabled(context, src, dst));
}

TEST_F(CustomHttpOptionsTest, SrcDisabledAndDstEnabled)
{
    storeConfig(surlToConfigGroup(src), "ENABLE_STREAM_COPY", false);
    storeConfig(surlToConfigGroup(dst), "ENABLE_STREAM_COPY", true);
    ASSERT_FALSE(is_http_streaming_enabled(context, src, dst));

    storeConfig(surlToConfigGroup(src), "ENABLE_FALLBACK_TPC_COPY", false);
    storeConfig(surlToConfigGroup(dst), "ENABLE_FALLBACK_TPC_COPY", true);
    ASSERT_FALSE(is_http_3rdcopy_fallback_enabled(context, src, dst));
}

TEST_F(CustomHttpOptionsTest, SrcAndDstEnabled)
{
    storeConfig(surlToConfigGroup(src), "ENABLE_STREAM_COPY", true);
    storeConfig(surlToConfigGroup(dst), "ENABLE_STREAM_COPY", true);
    ASSERT_TRUE(is_http_streaming_enabled(context, src, dst));

    storeConfig(surlToConfigGroup(src), "ENABLE_FALLBACK_TPC_COPY", true);
    storeConfig(surlToConfigGroup(dst), "ENABLE_FALLBACK_TPC_COPY", true);
    ASSERT_TRUE(is_http_3rdcopy_fallback_enabled(context, src, dst));
}
