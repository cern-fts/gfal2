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
 * @file gfal_common_config.h
 * @author Devresse Adrien
 * @version 1.0
 * @date 04/11/2011
 * */

#include "gfal_prototypes.h"
#include "gfal_types.h"

void gfal_config_container_init(gfal_handle  handle);

void gfal_config_container_delete(gfal_handle  handle);


void gfal_config_set_boolean(gfal_handle handle, const char* namespace, const char* key, gboolean value);

gboolean gfal_config_get_boolean(gfal_handle handle, const char* namespace, const char* key, GError** err);


void gfal_config_set_string(gfal_handle handle, const char* namespace, const char* key, const char* value);


char* gfal_config_get_string(gfal_handle handle, const char* namespace, const char* key, GError** err);
