/*
 * Copyright (c) CERN 2022
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


static GHashTable *archiving_end_table;

__attribute__((constructor))
static void init() {
    archiving_end_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
}


int gfal_plugin_mock_archive_poll(plugin_handle plugin_data, const char* url, GError** err)
{
    // Archiving errno
    char arg_buffer[64];
    gfal_plugin_mock_get_value(url, "archiving_errno", arg_buffer, sizeof(arg_buffer));
    int archiving_error = gfal_plugin_mock_get_int_from_str(arg_buffer);

    // Look up in the hash table to see if a poll task is already in progress
    gboolean archiving_started = g_hash_table_contains(archiving_end_table, url);

    if (!archiving_started) {
        // Get archiving time from url
        gfal_plugin_mock_get_value(url, "archiving_time", arg_buffer, sizeof(arg_buffer));
        time_t *archiving_end = g_new0(time_t, 1);
        *archiving_end = time(NULL) + gfal_plugin_mock_get_int_from_str(arg_buffer);

        g_hash_table_insert(archiving_end_table, g_strdup(url), archiving_end);
    }

    // Get archiving time from hash table in memory
    time_t *archiving_end = g_hash_table_lookup(archiving_end_table, url);

    if (archiving_end == NULL || *archiving_end <= time(NULL)) {
        if (archiving_error) {
            gfal_plugin_mock_report_error(strerror(archiving_error), archiving_error, err);
            return -1;
        }
        return 1;
    }
    else {
        gfal_plugin_mock_report_error("Not ready", EAGAIN, err);
    }

    return 0;
}


int gfal_plugin_mock_archive_poll_list(plugin_handle plugin_data, int nbfiles, const char* const* urls, GError** errors)
{
    int terminal_count = 0, r, i, error_count = 0;

    for (i = 0; i < nbfiles; ++i) {
        r = gfal_plugin_mock_archive_poll(plugin_data, urls[i], &(errors[i]));
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
