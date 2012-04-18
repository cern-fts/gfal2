#pragma once
#ifndef GFAL_CPP_WRAPPER_H
#define GFAL_CPP_WRAPPER_H
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
#include <common/gfal_common_internal.h>



gfal_handle gfal_init_cpp();

plugin_handle plugin_load(gfal_handle handle, GError ** err);


void plugin_unload(plugin_handle);

#endif /* GFAL_CPP_WRAPPER_H */ 
