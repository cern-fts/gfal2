#pragma once
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
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

/**
 * @file gfal_common_srm_access.h
 * @brief header file for the access methode on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 05/05/2011
 * */

#include <glib.h>

#include "gfal_common_srm.h"
#include "../gfal_types.h" 

int gfal_srm_accessG(plugin_handle handle, const char* surl, int mode, GError** err); 

int gfal_access_srmv2_internal(gfal_srmv2_opt* opts, char* endpoint, const char* surl, int mode,  GError** err);



