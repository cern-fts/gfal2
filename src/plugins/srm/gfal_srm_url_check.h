#pragma once
#ifndef GFAL_COMMON_SRM_URL_CHECK_H
#define GFAL_COMMON_SRM_URL_CHECK_H
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


#include "gfal_srm.h"

gboolean srm_check_url(const char * surl);

/*
 * 
 * implementation of the plugi_url_transfer_check for the srm plugin
 * Check if the srm plugin is able to handle a given type of URL.
 * 
 * */

gboolean plugin_url_check2(plugin_handle handle, const char* src, const char* dst, gfal_url2_check type );


#endif /* GFAL_COMMON_SRM_URL_CHECK_H */ 
