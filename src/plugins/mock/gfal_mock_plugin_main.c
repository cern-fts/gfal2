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

#include <glib.h>
#include <string.h>
#include <sys/stat.h>

#include "gfal_mock_plugin.h"


GQuark gfal2_get_plugin_mock_quark()
{
    return g_quark_from_static_string(GFAL2_QUARK_PLUGINS "::FILE");
}


static gboolean is_mock_uri(const char *src)
{
    return strncmp(src, "mock:", 5) == 0;
}


const char *gfal_mock_plugin_getName()
{
    return GFAL2_PLUGIN_VERSIONED("mock", VERSION);
}


static gboolean gfal_mock_check_url(plugin_handle handle, const char *url, plugin_mode mode, GError **err)
{
    g_return_val_err_if_fail(url != NULL, EINVAL, err, "[gfal_lfile_path_checker] Invalid url ");

    switch (mode) {
        //case GFAL_PLUGIN_ACCESS:
        //case GFAL_PLUGIN_MKDIR:
        case GFAL_PLUGIN_STAT:
        case GFAL_PLUGIN_LSTAT:
            //case GFAL_PLUGIN_RMDIR:
        case GFAL_PLUGIN_OPENDIR:
            //case GFAL_PLUGIN_OPEN:
            //case GFAL_PLUGIN_CHMOD:
        case GFAL_PLUGIN_UNLINK:
            //case GFAL_PLUGIN_GETXATTR:
            //case GFAL_PLUGIN_LISTXATTR:
            //case GFAL_PLUGIN_SETXATTR:
            //case GFAL_PLUGIN_RENAME:
            //case GFAL_PLUGIN_SYMLINK:
            //case GFAL_PLUGIN_CHECKSUM:
        case GFAL_PLUGIN_BRING_ONLINE:
        case GFAL_PLUGIN_OPEN:
            return is_mock_uri(url);
        default:
            return FALSE;
    }
}

void gfal_plugin_mock_report_error(const char *msg, int errn, GError **err)
{
    g_set_error(err, gfal2_get_plugin_mock_quark(), errn, "%s", msg);
}


void gfal_plugin_mock_get_value(const char *url, const char *key, char *value, size_t val_size)
{
    // make sure it's an empty C-string
    value[0] = '\0';

    char *str = strchr(url, '?');
    if (str == NULL) {
        return;
    }

    size_t key_len = strlen(key);
    char **args = g_strsplit(str + 1, "&", 0);
    int i;
    for (i = 0; args[i] != NULL; ++i) {
        if (strncmp(args[i], key, key_len) == 0) {
            char *p = strchr(args[i], '=');
            if (p) {
                g_strlcpy(value, p + 1, val_size);
                break;
            }
        }
    }

    g_strfreev(args);
}


long long gfal_plugin_mock_get_int_from_str(const char *buff)
{
    if (buff == 0 || buff[0] == '\0')
        return 0;
    return atoll(buff);
}


gboolean gfal_plugin_mock_check_url_transfer(plugin_handle handle, gfal2_context_t ctx, const char *src,
    const char *dst, gfal_url2_check type)
{
    gboolean res = FALSE;
    if (src != NULL && dst != NULL) {
        if (type == GFAL_FILE_COPY && is_mock_uri(src) && is_mock_uri(dst)) {
            res = TRUE;
        }
    }
    return res;
}


void gfal_plugin_mock_delete(plugin_handle plugin_data)
{
    MockPluginData *mdata = (MockPluginData*)plugin_data;
    g_hash_table_destroy(mdata->staging_end);
    free(plugin_data);
}


/*
 * Init function, called before all
 **/
gfal_plugin_interface gfal_plugin_init(gfal2_context_t handle, GError **err)
{
    gfal_plugin_interface mock_plugin;
    memset(&mock_plugin, 0, sizeof(gfal_plugin_interface));

    MockPluginData *mdata = calloc(1, sizeof(MockPluginData));
    mdata->handle = handle;
    mdata->staging_end = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

    mock_plugin.plugin_data = mdata;
    mock_plugin.plugin_delete = gfal_plugin_mock_delete;
    mock_plugin.check_plugin_url = &gfal_mock_check_url;
    mock_plugin.getName = &gfal_mock_plugin_getName;

    mock_plugin.statG = &gfal_plugin_mock_stat;
    mock_plugin.lstatG = &gfal_plugin_mock_stat;
    mock_plugin.unlinkG = &gfal_plugin_mock_unlink;

    mock_plugin.bring_online = gfal_plugin_mock_bring_online;
    mock_plugin.bring_online_poll = gfal_plugin_mock_bring_online_poll;
    mock_plugin.release_file = gfal_plugin_mock_release_file;

    mock_plugin.bring_online_list = gfal_plugin_mock_bring_online_list;
    mock_plugin.bring_online_poll_list = gfal_plugin_mock_bring_online_poll_list;
    mock_plugin.release_file_list = gfal_plugin_mock_release_file_list;
    mock_plugin.abort_files = gfal_plugin_mock_abort_file_list;

    mock_plugin.check_plugin_url_transfer = &gfal_plugin_mock_check_url_transfer;
    mock_plugin.copy_file = &gfal_plugin_mock_filecopy;

    mock_plugin.opendirG = gfal_plugin_mock_opendir;
    mock_plugin.readdirG = gfal_plugin_mock_readdir;
    mock_plugin.readdirppG = gfal_plugin_mock_readdirpp;
    mock_plugin.closedirG = gfal_plugin_mock_closedir;

    mock_plugin.openG = gfal_plugin_mock_open;
    mock_plugin.closeG = gfal_plugin_mock_close;
    mock_plugin.readG = gfal_plugin_mock_read;
    mock_plugin.writeG = gfal_plugin_mock_write;
    mock_plugin.lseekG = gfal_plugin_mock_seek;

    return mock_plugin;
}



