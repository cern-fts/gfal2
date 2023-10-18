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
#include <gfal_plugins_api.h>
#include <gtest/gtest.h>


TEST(gfalGlobal, testLogLevel)
{
    // Restrict log level to "Warning" to reduce verbosity
    gfal2_log_set_level(G_LOG_LEVEL_WARNING);
    GLogLevelFlags r = gfal2_log_get_level();
    ASSERT_EQ(G_LOG_LEVEL_WARNING, r);
}


TEST(gfalGlobal, testLoad)
{
    GError *tmp_err = NULL;
    gfal2_context_t c = gfal2_context_new(&tmp_err);
    if (tmp_err)
        printf("%s\n", tmp_err->message);
    ASSERT_EQ(NULL, tmp_err);
    ASSERT_NE((void *) NULL, c);
    gfal2_context_free(c);
    c = NULL;
    gfal2_context_free(c);
}


static const char *test_plugin_get_name(void)
{
    return "TEST PLUGIN";
}


static gboolean test_plugin_url(plugin_handle plugin_data, const char *url,
    plugin_mode operation, GError **err)
{
    return strncmp(url, "test://", 7) == 0 && operation == GFAL_PLUGIN_STAT;
}


static int test_plugin_stat(plugin_handle plugin_data, const char *url, struct stat *buf, GError **err)
{
    buf->st_mode = 12345;
    return 0;
}


TEST(gfalGlobal, registerPlugin)
{
    GError *tmp_err = NULL;
    gfal2_context_t c = gfal2_context_new(&tmp_err);
    ASSERT_NE((void *) NULL, c);

    gfal_plugin_interface test_plugin;
    memset(&test_plugin, 0, sizeof(test_plugin));

    test_plugin.getName = test_plugin_get_name;
    test_plugin.check_plugin_url = test_plugin_url;
    test_plugin.statG = test_plugin_stat;

    int ret = gfal2_register_plugin(c, &test_plugin, &tmp_err);
    if (tmp_err)
        printf("%s\n", tmp_err->message);
    ASSERT_EQ(0, ret);

    struct stat st;
    ret = gfal2_stat(c, "test://blah", &st, &tmp_err);
    ASSERT_EQ(0, ret);

    ASSERT_EQ(12345, st.st_mode);

    gchar **plugins = gfal2_get_plugin_names(c);
    int found = 0;
    for (int i = 0; plugins[i] != NULL; ++i) {
        found += (strncmp(plugins[i], "TEST PLUGIN", 11) == 0);
    }
    ASSERT_EQ(found, 1);
    g_strfreev(plugins);

    gfal2_context_free(c);
}
