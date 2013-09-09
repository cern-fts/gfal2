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
 * @file gfal_srm_endpoint.h
 * @brief header file for internal use in the srm part, endpoint determination
 * @author Devresse Adrien
 * @version 2.0
 * @date 22/06/2011
 * */

#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include "gfal_srm.h"
#include <common/gfal_constants.h>
#include <stdlib.h>

int gfal_srm_determine_endpoint(gfal_srmv2_opt* opts, const char* surl, char* buff_endpoint, size_t s_buff, enum gfal_srm_proto* srm_type, GError** err);
