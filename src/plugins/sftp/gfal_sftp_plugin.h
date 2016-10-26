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

#ifndef GFAL_SFTP_PLUGIN_H
#define GFAL_SFTP_PLUGIN_H

#include <gfal_plugins_api.h>
#include <libssh2_sftp.h>
#include "gfal_sftp_connection.h"


GQuark gfal2_get_plugin_sftp_quark();


// Get the plugin name
const char *gfal_sftp_plugin_get_name();

// Metadata operations
void gfal_sftp_fill_stat(struct stat *st, LIBSSH2_SFTP_ATTRIBUTES *attrs);

int gfal_sftp_stat(plugin_handle plugin_data,
    const char *url, struct stat *buf, GError **err);

int gfal_sftp_unlink(plugin_handle plugin_data,
    const char *url, GError **err);

int gfal_sftp_rename(plugin_handle plugin_data,
    const char *oldurl, const char *urlnew, GError **err);

int gfal_sftp_mkdir(plugin_handle plugin_data,
    const char *url, mode_t mode, gboolean rec_flag, GError **err);

int gfal_sftp_rmdir(plugin_handle plugin_data,
    const char *url, GError **err);

int gfal_sftp_symlink(plugin_handle plugin_data,
    const char *oldurl, const char *urlnew, GError **err);

ssize_t gfal_sftp_readlink(plugin_handle plugin_data,
    const char *url, char *buff, size_t buffsiz, GError **err);

int	gfal_sftp_chmod(plugin_handle plugin_data, const char * url, mode_t mode, GError** err);

// Directory operations
gfal_file_handle gfal_sftp_opendir(plugin_handle plugin_data,
    const char *url, GError **err);

int gfal_sftp_closedir(plugin_handle plugin_data,
    gfal_file_handle dir_desc, GError **err);

struct dirent *gfal_sftp_readdirpp(plugin_handle plugin_data,
    gfal_file_handle dir_desc, struct stat *st,
    GError **err);

struct dirent *gfal_sftp_readdir(plugin_handle plugin_data,
    gfal_file_handle dir_desc, GError **err);

// IO operations
gfal_file_handle gfal_sftp_open(plugin_handle plugin_data,
    const char *url, int flag, mode_t mode, GError **err);

ssize_t gfal_sftp_read(plugin_handle plugin_data, gfal_file_handle fd,
    void *buff, size_t count, GError **err);

ssize_t gfal_sftp_write(plugin_handle plugin_data, gfal_file_handle fd,
    const void *buff, size_t count, GError **err);

int gfal_sftp_close(plugin_handle plugin_data, gfal_file_handle fd, GError **err);

off_t gfal_sftp_seek(plugin_handle plugin_data, gfal_file_handle fd,
    off_t offset, int whence, GError **err);

#endif // GFAL_SFTP_PLUGIN_H
