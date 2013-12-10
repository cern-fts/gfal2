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
 * @file gfal_srm_readdir.h
 * @brief header file for the readdir function on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 14/06/2011
 * */

#include <common/gfal_common_filedescriptor.h>
#include <dirent.h>
#include <glib.h>
#include <sys/stat.h>


struct dirent* gfal_srm_readdirG(plugin_handle handle, gfal_file_handle fh, GError** err);

struct dirent* gfal_srm_readdirppG(plugin_handle ch, gfal_file_handle fh, struct stat* st, GError** err);
