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


int gfal_plugin_mock_stat(plugin_handle plugin_data, const char *path, struct stat *buf, GError **err)
{
    MockPluginData *mdata = plugin_data;

    char arg_buffer[64] = {0};
    int errcode = 0;
    int signum = 0;
    long long size = 0, wait_time = 0;

    // Is fts_url_copy calling us?
    const char *agent, *version;
    gfal2_get_user_agent(mdata->handle, &agent, &version);
    int is_url_copy = (agent && strncmp(agent, "fts_url_copy", 12) == 0);

    // Wait a bit?
    gfal_plugin_mock_get_value(path, "wait", arg_buffer, sizeof(arg_buffer));
    wait_time = gfal_plugin_mock_get_int_from_str(arg_buffer);
    if (wait_time > 0) {
        sleep(wait_time);
    }

    // Trigger signal
    gfal_plugin_mock_get_value(path, "signal", arg_buffer, sizeof(arg_buffer));
    signum = gfal_plugin_mock_get_int_from_str(arg_buffer);
    if (signum > 0 && mdata->enable_signals) {
        sleep(1);
        raise(signum);
    }

    // Check errno first
    gfal_plugin_mock_get_value(path, "errno", arg_buffer, sizeof(arg_buffer));
    errcode = gfal_plugin_mock_get_int_from_str(arg_buffer);
    if (errcode > 0) {
        gfal_plugin_mock_report_error(strerror(errcode), errcode, err);
        return -1;
    }

    // Default size first
    gfal_plugin_mock_get_value(path, "size", arg_buffer, sizeof(arg_buffer));
    size = gfal_plugin_mock_get_int_from_str(arg_buffer);

    // Try specific stage then
    if (is_url_copy) {
        switch (mdata->stat_stage) {
            case STAT_DESTINATION_BEFORE_TRANSFER:
                mdata->stat_stage = STAT_DESTINATION_AFTER_TRANSFER;

                gfal_plugin_mock_get_value(path, "size_pre", arg_buffer, sizeof(arg_buffer));
                size = gfal_plugin_mock_get_int_from_str(arg_buffer);
                // If this size were <= 0, consider it a ENOENT
                if (size <= 0) {
                    gfal_plugin_mock_report_error(strerror(ENOENT), ENOENT, err);
                    return -1;
                }
                break;
            case STAT_DESTINATION_AFTER_TRANSFER:
                mdata->stat_stage = STAT_SOURCE;

                gfal_plugin_mock_get_value(path, "size_post", arg_buffer, sizeof(arg_buffer));
                size = gfal_plugin_mock_get_int_from_str(arg_buffer);
                break;
            case STAT_SOURCE:
                mdata->stat_stage = STAT_DESTINATION_BEFORE_TRANSFER;
                break;
        }
    }

    // Set the struct
    memset(buf, 0x00, sizeof(*buf));
    buf->st_size = size;
    buf->st_mode = 0755;

    arg_buffer[0] = '\0';
    gfal_plugin_mock_get_value(path, "list", arg_buffer, sizeof(arg_buffer));
    if (arg_buffer[0]) {
        buf->st_mode |= S_IFDIR;
    }
    else {
        buf->st_mode |= S_IFREG;
    }

    return 0;
}


int gfal_plugin_mock_unlink(plugin_handle plugin_data, const char *url, GError **err)
{
    struct stat buf;
    if (gfal_plugin_mock_stat(plugin_data, url, &buf, err) < 0)
        return -1;
    return 0;
}
