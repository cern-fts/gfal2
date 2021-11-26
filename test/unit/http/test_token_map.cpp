/*
 * Copyright (c) CERN 2021
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


class TokenMapTest: public testing::Test {
public:
    TokenMapTest() {
        GError *error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);

        gfal_plugin_interface* p = gfal_find_plugin(context, "https://", GFAL_PLUGIN_TOKEN, &error);
        Gfal::gerror_to_cpp(&error);
        httpData = static_cast<GfalHttpPluginData*>(gfal_get_plugin_handle(p));
    }

    virtual ~TokenMapTest() {
        gfal2_context_free(context);
    }

    virtual void TearDown() {
        GError *error = NULL;
        int ret = gfal2_cred_clean(context, &error);
        ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    }

protected:
    using OP = GfalHttpPluginData::OP;

    gfal2_context_t context;
    GfalHttpPluginData* httpData;

    void storeInTokenMap(const char* path, const char* token, const OP& operation, bool user_set = false) {
        GError* error = NULL;
        gfal2_cred_t* cred = gfal2_cred_new(GFAL_CRED_BEARER, token);
        gfal2_cred_set(context, path, cred, &error);
        ASSERT_PRED_FORMAT2(AssertGfalSuccess, 0, error);

        if (!user_set) {
            httpData->token_map[std::string(token)] = httpData->writeFlagFromOperation(operation);
        }
    }

    char* findInTokenMap(const char* path, const OP& operation) {
        return httpData->find_se_token(Davix::Uri(path), operation);
    }
};

TEST_F(TokenMapTest, ReadOperation)
{
    const char* path = "davs://example.cern.ch:443/path/subpath/file";

    storeInTokenMap(path, "token_read", OP::READ);
    ASSERT_STREQ(findInTokenMap(path, OP::READ), "token_read");
}

TEST_F(TokenMapTest, WriteOperation)
{
    const char* path = "davs://example.cern.ch:443/path/subpath/file";

    storeInTokenMap(path, "token_read", OP::READ);
    ASSERT_STREQ(findInTokenMap(path, OP::WRITE), nullptr);

    storeInTokenMap(path, "token_write", OP::WRITE);
    ASSERT_STREQ(findInTokenMap(path, OP::WRITE), "token_write");

    // Write token fulfills a read operation
    storeInTokenMap(path, "token_write", OP::WRITE);
    ASSERT_STREQ(findInTokenMap(path, OP::READ), "token_write");
}

TEST_F(TokenMapTest, MkColOperation)
{
    const char* path = "davs://example.cern.ch:443/path/subpath/location";
    const char* reservedpath = "davs://example.cern.ch:443/path/subpath/location/gfal2_mkdir.reserved";

    storeInTokenMap(path, "token_path", OP::MKCOL);
    storeInTokenMap(reservedpath, "token_reservedpath", OP::MKCOL);
    ASSERT_STREQ(findInTokenMap(path, OP::MKCOL), "token_reservedpath");
}

TEST_F(TokenMapTest, MkColSubPath)
{
    const char* path = "davs://example.cern.ch:443/path/subpath/location";
    const char* otherpath = "davs://example.cern.ch:443/path/subpath/file";
    const char* subpath = "davs://example.cern.ch:443/path/subpath/subdir/file";
    const char* enclosingpath = "davs://example.cern.ch:443/path/subpath/location/file";
    const char* enclosingreadpath = "davs://example.cern.ch:443/path/subpath/location/readfile";

    storeInTokenMap(path, "token_path", OP::READ);
    storeInTokenMap(otherpath, "token_otherpath", OP::WRITE);
    ASSERT_STREQ(findInTokenMap(path, OP::MKCOL), nullptr);

    storeInTokenMap(path, "token_path", OP::MKCOL);
    ASSERT_STREQ(findInTokenMap(path, OP::MKCOL), "token_path");

    storeInTokenMap(enclosingreadpath, "token_enclosingreadpath", OP::READ);
    storeInTokenMap(enclosingpath, "token_enclosingpath", OP::WRITE);
    storeInTokenMap(subpath, "token_subpath", OP::WRITE);
    ASSERT_STREQ(findInTokenMap(path, OP::MKCOL), "token_enclosingpath");
}

TEST_F(TokenMapTest, ParentPath)
{
    const char* path = "davs://example.cern.ch:443/path/subpath/location";
    const char* parentpath = "davs://example.cern.ch:443/path/subpath";
    const char* rootpath = "davs://example.cern.ch:443/path";

    storeInTokenMap(path, "token_path", OP::READ);
    storeInTokenMap(parentpath, "token_parentpath", OP::READ);
    storeInTokenMap(rootpath, "token_rootpath", OP::WRITE);
    ASSERT_STREQ(findInTokenMap(path, OP::READ), "token_path");
    ASSERT_STREQ(findInTokenMap(path, OP::WRITE), "token_rootpath");
}

TEST_F(TokenMapTest, ParentPathSlashMatch)
{
    const char* path = "davs://example.cern.ch:443/path/subpath/location";
    const char* siblingpath = "davs://example.cern.ch:443/path/subpath/location_sibling";
    const char* parentpath = "davs://example.cern.ch:443/path/subpath";
    const char* parentpath_sibling = "davs://example.cern.ch:443/path/subpath_sibling";

    const char* other = "davs://example.cern.ch:443/path/other/location";
    const char* parentother = "davs://example.cern.ch:443/path/other/";
    const char* parentother_sibling = "davs://example.cern.ch:443/path/other_sibling/";

    storeInTokenMap(path, "token_path", OP::READ);
    storeInTokenMap(siblingpath, "token_siblingpath", OP::READ);
    storeInTokenMap(parentpath, "token_parentpath", OP::WRITE);
    storeInTokenMap(parentpath_sibling, "token_parentpath_sibling", OP::WRITE);
    ASSERT_STREQ(findInTokenMap(path, OP::WRITE), "token_parentpath");

    storeInTokenMap(parentother, "token_parentother", OP::READ);
    storeInTokenMap(parentother_sibling, "token_parentother_sibling", OP::READ);
    ASSERT_STREQ(findInTokenMap(other, OP::READ), "token_parentother");
}

TEST_F(TokenMapTest, HostCred)
{
    const char* path = "davs://example.cern.ch:443/path/subpath/file";
    const char* host = "example.cern.ch";

    storeInTokenMap(host, "token_host", OP::READ, true);
    ASSERT_STREQ(findInTokenMap(path, OP::WRITE), "token_host");
    ASSERT_STREQ(findInTokenMap(path, OP::READ), "token_host");
}

TEST_F(TokenMapTest, UserSetCred)
{
    const char* path = "davs://example.cern.ch:443/path/subpath/file";
    const char* pathuser = "davs://example.cern.ch:443/path/subpath";

    storeInTokenMap(path, "token_path", OP::READ);
    storeInTokenMap(pathuser, "token_user", OP::READ, true);
    ASSERT_STREQ(findInTokenMap(path, OP::WRITE), "token_user");
    ASSERT_STREQ(findInTokenMap(path, OP::READ), "token_path");
}

TEST_F(TokenMapTest, CopyTest)
{
    const char* source = "davs://source.cern.ch:443/path/subpath/file";
    const char* dest = "davs://destination.cern.ch:443/path/subpath/location/file";

    const char* destparent_reserved = "davs://destination.cern.ch:443/path/subpath/location/gfal2_mkdir.reserved";
    const char* destparent_enoent = "davs://destination.cern.ch:443/path/subpath/location/";
    const char* rootparent = "davs://destination.cern.ch:443/path/subpath";

    // Stat files
    storeInTokenMap(source, "token_source", OP::READ);
    storeInTokenMap(dest, "token_dest_read", OP::READ);
    ASSERT_STREQ(findInTokenMap(source, OP::READ), "token_source");
    ASSERT_STREQ(findInTokenMap(dest, OP::READ), "token_dest_read");

    // Stat destination directory
    ASSERT_STREQ(findInTokenMap(destparent_enoent, OP::HEAD), "token_dest_read");
    ASSERT_STREQ(findInTokenMap(rootparent, OP::HEAD), "token_dest_read");

    // Mkdir destination directory
    ASSERT_STREQ(findInTokenMap(destparent_enoent, OP::MKCOL), nullptr);
    storeInTokenMap(destparent_reserved, "token_destparent_reserved", OP::MKCOL);
    ASSERT_STREQ(findInTokenMap(rootparent, OP::MKCOL), "token_destparent_reserved");
    ASSERT_STREQ(findInTokenMap(destparent_enoent, OP::MKCOL), "token_destparent_reserved");

    // Read source file
    ASSERT_STREQ(findInTokenMap(source, OP::READ), "token_source");

    // Write destination file
    ASSERT_STREQ(findInTokenMap(dest, OP::WRITE), nullptr);
    storeInTokenMap(dest, "token_dest_write", OP::WRITE);
    ASSERT_STREQ(findInTokenMap(dest, OP::WRITE), "token_dest_write");

    // Stat files
    ASSERT_STREQ(findInTokenMap(source, OP::HEAD), "token_source");
    ASSERT_STREQ(findInTokenMap(dest, OP::HEAD), "token_dest_write");
}

TEST_F(TokenMapTest, CopyTestFTSCompatibility)
{
    const char* source = "davs://source.cern.ch:443/path/subpath/file";
    const char* dest = "davs://destination.cern.ch:443/path/subpath/location/file";
    const char* dest_host = "destination.cern.ch";

    const char* destparent_enoent = "davs://destination.cern.ch:443/path/subpath/location/";
    const char* rootparent = "davs://destination.cern.ch:443/path/subpath";

    // Stat files
    storeInTokenMap(source, "token_source", OP::READ, true);
    storeInTokenMap(dest_host, "token_dest_host", OP::WRITE, true);
    ASSERT_STREQ(findInTokenMap(source, OP::READ), "token_source");
    ASSERT_STREQ(findInTokenMap(dest, OP::READ), "token_dest_host");

    // Stat destination directory
    ASSERT_STREQ(findInTokenMap(destparent_enoent, OP::HEAD), "token_dest_host");
    ASSERT_STREQ(findInTokenMap(rootparent, OP::HEAD), "token_dest_host");

    // Mkdir destination directory
    ASSERT_STREQ(findInTokenMap(destparent_enoent, OP::MKCOL), "token_dest_host");
    ASSERT_STREQ(findInTokenMap(rootparent, OP::MKCOL), "token_dest_host");
    ASSERT_STREQ(findInTokenMap(destparent_enoent, OP::MKCOL), "token_dest_host");

    // Read source file
    ASSERT_STREQ(findInTokenMap(source, OP::READ), "token_source");

    // Write destination file
    ASSERT_STREQ(findInTokenMap(dest, OP::WRITE), "token_dest_host");

    // Stat files
    ASSERT_STREQ(findInTokenMap(source, OP::HEAD), "token_source");
    ASSERT_STREQ(findInTokenMap(dest, OP::HEAD), "token_dest_host");
}
