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

#pragma once

#include <regex.h>
#include <time.h>

#include "gfal_srm.h"


gfal_file_handle gfal_srm_openG(plugin_handle, const char* path, int flag, mode_t mode, GError**);

ssize_t gfal_srm_readG(plugin_handle, gfal_file_handle fd, void* buff, size_t count, GError**);

ssize_t gfal_srm_preadG(plugin_handle ch, gfal_file_handle fd, void* buff, size_t count, off_t offset, GError** err);

ssize_t gfal_srm_writeG(plugin_handle, gfal_file_handle fd, const void* buff, size_t count, GError**);

int gfal_srm_closeG(plugin_handle, gfal_file_handle fd, GError **);

/*
 * lseek function for the srm  plugin
 */
off_t gfal_srm_lseekG(plugin_handle ch, gfal_file_handle fd, off_t offset, int whence, GError** err);


