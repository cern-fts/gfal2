#pragma once
#ifndef GRIDFTP_PLUGIN_MAIN_H
#define GRIDFTP_PLUGIN_MAIN_H

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
#include <glib.h>
#include <common/gfal_constants.h>
#include <common/gfal_types.h>

#ifdef __cplusplus
extern "C"
{
#endif


gboolean plugin_url_check2(plugin_handle handle, const char* src, const char* dst, gfal_url2_check type );

int plugin_url_check_with_gerror(plugin_handle handle, const char* src, plugin_mode check, GError ** err);

int plugin_url_check(plugin_handle, const char* src, plugin_mode check);


#ifdef __cplusplus
}
#endif

#endif /* GRIDFTP_PLUGIN_MAIN_H */ 
