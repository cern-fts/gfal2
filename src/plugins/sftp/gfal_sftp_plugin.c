/*
 * Copyright (c) CERN 2016
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

#include "gfal_sftp_plugin.h"


GQuark gfal2_get_plugin_sftp_quark()
{
    return g_quark_from_static_string(GFAL2_QUARK_PLUGINS "::SFTP");
}


static gboolean is_sftp_uri(const char *src)
{
    return strncmp(src, "sftp:", 5) == 0;
}


const char *gfal_sftp_plugin_get_name()
{
    return GFAL2_PLUGIN_VERSIONED("sftp", VERSION);
}


static gboolean gfal_sftp_check_url(plugin_handle handle, const char *url, plugin_mode mode, GError **err)
{
    g_return_val_err_if_fail(url != NULL, EINVAL, err, "[gfal_sftp_check_url] Invalid url ");

    switch (mode) {
        case GFAL_PLUGIN_STAT:
        case GFAL_PLUGIN_LSTAT:
        case GFAL_PLUGIN_OPENDIR:
        case GFAL_PLUGIN_RENAME:
        case GFAL_PLUGIN_UNLINK:
        case GFAL_PLUGIN_SYMLINK:
        case GFAL_PLUGIN_READLINK:
        case GFAL_PLUGIN_MKDIR:
        case GFAL_PLUGIN_RMDIR:
        case GFAL_PLUGIN_CHMOD:
        case GFAL_PLUGIN_OPEN:
            return is_sftp_uri(url);
        default:
            return FALSE;
    }
}


static void gfal_plugin_sftp_delete(plugin_handle plugin_data)
{
    gfal_sftp_data_t *data = (gfal_sftp_data_t*)plugin_data;
    free(data);
}


gfal_plugin_interface gfal_plugin_init(gfal2_context_t context, GError **err)
{
    gfal_plugin_interface sftp_plugin;
    memset(&sftp_plugin, 0, sizeof(gfal_plugin_interface));

    gfal_sftp_data_t *data = g_malloc(sizeof(gfal_sftp_data_t));
    data->context = context;

    sftp_plugin.plugin_data = data;
    sftp_plugin.plugin_delete = gfal_plugin_sftp_delete;
    sftp_plugin.check_plugin_url = &gfal_sftp_check_url;
    sftp_plugin.getName = &gfal_sftp_plugin_get_name;

    sftp_plugin.statG = &gfal_sftp_stat;
    sftp_plugin.lstatG = &gfal_sftp_stat;

    sftp_plugin.opendirG = gfal_sftp_opendir;
    sftp_plugin.readdirG = gfal_sftp_readdir;
    sftp_plugin.readdirppG = gfal_sftp_readdirpp;
    sftp_plugin.closedirG = gfal_sftp_closedir;

    sftp_plugin.renameG = &gfal_sftp_rename;
    sftp_plugin.unlinkG = &gfal_sftp_unlink;
    sftp_plugin.mkdirpG = &gfal_sftp_mkdir;
    sftp_plugin.rmdirG = &gfal_sftp_rmdir;
    sftp_plugin.symlinkG = &gfal_sftp_symlink;
    sftp_plugin.readlinkG = &gfal_sftp_readlink;
    sftp_plugin.chmodG = &gfal_sftp_chmod;

    sftp_plugin.openG = gfal_sftp_open;
    sftp_plugin.closeG = gfal_sftp_close;
    sftp_plugin.readG = gfal_sftp_read;
    sftp_plugin.writeG = gfal_sftp_write;
    sftp_plugin.lseekG = gfal_sftp_seek;

    return sftp_plugin;
}
