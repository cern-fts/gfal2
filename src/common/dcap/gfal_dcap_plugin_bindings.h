/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
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


/*
 * gfal_dcap_plugin_main.c
 * headers for bindings for dcap funcs
 * Devresse Adrien
 * 
 */


#include <regex.h>
#include <time.h> 
#include <glib.h>
#include <common/gfal_common_internal.h>
#include <common/gfal_common_errverbose.h>
#include <common/gfal_common_plugin.h>
#include <common/gfal_common_filedescriptor.h>
#include <common/gfal_types.h>

gfal_file_handle gfal_dcap_openG(plugin_handle handle , const char* path, int flag, mode_t mode, GError** err);

ssize_t gfal_dcap_readG(plugin_handle handle , gfal_file_handle fd, void* buff, size_t s_buff, GError** err);

ssize_t gfal_dcap_preadG(plugin_handle handle , gfal_file_handle fd, void* buff, size_t s_buff, off_t offset,  GError** err);

ssize_t gfal_dcap_pwriteG(plugin_handle handle , gfal_file_handle fd, const void* buff, size_t s_buff, off_t offset,  GError** err);

off_t gfal_dcap_lseekG(plugin_handle handle , gfal_file_handle fd, off_t offset, int whence, GError** err);

ssize_t gfal_dcap_writeG(plugin_handle handle , gfal_file_handle fd, const void* buff, size_t s_buff, GError** err);

int gfal_dcap_closeG(plugin_handle handle, gfal_file_handle fd, GError ** err);

int gfal_dcap_statG(plugin_handle handle, const char* name, struct stat* buff, GError ** err);

int gfal_dcap_lstatG(plugin_handle handle, const char* name, struct stat* buff, GError ** err);

int gfal_dcap_mkdirG(plugin_handle handle, const char* name, mode_t mode, gboolean pflag, GError** err);

int gfal_dcap_chmodG(plugin_handle handle, const char* name, mode_t mode,  GError** err);

int gfal_dcap_rmdirG(plugin_handle handle, const char* name, GError** err);

gfal_file_handle gfal_dcap_opendirG(plugin_handle handle, const char* path, GError ** err);

int gfal_dcap_closedirG(plugin_handle handle, gfal_file_handle fh, GError** err);

struct dirent* gfal_dcap_readdirG(plugin_handle handle, gfal_file_handle fh, GError** err);

const char* gfal_dcap_getName();
