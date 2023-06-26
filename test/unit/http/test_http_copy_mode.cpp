/*
 * Copyright (c) CERN 2023
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

using CopyMode = HttpCopyMode::CopyMode;

class HttpCopyModeTest: public testing::Test {
public:
    HttpCopyModeTest() = default;
    ~HttpCopyModeTest() = default;

    virtual void SetUp() {
        GError* error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
        storeConfig("HTTP PLUGIN", "ENABLE_REMOTE_COPY", TRUE);
        storeConfig("HTTP PLUGIN", "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_PULL);
    }

    virtual void TearDown() {
        gfal2_context_free(context);
        context = NULL;
    }

protected:
    gfal2_context_t context = NULL;
    const char* src = "https://eospublic.cern.ch:443/path/file.src";
    const char* dst = "https://eospps.cern.ch:443/path/file.dst";

    void storeConfig(const std::string& group, const char* key, int value) {
        GError* error = NULL;
        gfal2_set_opt_boolean(context, group.c_str(), key, value, &error);
        ASSERT_PRED_FORMAT2(AssertGfalSuccess, 0, error);
    }

    void storeConfig(const std::string& group, const char* key, const char* value) {
        GError* error = NULL;
        gfal2_set_opt_string(context, group.c_str(), key, value, &error);
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

TEST_F(HttpCopyModeTest, Default)
{
    auto copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::PULL, copyMode.value());
}

TEST_F(HttpCopyModeTest, DefaultConfigOptions)
{
    storeConfig("HTTP PLUGIN", "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_PULL);
    auto copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::PULL, copyMode.value());

    storeConfig("HTTP PLUGIN", "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_PUSH);
    copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::PUSH, copyMode.value());

    storeConfig("HTTP PLUGIN", "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_STREAMED);
    copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::STREAM, copyMode.value());
    ASSERT_TRUE(copyMode.isStreamingOnly());

    storeConfig("HTTP PLUGIN", "DEFAULT_COPY_MODE", "bad-value");
    copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::PULL, copyMode.value());
}

TEST_F(HttpCopyModeTest, TPCDisabled)
{
    storeConfig("HTTP PLUGIN", "ENABLE_REMOTE_COPY", FALSE);
    auto copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::STREAM, copyMode.value());
    ASSERT_TRUE(copyMode.isStreamingOnly());
}

TEST_F(HttpCopyModeTest, StreamingDisabled)
{
    storeConfig("HTTP PLUGIN", "ENABLE_STREAM_COPY", FALSE);
    auto copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::PULL, copyMode.value());
    ASSERT_FALSE(copyMode.isStreamingOnly());
}

TEST_F(HttpCopyModeTest, TPCIncompatible)
{
    src = "root://eospublic.cern.ch:1094//path/file.src";
    auto copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::STREAM, copyMode.value());
    ASSERT_TRUE(copyMode.isStreamingOnly());
}

TEST_F(HttpCopyModeTest, SESpecificSource)
{
    storeConfig(surlToConfigGroup(src), "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_PUSH);
    auto copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::PUSH, copyMode.value());

    storeConfig(surlToConfigGroup(src), "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_STREAMED);
    copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::STREAM, copyMode.value());
}

TEST_F(HttpCopyModeTest, SESpecificDest)
{
    storeConfig(surlToConfigGroup(dst), "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_PUSH);
    auto copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::PUSH, copyMode.value());

    storeConfig(surlToConfigGroup(dst), "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_STREAMED);
    copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::STREAM, copyMode.value());
}

TEST_F(HttpCopyModeTest, SESpecificBoth)
{
    storeConfig(surlToConfigGroup(src), "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_PUSH);
    storeConfig(surlToConfigGroup(dst), "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_STREAMED);
    auto copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::PUSH, copyMode.value());

    storeConfig(surlToConfigGroup(src), "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_PULL);
    storeConfig(surlToConfigGroup(dst), "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_PUSH);
    copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::PULL, copyMode.value());

    storeConfig(surlToConfigGroup(src), "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_STREAMED);
    storeConfig(surlToConfigGroup(dst), "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_PULL);
    copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::STREAM, copyMode.value());
}

TEST_F(HttpCopyModeTest, TPCIncompatible_and_SESpecific)
{
    src = "root://eospublic.cern.ch:1094//path/file.src";
    storeConfig(surlToConfigGroup(src), "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_PULL);
    storeConfig(surlToConfigGroup(dst), "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_PUSH);
    auto copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::STREAM, copyMode.value());
    ASSERT_TRUE(copyMode.isStreamingOnly());
}

TEST_F(HttpCopyModeTest, QueryStringSource)
{
    src = "https://eospublic.cern.ch:443/path/file.src?key=value&copy_mode=push";
    storeConfig(surlToConfigGroup(src), "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_PULL);
    auto copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::PUSH, copyMode.value());
}

TEST_F(HttpCopyModeTest, QueryStringDest)
{
    dst = "https://eospps.cern.ch:443/path/file.dst?key=value&copy_mode=push";
    storeConfig(surlToConfigGroup(dst), "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_PULL);
    auto copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::PUSH, copyMode.value());
}

TEST_F(HttpCopyModeTest, QueryStringBoth)
{
    src = "https://eospublic.cern.ch:443/path/file.src?key=value&copy_mode=push";
    dst = "https://eospps.cern.ch:443/path/file.dst?key=value&copy_mode=push";
    storeConfig(surlToConfigGroup(src), "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_PULL);
    storeConfig(surlToConfigGroup(dst), "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_PULL);
    auto copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::PUSH, copyMode.value());
}

TEST_F(HttpCopyModeTest, QueryStringInvalid)
{
    src = "https://eospublic.cern.ch:443/path/file.src?key=value&copy_mode=stream";
    dst = "https://eospps.cern.ch:443/path/file.dst?key=value&copy_mode=stream";
    auto copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::PULL, copyMode.value());

    src = "https://eospublic.cern.ch:443/path/file.src?copy_mode=invalid";
    dst = "https://eospps.cern.ch:443/path/file.dst?copy_mode=invalid";
    copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::PULL, copyMode.value());
}

TEST_F(HttpCopyModeTest, QueryString_and_SESpecific)
{
    src = "https://eospublic.cern.ch:443/path/file.src?key=value&copy_mode=push";
    dst = "https://eospps.cern.ch:443/path/file.dst?key=value&copy_mode=pull";
    storeConfig(surlToConfigGroup(src), "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_PULL);
    storeConfig(surlToConfigGroup(dst), "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_PULL);
    auto copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::PUSH, copyMode.value());
}

TEST_F(HttpCopyModeTest, TPCDisabled_and_QueryString)
{
    storeConfig("HTTP PLUGIN", "ENABLE_REMOTE_COPY", FALSE);
    src = "https://eospublic.cern.ch:443/path/file.src?copy_mode=push";
    dst = "https://eospps.cern.ch:443/path/file.dst?copy_mode=push";
    auto copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::STREAM, copyMode.value());
    ASSERT_TRUE(copyMode.isStreamingOnly());
}

TEST_F(HttpCopyModeTest, CopyModeToStr)
{
    ASSERT_STREQ(HttpCopyMode::CopyModeToStr(CopyMode::PULL), GFAL_TRANSFER_TYPE_PULL);
    ASSERT_STREQ(HttpCopyMode::CopyModeToStr(CopyMode::PUSH), GFAL_TRANSFER_TYPE_PUSH);
    ASSERT_STREQ(HttpCopyMode::CopyModeToStr(CopyMode::STREAM), GFAL_TRANSFER_TYPE_STREAMED);
    ASSERT_STREQ(HttpCopyMode::CopyModeToStr(CopyMode::NONE), "None");
}

TEST_F(HttpCopyModeTest, CopyModeIteration)
{
    auto copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::PULL, copyMode.value());
    ASSERT_FALSE(copyMode.end());

    copyMode.next();
    ASSERT_EQ(CopyMode::PUSH, copyMode.value());
    ASSERT_FALSE(copyMode.end());

    copyMode.next();
    ASSERT_EQ(CopyMode::STREAM, copyMode.value());
    ASSERT_FALSE(copyMode.end());

    copyMode.next();
    ASSERT_EQ(CopyMode::NONE, copyMode.value());
    ASSERT_TRUE(copyMode.end());

    copyMode.next();
    ASSERT_EQ(CopyMode::NONE, copyMode.value());
    ASSERT_TRUE(copyMode.end());
}

TEST_F(HttpCopyModeTest, CopyModeIteration_StreamingDisabled)
{
    storeConfig("HTTP PLUGIN", "ENABLE_STREAM_COPY", FALSE);
    dst = "https://eospps.cern.ch:443/path/file.dst?key=value&copy_mode=push";
    auto copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::PUSH, copyMode.value());
    ASSERT_FALSE(copyMode.isStreamingEnabled());
    ASSERT_FALSE(copyMode.end());

    copyMode.next();
    ASSERT_EQ(CopyMode::NONE, copyMode.value());
    ASSERT_TRUE(copyMode.end());
}

TEST_F(HttpCopyModeTest, CopyModeLoopIteration)
{
    auto copyMode = HttpCopyMode::ConstructCopyMode(context, src, dst);
    ASSERT_EQ(CopyMode::PULL, copyMode.value());

    do {
        copyMode.next();
    } while (!copyMode.end());
    ASSERT_TRUE(copyMode.end());
}
