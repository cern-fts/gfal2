#pragma once
/*
* Copyright @ Members of the EMI Collaboration, 2010.
* See www.eu-emi.eu for details on the copyright holders.
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

/*
 * @file gfal_srm_opendir.h
 * @brief header file for the opendir function on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 09/06/2011
 * */
#include <gfal_srm_ifce_types.h>
#include <stdint.h>
#include <stdlib.h>
#include <dirent.h>
#include <glib.h>
#include <common/gfal_common_filedescriptor.h>

typedef struct _gfal_srm_opendir_handle {
    // SURL we are listing
    char surl[GFAL_URL_MAX_LEN];

    // Buffer where to store read entries, and returned by readdir calls
    struct dirent dirent_buffer;

    // Will be set to 1 by gfal_srm_readdirppG if the directory was too big
    // and we decided to read in chunks
    int is_chunked_listing;

    // These two are used internally in chunk listing, to keep track
    int chunk_offset;
    int chunk_size;

    // Array of file statuses as returned by srm-ifce
    struct srmv2_mdfilestatus *srm_file_statuses;
    // Array position inside srm_file_statuses while iterating
    int response_index;
}* gfal_srm_opendir_handle;

gfal_file_handle gfal_srm_opendirG(plugin_handle handle, const char* path, GError ** err);

int gfal_srm_closedirG(plugin_handle handle, gfal_file_handle fh, GError** err);

struct dirent* gfal_srm_readdirG(plugin_handle handle, gfal_file_handle fh, GError** err);

struct dirent* gfal_srm_readdirppG(plugin_handle ch, gfal_file_handle fh, struct stat* st, GError** err);
