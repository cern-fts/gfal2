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

#include <regex.h>
#include <time.h>
#include <stdio.h>

#include <gfal_plugins_api.h>

gfal_file_handle gfal_rfio_openG(plugin_handle ch , const char* path, int flag, mode_t mode, GError**);

int gfal_rfio_closeG(plugin_handle handle, gfal_file_handle fd, GError ** err);

ssize_t gfal_rfio_writeG(plugin_handle handle , gfal_file_handle fd, const void* buff, size_t s_buff, GError** err);

ssize_t gfal_rfio_readG(plugin_handle handle , gfal_file_handle fd, void* buff, size_t s_buff, GError** err);

off_t gfal_rfio_lseekG(plugin_handle handle , gfal_file_handle fd, off_t offset, int whence, GError** err);

int gfal_rfio_statG(plugin_handle handle, const char* name, struct stat* buff, GError ** err);

int gfal_rfio_lstatG(plugin_handle handle, const char* name, struct stat* buff, GError ** err);

gfal_file_handle gfal_rfio_opendirG(plugin_handle handle, const char* name, GError ** err);

struct dirent* gfal_rfio_readdirG(plugin_handle handle, gfal_file_handle fh , GError** err);

int gfal_rfio_closedirG(plugin_handle handle, gfal_file_handle fh, GError** err);

const char* gfal_rfio_getName();
