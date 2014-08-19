#pragma once
#ifndef GRIDFTP_NAMESPACE_H
#define GRIDFTP_NAMESPACE_H

/*
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gridftpwrapper.h"
#include "gridftpmodule.h"


#ifdef __cplusplus
extern "C"
{
#endif

int gfal_gridftp_statG(plugin_handle handle, const char* name,
        struct stat* buff, GError ** err);

int gfal_gridftp_accessG(plugin_handle handle, const char* name, int mode,
        GError** err);

int gfal_gridftp_chmodG(plugin_handle handle, const char* path, mode_t mode,
        GError** err);

int gfal_gridftp_mkdirG(plugin_handle handle, const char* path, mode_t mode,
        gboolean pflag, GError** err);

int gfal_gridftp_rmdirG(plugin_handle handle, const char* url, GError** err);

void gridftp_unlink_internal(gfal2_context_t context, GridFTP_session* sess,
        const char * path, bool own_session = true);

int gfal_gridftp_unlinkG(plugin_handle handle, const char* url, GError** err);

int gfal_gridftp_renameG(plugin_handle plugin_data, const char * oldurl,
        const char * urlnew, GError** err);

gfal_file_handle gfal_gridftp_opendirG(plugin_handle handle, const char* path,
        GError** err);

struct dirent* gfal_gridftp_readdirG(plugin_handle handle, gfal_file_handle fh,
        GError** err);

struct dirent* gfal_gridftp_readdirppG(plugin_handle handle, gfal_file_handle fh,
        struct stat*, GError** err);

int gfal_gridftp_closedirG(plugin_handle handle, gfal_file_handle fh,
        GError** err);

int gfal_gridftp_checksumG(plugin_handle handle, const char* url,
        const char* check_type, char * checksum_buffer, size_t buffer_length,
        off_t start_offset, size_t data_length, GError ** err);

#ifdef __cplusplus
}
#endif

#endif /* GRIDFTP_NAMESPACE_H */
