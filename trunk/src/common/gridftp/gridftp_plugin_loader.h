#pragma once
#ifndef GRIDFTP_PLUGIN_LOADER_H
#define GRIDFTP_PLUGIN_LOADER_H
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

#include <common/gfal_common_internal.h>
#include <common/gfal_common_filedescriptor.h>

#ifdef __cplusplus
extern "C"
{
#endif


gfal_plugin_interface gfal_plugin_init(gfal_handle handle, GError** err);

plugin_handle plugin_load(gfal_handle handle, GError ** err);

void plugin_unload(plugin_handle handle);

const char * plugin_name();

#ifdef __cplusplus
}
#endif

#endif /* GRIDFTP_PLUGIN_LOADER_H */ 
