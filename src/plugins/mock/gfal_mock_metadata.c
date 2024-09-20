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
    unsigned long long size = 0, wait_time = 0;

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
    size = gfal_plugin_mock_get_unsigned_int_from_str(arg_buffer);

    // Try specific stage then
    if (is_url_copy) {
        switch (mdata->stat_stage) {
            case STAT_DESTINATION_BEFORE_TRANSFER:
                mdata->stat_stage = STAT_DESTINATION_AFTER_TRANSFER;

                gfal_plugin_mock_get_value(path, "size_pre", arg_buffer, sizeof(arg_buffer));
                size = gfal_plugin_mock_get_unsigned_int_from_str(arg_buffer);
                // If this size were <= 0, consider it a ENOENT
                if (size <= 0) {
                    gfal_plugin_mock_report_error(strerror(ENOENT), ENOENT, err);
                    return -1;
                }
                break;
            case STAT_DESTINATION_AFTER_TRANSFER:
                mdata->stat_stage = STAT_SOURCE;

                gfal_plugin_mock_get_value(path, "size_post", arg_buffer, sizeof(arg_buffer));
                size = gfal_plugin_mock_get_unsigned_int_from_str(arg_buffer);
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

int gfal_plugin_mock_access(plugin_handle plugin_data, const char* url, int mode, GError** err)
{
    char arg_buffer[64] = {0};

    gfal_plugin_mock_get_value(url, "access", arg_buffer, sizeof(arg_buffer));
    if (arg_buffer[0]) {
        const int has_access = gfal_plugin_mock_get_int_from_str(arg_buffer);
        if (has_access > 0) {
            return 1;
        }
    }

    gfal_plugin_mock_get_value(url, "exists", arg_buffer, sizeof(arg_buffer));
    if (arg_buffer[0]) {
        const int has_access = gfal_plugin_mock_get_int_from_str(arg_buffer);
        if (has_access > 0) {
            return 1;
        }
    }

    gfal_plugin_mock_get_value(url, "access_errno", arg_buffer, sizeof(arg_buffer));
    int code = gfal_plugin_mock_get_int_from_str(arg_buffer);
    if (code > 0) {
        gfal_plugin_mock_report_error(strerror(code), code, err);
    } else {
        gfal_plugin_mock_report_error(strerror(ENOENT), ENOENT, err);
    }

    return -1;
}

int gfal_plugin_mock_mkdirpG(plugin_handle plugin_data, const char* url, mode_t mode, gboolean rec_flag, GError** err)
{
    GStrv read_only_paths = gfal_plugin_mock_get_values(url, "rd_path");
    if (!read_only_paths) {
        return 0;
    }

    for (int i = 0; read_only_paths[i]; ++i) {
        const char *query = strchr(url, '?');
        const size_t url_len = query - url;
        if (!strncmp(url, read_only_paths[i], url_len)) {
            g_strfreev(read_only_paths);
            gfal_plugin_mock_report_error(strerror(EPERM), EPERM, err);
            return -1;
        }
    }

    g_strfreev(read_only_paths);
    return 0;
}

int gfal_plugin_mock_unlink(plugin_handle plugin_data, const char *url, GError **err)
{
    struct stat buf;
    if (gfal_plugin_mock_stat(plugin_data, url, &buf, err) < 0)
        return -1;
    return 0;
}

int gfal_mock_checksumG(plugin_handle plugin_data, const char* url,
        const char* check_type, char * checksum_buffer, size_t buffer_length,
        off_t start_offset, size_t data_length, GError ** err)
{
    char arg_buffer[GFAL_URL_MAX_LEN] = {0};
    int errcode = 0;

    // Check errno first
    gfal_plugin_mock_get_value(url, "errno", arg_buffer, sizeof(arg_buffer));
    errcode = gfal_plugin_mock_get_int_from_str(arg_buffer);

    if (errcode > 0) {
        gfal_plugin_mock_report_error(strerror(errcode), errcode, err);
        return -1;
    }

    gfal_plugin_mock_get_value(url, "checksum", arg_buffer, sizeof(arg_buffer));
    g_strlcpy(checksum_buffer, arg_buffer, buffer_length);

    return 0;
}

ssize_t gfal_mock_getxattrG(plugin_handle plugin_data, const char* url, const char* key, void* buff, size_t s_buff, GError** err)
{
    char arg_buffer[GFAL_URL_MAX_LEN] = {0};
    int errcode = 0;
    int emsg_size;
    char* emsg = NULL;

    // Check errno first
    gfal_plugin_mock_get_value(url, "errno", arg_buffer, sizeof(arg_buffer));
    errcode = gfal_plugin_mock_get_int_from_str(arg_buffer);

    if (errcode > 0) {
        gfal_plugin_mock_report_error(strerror(errcode), errcode, err);
        return -1;
    }

    if ((strncmp(key, GFAL_XATTR_STATUS, sizeof(GFAL_XATTR_STATUS)) == 0) ||
        (strncmp(key, GFAL_XATTR_REPLICA, sizeof(GFAL_XATTR_REPLICA)) == 0) ||
        (strncmp(key, GFAL_XATTR_GUID, sizeof(GFAL_XATTR_GUID)) == 0) ||
        (strncmp(key, GFAL_XATTR_COMMENT, sizeof(GFAL_XATTR_COMMENT)) == 0) ||
        (strncmp(key, GFAL_XATTR_SPACETOKEN, sizeof(GFAL_XATTR_SPACETOKEN)) == 0)) {
        gfal_plugin_mock_get_value(url, key, arg_buffer, sizeof(arg_buffer));
        g_strlcpy(buff, arg_buffer, s_buff);
    }

    if (arg_buffer[0] == '\0') {
        emsg_size = 26 + strlen(key);
        emsg =  malloc(emsg_size);
        snprintf(emsg, emsg_size, "Failed to retrieve xattr %s", key);
        gfal_plugin_mock_report_error(emsg, ENOATTR, err);
        free(emsg);
        return -1;
    }

    return strlen(buff);
}
