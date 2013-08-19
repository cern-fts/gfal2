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
#include <stdlib.h>
#include <dirent.h>
#include <glib.h>
#include <common/gfal_common_filedescriptor.h>

typedef struct _gfal_srm_opendir_handle{
	char surl[GFAL_URL_MAX_LEN];
	char endpoint[GFAL_URL_MAX_LEN];
	off_t resu_offset;
	struct srmv2_mdfilestatus *srm_ls_resu;
	struct dirent current_readdir;
	off_t dir_offset;
}* gfal_srm_opendir_handle;

gfal_file_handle gfal_srm_opendirG(plugin_handle handle, const char* path, GError ** err);

int gfal_srm_closedirG(plugin_handle handle, gfal_file_handle fh, GError** err);
