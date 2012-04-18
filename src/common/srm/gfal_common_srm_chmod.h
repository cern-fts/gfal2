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
 * @file gfal_common_srm_chmod.c
 * @brief file for the change permission management
 * @author Devresse Adrien
 * @date 06/07/2011
 * */
 
 #define _GNU_SOURCE 

#include <regex.h>
#include <time.h> 
 
#include "gfal_common_srm.h"
#include "../gfal_common_internal.h"
#include "../gfal_common_errverbose.h"
#include "../gfal_common_plugin.h"
#include "gfal_common_srm_internal_layer.h"



int	gfal_srm_chmodG(plugin_handle, const char *, mode_t, GError** err);
