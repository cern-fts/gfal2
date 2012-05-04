#pragma once
#ifndef GRIDFTP_OPENDIR_MODULE_H
#define GRIDFTP_OPENDIR_MODULE_H
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

#include "gridftpmodule.h"

extern "C" gfal_file_handle gfal_gridftp_opendirG(plugin_handle handle , const char* path,  GError** err);

extern "C" struct dirent* gfal_gridftp_readdirG(plugin_handle handle, gfal_file_handle fh, GError** err);

extern "C" int gfal_gridftp_closedirG(plugin_handle handle, gfal_file_handle fh, GError** err);

#endif /* GRIDFTP_OPENDIR_MODULE_H */ 
