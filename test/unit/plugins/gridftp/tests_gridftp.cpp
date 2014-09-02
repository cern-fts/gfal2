/*
* Copyright @ Members of the EMI Collaboration, 2010.
* See www.eu-emi.eu for details on the copyright holders.
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


#include "tests_gridftp.h"
#include <common/gfal_common_internal.h>
#include <plugins/gridftp/gridftp_plugin.h>
#include <plugins/gridftp/gridftpmodule.h>
#include <plugins/gridftp/gridftpwrapper.h>
#include <gtest/gtest.h>



gfal2_context_t init_gfal_handle()
{
    GError * tmp_err = NULL;
    gfal2_context_t h = gfal_initG(&tmp_err);
    g_assert(tmp_err == NULL);
    g_assert(h != NULL);
    return h;
}

plugin_handle init_gridftp_plugin_test(gfal2_context_t h)
{
    GError * tmp_err = NULL;

    plugin_handle p = gridftp_plugin_load(h, &tmp_err);
    g_assert(p && tmp_err ==NULL);
    return p;
}

TEST(gfalGridFTP, load_gridftp)
{
    GError * tmp_err = NULL;
    core_init();
    gfal2_context_t h = gfal_initG(&tmp_err);
    ASSERT_TRUE(tmp_err== NULL && h);

    plugin_handle p = gridftp_plugin_load(h, &tmp_err);
    ASSERT_TRUE(p && tmp_err ==NULL);
    gridftp_plugin_unload(p);

    gfal_handle_freeG(h);
}

TEST(gfalGridFTP,handle_creation)
{
    GError * tmp_err = NULL;
    gfal2_context_t h = gfal_initG(&tmp_err);
    ASSERT_TRUE(tmp_err== NULL && h);

    GridFTPFactory* f = new GridFTPFactory(h);
    GridFTPModule* copy = new GridFTPModule(f);
    ASSERT_TRUE(copy != NULL);
    // create and delete properly
    GridFTPSession* session = f->get_session("gsiftp://fsdfdsfsd/fsdfds");
    f->release_session(session, false);

    // wild delete for exception clean recovery
    session = f->get_session("gsiftp://fsdfdsfsd/fsdfds");
    delete session;
    delete copy;
    gfal_handle_freeG(h);
}

TEST(gfalGridFTP,gridftp_parseURL)
{
    // check null handle, must not segfault
    gridftp_check_url_transfer(NULL, NULL, "gsiftp://myurl.com/mypath/myfile",
            "gsiftp://myurl.com/mypath/myfile", GFAL_FILE_COPY);

    gfal2_context_t a = init_gfal_handle();
    plugin_handle p = init_gridftp_plugin_test(a);
    // check with URL null, must not segfault and return false
    bool res = gridftp_check_url_transfer(p, a, NULL, NULL, GFAL_FILE_COPY);
    ASSERT_TRUE(res == FALSE);

    res = gridftp_check_url_transfer(p, a, "gsiftp://myurl.com/mypath/myfile",
            "gsiftp://myurl.com/mypath/myfile", GFAL_FILE_COPY);
    ASSERT_TRUE(res == TRUE);
    res = gridftp_check_url_transfer(p, a, "myurl.com/mypath/myfile",
            "gsiftp://myurl.com/mypath/myfile", GFAL_FILE_COPY);
    ASSERT_TRUE(res == FALSE);
    res = gridftp_check_url_transfer(p, a, "gsiftp://myurl.com/mypath/myfile",
            "myurl.com/mypath/myfile", GFAL_FILE_COPY);
    ASSERT_TRUE(res == FALSE);
    gridftp_plugin_unload(p);

    gfal_handle_freeG(a);
}

