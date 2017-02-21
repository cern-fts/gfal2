/*
 * Copyright (c) CERN 2013-2015
 *
 * Copyright (c) Members of the EMI Collaboration. 2010-2013
 *  See  http://www.eu-emi.eu/partners for details on the copyright
 *  holders.
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

#pragma once
#ifndef GFAL_TYPES_H_
#define GFAL_TYPES_H_

#if !defined(__GFAL2_H_INSIDE__) && !defined(__GFAL2_BUILD__)
#   warning "Direct inclusion of gfal2 headers is deprecated. Please, include only gfal_api.h or gfal_plugins_api.h"
#endif

#include "gfal_plugin_interface.h"
#include <g_config_manager/g_config_manager.h>

/* enforce proper calling convention */
#ifdef __cplusplus
extern "C"
{
#endif


struct _gfal_plugin_opts {
    gfal_plugin_interface plugin_list[MAX_PLUGIN_LIST];
    GList* sorted_plugin;
    int plugin_number;
};
typedef struct _gfal_plugin_opts gfal_plugin_opts;


struct gfal_handle_ {
	gboolean initiated;
	// struct of the plugin opts
    gfal_plugin_opts plugin_opt;
	//struct for the file descriptors
	gfal_file_handle_container fdescs;
	GConfigManager_t conf;
    // cancel logic
    volatile gint running_ops;
    gboolean cancel;
    GMutex* mux_cancel;
    GHookList cancel_hooks;

	// Credential mapping
    GList *cred_mapping;

    // client information
    char* agent_name;
    char* agent_version;
    GPtrArray* client_info;
};


#ifdef __cplusplus
}
#endif

#endif
