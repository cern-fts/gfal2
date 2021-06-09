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

#ifndef GFAL_XROOTD_PLUGIN_INTERFACE_H_
#define GFAL_XROOTD_PLUGIN_INTERFACE_H_

#include <gfal_plugins_api.h>

#define XROOTD_CONFIG_GROUP     "XROOTD PLUGIN"
#define XROOTD_DEFAULT_CHECKSUM "COPY_CHECKSUM_TYPE"
#define XROOTD_CHECKSUM_MODE    "COPY_CHECKSUM_MODE"
#define XROOTD_PARALLEL_COPIES  "PARALLEL_COPIES"
#define XROOTD_NORMALIZE_PATH   "NORMALIZE_PATH"
#define XROOTD_MONIT_MESSAGE    "MONIT_MESSAGE"

extern "C" {


int gfal_xrootd_statG(plugin_handle handle, const char* name, struct stat* buff, GError ** err);

gfal_file_handle gfal_xrootd_openG(plugin_handle handle, const char *path, int flag, mode_t mode, GError ** err);

ssize_t gfal_xrootd_readG(plugin_handle handle, gfal_file_handle fd, void *buff, size_t count, GError ** err);

ssize_t gfal_xrootd_writeG(plugin_handle handle, gfal_file_handle fd, const void *buff, size_t count, GError ** err);

off_t gfal_xrootd_lseekG(plugin_handle handle, gfal_file_handle fd, off_t offset, int whence, GError **err);

int gfal_xrootd_closeG(plugin_handle handle, gfal_file_handle fd, GError ** err);

int gfal_xrootd_mkdirpG(plugin_handle plugin_data, const char *url, mode_t mode, gboolean pflag, GError **err);

int gfal_xrootd_chmodG(plugin_handle plugin_data, const char *url, mode_t mode, GError **err);

int gfal_xrootd_unlinkG(plugin_handle plugin_data, const char *url, GError **err);

int gfal_xrootd_rmdirG(plugin_handle plugin_data, const char *url, GError **err);

int gfal_xrootd_accessG(plugin_handle plugin_data, const char *url, int mode, GError **err);

int gfal_xrootd_renameG(plugin_handle plugin_data, const char *oldurl, const char *urlnew, GError **err);

gfal_file_handle gfal_xrootd_opendirG(plugin_handle plugin_data, const char* url, GError** err);

struct dirent* gfal_xrootd_readdirG(plugin_handle plugin_data, gfal_file_handle dir_desc, GError** err);

struct dirent* gfal_xrootd_readdirppG(plugin_handle plugin_data, gfal_file_handle dir_desc, struct stat* st, GError** err);

int gfal_xrootd_closedirG(plugin_handle plugin_data, gfal_file_handle dir_desc, GError** err);

int gfal_xrootd_checksumG(plugin_handle data, const char* url, const char* check_type,
                          char * checksum_buffer, size_t buffer_length,
                          off_t start_offset, size_t data_length,
                          GError ** err);

ssize_t gfal_xrootd_getxattrG(plugin_handle plugin_data, const char* url, const char* key,
                            void* buff, size_t s_buff, GError** err);

ssize_t gfal_xrootd_listxattrG(plugin_handle plugin_data, const char* url,
        char* list, size_t s_list, GError** err);

int gfal_xrootd_setxattrG(plugin_handle plugin_data, const char* url, const char* key,
                    const void* buff , size_t s_buff, int flags, GError** err);

int gfal_xrootd_3rdcopy_check(plugin_handle plugin_data,
        gfal2_context_t context, const char* src, const char* dst,
        gfal_url2_check check);

int gfal_xrootd_3rd_copy(plugin_handle plugin_data, gfal2_context_t context,
                         gfalt_params_t params,
                         const char* src, const char* dst,
                         GError** err);

int gfal_xrootd_3rd_copy_bulk(plugin_handle plugin_data,
        gfal2_context_t context, gfalt_params_t params, size_t nbfiles,
        const char* const * srcs, const char* const * dsts,
        const char* const * checksums, GError** op_error,
        GError*** file_errors);

ssize_t gfal_xrootd_space_reporting(plugin_handle plugin_data, const char *sanitizedUrl,
    const char *key, void *buff, size_t s_buf, GError **err);

int gfal_xrootd_bring_online(plugin_handle plugin_data,
    const char* url, time_t pintime, time_t timeout, char* token, size_t tsize, int async, GError** err);
int gfal_xrootd_bring_online_poll(plugin_handle plugin_data,
    const char* url, const char* token, GError** err);

int gfal_xrootd_release_file(plugin_handle plugin_data,
    const char* url, const char* token, GError** err);

int gfal_xrootd_bring_online_list(plugin_handle plugin_data,
    int nbfiles, const char* const* urls, time_t pintime, time_t timeout, char* token, size_t tsize,
    int async, GError** err);

int gfal_xrootd_bring_online_poll_list(plugin_handle plugin_data,
    int nbfiles, const char* const* urls, const char* token, GError** err);

int gfal_xrootd_release_file_list(plugin_handle plugin_data,
    int nbfiles, const char* const* urls, const char* token, GError** err);

int gfal_xrootd_abort_files(plugin_handle plugin_data,
    int nbfiles, const char* const* urls, const char* token, GError** err);

int gfal_xrootd_archive_poll(plugin_handle plugin_data, const char* url, GError** err);

int gfal_xrootd_archive_poll_list(plugin_handle plugin_data,
    int nbfiles, const char* const* urls, GError** errors);

const char* gfal_xrootd_getName();

void set_xrootd_log_level();

}

#endif /* GFAL_XROOTD_PLUGIN_INTERFACE_H_ */
