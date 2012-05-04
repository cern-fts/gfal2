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

/*
 * @file gfal_common_srm_stat.h
 * @brief header file for the stat function on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 16/05/2011
 * */

#include <glib.h>
#include <common/gfal_types.h>

int gfal_statG_srmv2_internal(gfal_srmv2_opt* opts, struct stat* buf, const char* endpoint, const char* surl, GError** err);

int gfal_srm_statG(plugin_handle handle, const char* surl, struct stat* buf, GError** err); 



