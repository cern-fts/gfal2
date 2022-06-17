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

#include "gfal_mock_plugin.h"
#include <string.h>
#include <uuid/uuid.h>


static GHashTable *staging_end_table;

__attribute__((constructor))
static void init() {
    staging_end_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
}


int gfal_plugin_mock_bring_online(plugin_handle plugin_data, const char *url,
    time_t pintime, time_t timeout, char *token, size_t tsize, int async,
    GError **err)
{
    char arg_buffer[64];

    // Bring online errno
    gfal_plugin_mock_get_value(url, "staging_errno", arg_buffer, sizeof(arg_buffer));
    int staging_errno = gfal_plugin_mock_get_int_from_str(arg_buffer);

    // Polling time
    gfal_plugin_mock_get_value(url, "staging_time", arg_buffer, sizeof(arg_buffer));
    time_t *staging_end = g_new0(time_t, 1);
    *staging_end = time(NULL) + gfal_plugin_mock_get_int_from_str(arg_buffer);

    g_hash_table_insert(staging_end_table, g_strdup(url), staging_end);

    // Fake token
    if (tsize > 36) {
        uuid_t uuid;
        uuid_generate_random(uuid);
        uuid_unparse(uuid, token);
    }
    else {
        g_strlcpy(token, "mock-token", tsize);
    }

    // Now, if remaining is <= 0, or blocking call, we are done
    if (*staging_end <= time(NULL) || !async) {
        if (staging_errno) {
            gfal_plugin_mock_report_error(strerror(staging_errno), staging_errno, err);
            return -1;
        }
        return 1;
    }

    return 0;
}


int gfal_plugin_mock_bring_online_v2(plugin_handle plugin_data, const char *url, const char *metadata,
    time_t pintime, time_t timeout, char *token, size_t tsize, int async,
    GError **err)
{
    return gfal_plugin_mock_bring_online(plugin_data, url, pintime, timeout, token, tsize, async, err);
}


int gfal_plugin_mock_bring_online_poll(plugin_handle plugin_data,
    const char *url, const char *token, GError **err)
{
    char arg_buffer[64];
    gfal_plugin_mock_get_value(url, "staging_errno", arg_buffer, sizeof(arg_buffer));
    int staging_errno = gfal_plugin_mock_get_int_from_str(arg_buffer);

    time_t *staging_end = g_hash_table_lookup(staging_end_table, url);

    if (staging_end == NULL || *staging_end <= time(NULL)) {
        if (staging_errno) {
            gfal_plugin_mock_report_error(strerror(staging_errno), staging_errno, err);
            return -1;
        }
        return 1;
    }
    else {
        gfal_plugin_mock_report_error("Not ready", EAGAIN, err);
    }

    return 0;
}


int gfal_plugin_mock_release_file(plugin_handle plugin_data, const char *url,
    const char *token, GError **err)
{
    char arg_buffer[64];
    gfal_plugin_mock_get_value(url, "release_errno", arg_buffer, sizeof(arg_buffer));
    int release_errno = gfal_plugin_mock_get_int_from_str(arg_buffer);

    if (release_errno) {
        gfal_plugin_mock_report_error(strerror(release_errno), release_errno, err);
        return -1;
    }
    return 0;
}


int gfal_plugin_mock_bring_online_list(plugin_handle plugin_data, int nbfiles,
    const char *const *urls, time_t pintime, time_t timeout, char *token,
    size_t tsize, int async, GError **err)
{
    int terminal_count = 0, r, i;

    for (i = 0; i < nbfiles; ++i) {
        r = gfal_plugin_mock_bring_online(plugin_data, urls[i], pintime, timeout, token, tsize, async, &(err[i]));
        if (r > 0)
            ++terminal_count;
    }

    if (terminal_count == nbfiles)
        return 1;
    return 0;
}


int gfal_plugin_mock_bring_online_list_v2(plugin_handle plugin_data, int nbfiles,
    const char *const *urls, const char *const *metadata, time_t pintime, time_t timeout,
    char *token, size_t tsize, int async, GError **err)
{
    return gfal_plugin_mock_bring_online_list(plugin_data, nbfiles, urls, pintime, timeout, token, tsize, async, err);
}


int gfal_plugin_mock_bring_online_poll_list(plugin_handle plugin_data,
    int nbfiles, const char *const *urls, const char *token, GError **err)
{
    int terminal_count = 0, r, i, error_count = 0;

    for (i = 0; i < nbfiles; ++i) {
        r = gfal_plugin_mock_bring_online_poll(plugin_data, urls[i], token, &(err[i]));
        if (r > 0) {
            ++terminal_count;
        }
        else if (r < 0) {
            ++terminal_count;
            ++error_count;
        }
    }

    if (terminal_count == nbfiles)
        return 1;
    return 0;
}


int gfal_plugin_mock_release_file_list(plugin_handle plugin_data, int nbfiles,
    const char *const *urls, const char *token, GError **err)
{
    int i;
    for (i = 0; i < nbfiles; ++i) {
        gfal_plugin_mock_release_file(plugin_data, urls[i], token, &(err[i]));
    }
    return 1;
}


int gfal_plugin_mock_abort_file_list(plugin_handle plugin_data, int nbfiles, const char *const *uris, const char *token,
    GError **err)
{
    MockPluginData *mdata = plugin_data;

    // Just make sure the pointers are at least valid, so access them
    int token_len = strlen(token);
    int i = 0, total_len = 0;
    for (i = 0; i < nbfiles; ++i) {
        total_len += strlen(uris[i]);
    }

    gfal2_log(G_LOG_LEVEL_DEBUG, "Counter to avoid optimizing away: %d (state %d)",
        token_len + total_len, mdata->stat_stage);

    return 0;
}
