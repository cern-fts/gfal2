#pragma once
#ifndef _GFAL2_TRANSFER_INTERNAL_
#define _GFAL2_TRANSFER_INTERNAL_

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

#include <string>

#include "gfal_transfer.h"
#include "gfal_transfer_types.h"

extern "C" void gfalt_params_handle_init(gfalt_params_t  handle, GError ** err);

void perform_local_copy(gfal2_context_t context, gfalt_params_t params,
        const std::string & src, const std::string & dst);

#endif //_GFAL2_TRANSFER_INTERNAL_
