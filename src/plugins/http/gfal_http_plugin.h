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

#ifndef _GFAL_HTTP_PLUGIN_H
#define _GFAL_HTTP_PLUGIN_H

#include <gfal_plugins_api.h>


#include <davix.hpp>


class GfalHttpPluginData {
public:
    GfalHttpPluginData(gfal2_context_t);

    Davix::Context  context;
    Davix::DavPosix posix;
    gfal2_context_t handle;

    void get_params(Davix::RequestParams*, const Davix::Uri& uri);

private:
    Davix::RequestParams reference_params;

};

const char* gfal_http_get_name(void);

GfalHttpPluginData* gfal_http_get_plugin_context(gpointer plugin_data);

void gfal_http_context_delete(gpointer plugin_data);

extern GQuark http_plugin_domain;

// Initializes a GError from a DavixError
void davix2gliberr(const Davix::DavixError* daverr, GError** err);

// Initializes a GError from an HTTP code
void http2gliberr(GError** err, int http, const char* func, const char* msg);

// Removes +3rd from the url, if there
void strip_3rd_from_url(const char* url_full, char* url, size_t url_size);

// METADATA OPERATIONS
void gfal_http_delete(plugin_handle plugin_data);

int gfal_http_stat(plugin_handle plugin_data, const char* url, struct stat* buf, GError** err);

int gfal_http_rename(plugin_handle plugin_data, const char* oldurl, const char* newurl, GError** err);

int gfal_http_access(plugin_handle plugin_data, const char* url, int mode, GError** err);

int gfal_http_mkdirpG(plugin_handle plugin_data, const char* url, mode_t mode, gboolean rec_flag, GError** err);

int gfal_http_rmdirG(plugin_handle plugin_data, const char* url, GError** err);

int gfal_http_unlinkG(plugin_handle plugin_data, const char* url, GError** err);

gfal_file_handle gfal_http_opendir(plugin_handle plugin_data, const char* url, GError** err);

struct dirent* gfal_http_readdir(plugin_handle plugin_data, gfal_file_handle dir_desc, GError** err);

struct dirent* gfal_http_readdirpp(plugin_handle plugin_data, gfal_file_handle dir_desc, struct stat* st, GError** err);

int gfal_http_closedir(plugin_handle plugin_data, gfal_file_handle dir_desc, GError** err);

// IO
gfal_file_handle gfal_http_fopen(plugin_handle plugin_data, const char* url, int flag, mode_t mode, GError** err);

ssize_t gfal_http_fread(plugin_handle, gfal_file_handle fd, void* buff, size_t count, GError** err);

ssize_t gfal_http_fwrite(plugin_handle, gfal_file_handle fd, const void* buff, size_t count, GError** err);

int gfal_http_fclose(plugin_handle, gfal_file_handle fd, GError ** err);

off_t gfal_http_fseek(plugin_handle, gfal_file_handle fd, off_t offset, int whence, GError** err);

// Checksum
int gfal_http_checksum(plugin_handle data, const char* url, const char* check_type,
                       char * checksum_buffer, size_t buffer_length,
                       off_t start_offset, size_t data_length,
                       GError ** err);


int gfal_http_copy(plugin_handle plugin_data, gfal2_context_t context, gfalt_params_t params,
        const char* src, const char* dst, GError** err);

int gfal_http_copy_check(plugin_handle plugin_data, gfal2_context_t context,
        const char* src, const char* dst, gfal_url2_check check);

#endif //_GFAL_HTTP_PLUGIN_H
