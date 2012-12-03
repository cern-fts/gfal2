#pragma once
#ifndef GFAL_SRM_REQUEST_H
#define GFAL_SRM_REQUEST_H
/*
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

#include <glib.h>
#include "gfal_common_srm.h"

/* 
 * Next gen request srm system for gfal 2.0
 * Come with srm-ifce 2.0
 * */

gfal_srm_params_t gfal_srm_params_new(gfal_srm_plugin_t handle, GError ** err );

void gfal_srm_params_free(gfal_srm_params_t params);

char ** gfal_srm_params_get_protocols(gfal_srm_params_t params);

void gfal_srm_params_set_protocols(gfal_srm_params_t params, char** protocols);

gchar* gfal_srm_params_get_spacetoken(gfal_srm_params_t params);

void gfal_srm_params_set_spacetoken(gfal_srm_params_t params, const char* spacetoken);


void gfal_srm_params_set_size(gfal_srm_params_t params, size_t file_size);


#endif /* GFAL_SRM_REQUEST_H */ 
