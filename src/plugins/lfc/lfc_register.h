#pragma once
/*
* Copyright @ Members of the EMI Collaboration, 2013.
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

#include "gfal_lfc.h"
#include "lfc_ifce_ng.h"

int gfal_lfc_register_check(plugin_handle handle, const char* src,
        const char* dst, gfal_url2_check check);

int gfal_lfc_register(plugin_handle handle, gfal2_context_t context,
        gfalt_params_t params, const char* src, const char* dst, GError**);
