/*
 * Copyright (c) CERN 2013-2015
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

#ifndef GFAL_MOCK_PLUGIN_H
#define GFAL_MOCK_PLUGIN_H

#include <gfal_plugins_api.h>

// Types
typedef enum {
    STAT_SOURCE = 0,
    STAT_DESTINATION_BEFORE_TRANSFER,
    STAT_DESTINATION_AFTER_TRANSFER
} StatStage;


typedef struct {
    gfal2_context_t handle;
    StatStage stat_stage;
    time_t staging_end;
} MockPluginData;


// Helpers
const char *gfal_mock_plugin_getName();

GQuark gfal2_get_plugin_mock_quark();

void gfal_plugin_mock_report_error(const char *msg, int errn, GError **err);

void gfal_plugin_mock_get_value(const char *url, const char *key, char *value, size_t val_size);

long long gfal_plugin_mock_get_int_from_str(const char* buff);

// Metadata operations
int gfal_plugin_mock_stat(plugin_handle plugin_data,
    const char *path, struct stat *buf, GError **err);

int gfal_plugin_mock_unlink(plugin_handle plugin_data,
    const char *url, GError **err);

// Directory operations
gfal_file_handle gfal_plugin_mock_opendir(plugin_handle plugin_data,
    const char *url, GError **err);

int gfal_plugin_mock_closedir(plugin_handle plugin_data,
    gfal_file_handle dir_desc, GError **err);

struct dirent *gfal_plugin_mock_readdirpp(plugin_handle plugin_data,
    gfal_file_handle dir_desc, struct stat *st,
    GError **err);

struct dirent *gfal_plugin_mock_readdir(plugin_handle plugin_data,
    gfal_file_handle dir_desc, GError **err);

// IO operations
gfal_file_handle gfal_plugin_mock_open(plugin_handle plugin_data,
    const char *url, int flag, mode_t mode, GError **);

ssize_t gfal_plugin_mock_read(plugin_handle, gfal_file_handle fd,
    void *buff, size_t count, GError **);

ssize_t gfal_plugin_mock_write(plugin_handle, gfal_file_handle fd,
    const void *buff, size_t count, GError **);

int gfal_plugin_mock_close(plugin_handle, gfal_file_handle fd, GError **);

off_t gfal_plugin_mock_seek(plugin_handle, gfal_file_handle fd,
    off_t offset, int whence, GError **err);

// Staging
int gfal_plugin_mock_bring_online(plugin_handle plugin_data, const char *url,
    time_t pintime, time_t timeout, char *token, size_t tsize, int async,
    GError **err);


int gfal_plugin_mock_bring_online_poll(plugin_handle plugin_data,
    const char *url, const char *token, GError **err);


int gfal_plugin_mock_release_file(plugin_handle plugin_data, const char *url,
    const char *token, GError **err);

int gfal_plugin_mock_bring_online_list(plugin_handle plugin_data, int nbfiles,
    const char *const *urls, time_t pintime, time_t timeout, char *token,
    size_t tsize, int async, GError **err);


int gfal_plugin_mock_bring_online_poll_list(plugin_handle plugin_data,
    int nbfiles, const char *const *urls, const char *token, GError **err);

int gfal_plugin_mock_release_file_list(plugin_handle plugin_data, int nbfiles,
    const char *const *urls, const char *token, GError **err);

int gfal_plugin_mock_abort_file_list(plugin_handle plugin_data, int nbfiles, const char *const *uris, const char *token,
    GError **err);

// Copy
int gfal_plugin_mock_filecopy(plugin_handle plugin_data,
    gfal2_context_t context, gfalt_params_t params, const char *src,
    const char *dst, GError **err);

#endif // GFAL_MOCK_PLUGIN_H
