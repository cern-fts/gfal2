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

#pragma once

#include <glib.h>
#include <gfal_srm_ifce_types.h>
#include "gfal_srm.h"

int gfal_srm_accessG(plugin_handle handle, const char* surl, int mode, GError** err);

int gfal_srm_checksumG_fallback(plugin_handle handle, const char* url, const char* check_type,
                       char * checksum_buffer, size_t buffer_length,
                       off_t start_offset, size_t data_length,
                       gboolean turl_fallback,
                       GError ** err);

int gfal_srm_checksumG(plugin_handle handle, const char* url, const char* check_type,
                       char * checksum_buffer, size_t buffer_length,
                       off_t start_offset, size_t data_length,
                       GError ** err);

int gfal_srm_chmodG(plugin_handle, const char *, mode_t, GError** err);

ssize_t gfal_srm_getxattrG(plugin_handle handle, const char* path,
        const char* name, void* buff, size_t s_buff, GError** err);

ssize_t gfal_srm_status_getxattrG(plugin_handle handle, const char* path,
        const char* name, void* buff, size_t s_buff, GError** err);

ssize_t gfal_srm_listxattrG(plugin_handle handle, const char* path, char* list,
        size_t size, GError** err);

int gfal_srm_mkdir_recG(plugin_handle ch, const char* surl, mode_t mode, GError** err);

int gfal_srm_mkdirG(plugin_handle handle, const char* surl, mode_t mode, gboolean pflag, GError** err);

int gfal_srm_renameG(plugin_handle plugin_data, const char * oldurl, const char * urlnew, GError** err);

int gfal_srm_unlinkG(plugin_handle ch, const char * path, GError** err);

int gfal_srm_unlink_listG(plugin_handle ch, int nbfiles, const char* const* paths, GError** err);

int gfal_srm_rmdirG(plugin_handle handle, const char* surl, GError** err);

int gfal_srm_statG(plugin_handle handle, const char* surl, struct stat* buf, GError** err);

int gfal_statG_srmv2_internal(srm_context_t context, struct stat* buf, TFileLocality* loc, const char* surl, GError** err);
