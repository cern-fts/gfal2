#pragma once
#ifndef _GFAL_COMMON_PARAMETER_H
#define _GFAL_COMMON_PARAMETER_H
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
 * @file gfal_common_parameter.h
 * @brief header for internal settings set/get
 * @author Devresse Adrien
 * @version 2.0
 * @date 02/10/2011
 * */

#include <stdlib.h>
#include <glib.h>

#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * set the value of a GFAL 2.0 string parameter
 * @param namespace of the parameter, NULL for the core parameters, equal to the plugin_name for the plugin specific parameter
 * @param key : key of the parameter to set
 * @param value : value of the parameter to set
 * @param err : GError report
 * */
int gfal_common_parameter_set_string(gfal_handle handle, const char* nmespace, const char* key, const char* value, GError** err);


/**
 * get a GFAL 2.0 the value string parameter
 * @param namespace of the parameter, NULL for the core parameters, equal to the plugin_name for the plugin specific parameter
 * @param key : key of the parameter to set
 * @param value : value of the parameter to set
 * @param err : GError report
 * */
char* gfal_common_parameter_get_string(gfal_handle handle, const char* nmespace, const char* key, GError** err);


gboolean gfal_common_parameter_get_boolean(gfal_handle handle, const char* nmespace, const char* key, GError** err);


int gfal_common_parameter_set_boolean(gfal_handle handle, const char* nmespace, const char* key, gboolean value, GError** err);




#ifdef __cplusplus
}
#endif



#endif //_GFAL_COMMON_PARAMETER_H

