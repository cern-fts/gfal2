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
#include "gfal_sftp_connection.h"


struct gfal_sftp_dir_s {
    gfal_sftp_handle_t *sftp_handle;
    LIBSSH2_SFTP_HANDLE *dir_handle;
    struct dirent dent;
};
typedef struct gfal_sftp_dir_s gfal_sftp_dir_t;


gfal_file_handle gfal_sftp_opendir(plugin_handle plugin_data, const char *url, GError **err)
{
    gfal_sftp_context_t *data = (gfal_sftp_context_t*)plugin_data;
    gfal_sftp_handle_t *sftp_handle = gfal_sftp_connect(data, url, err);
    if (!sftp_handle) {
        return NULL;
    }

    gfal_sftp_dir_t *dir = g_malloc(sizeof(gfal_sftp_dir_t));
    dir->sftp_handle = sftp_handle;

    dir->dir_handle = libssh2_sftp_opendir(sftp_handle->sftp_session, sftp_handle->path);
    if (!dir->dir_handle) {
        gfal_plugin_sftp_translate_error(__func__, sftp_handle, err);
        g_free(dir);
        gfal_sftp_release(data, sftp_handle);
        return NULL;
    }

    return gfal_file_handle_new2(gfal_sftp_plugin_get_name(), dir, NULL, url);
}


int gfal_sftp_closedir(plugin_handle plugin_data, gfal_file_handle dir_desc, GError **err)
{
    gfal_sftp_context_t *data = (gfal_sftp_context_t*)plugin_data;
    gfal_sftp_dir_t *dir = gfal_file_handle_get_fdesc(dir_desc);

    libssh2_sftp_closedir(dir->dir_handle);
    gfal_sftp_release(data, dir->sftp_handle);
    g_free(dir);

    gfal_file_handle_delete(dir_desc);
    return 0;
}


struct dirent *gfal_sftp_readdirpp(plugin_handle plugin_data,
    gfal_file_handle dir_desc, struct stat *st, GError **err)
{
    gfal_sftp_dir_t *dir = gfal_file_handle_get_fdesc(dir_desc);

    LIBSSH2_SFTP_ATTRIBUTES attrs;

    int rc = libssh2_sftp_readdir(dir->dir_handle, dir->dent.d_name, sizeof(dir->dent.d_name), &attrs);
    if (rc < 0) {
        gfal_plugin_sftp_translate_error(__func__, dir->sftp_handle, err);
        return NULL;
    }
    if (rc == 0) {
        return NULL;
    }

    gfal_sftp_fill_stat(st, &attrs);

    return &dir->dent;
}


struct dirent *gfal_sftp_readdir(plugin_handle plugin_data,
    gfal_file_handle dir_desc, GError **err)
{
    struct stat st;
    return gfal_sftp_readdirpp(plugin_data, dir_desc, &st, err);
}
