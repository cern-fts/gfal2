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



// gfal_common_plugin.h
// common lib for the plugin management
// author : Devresse Adrien
 
 
 
#include <stdarg.h>
#include <glib.h>
#include <errno.h>
#include <string.h> 
#include <dirent.h>
#include <sys/stat.h>

#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_common_plugin_interface.h>


 
#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

typedef struct _plugin_pointer_handle{
	void* dlhandle;
	void* plugin_data;
	char plugin_name[GFAL_URL_MAX_LEN];
	char plugin_lib[GFAL_URL_MAX_LEN];
} *plugin_pointer_handle;

/*
 * 
 *  This API is a plugin reserved API and should be used by GFAL 2.0's plugins only
 *  Backward compatibility of this API is not guarantee
 * 
 * */


/*
 * plugins walker
 * provide a list of plugin handlers, each plugin handler is a reference to an usable plugin
 * give a NULL-ended table of plugin_pointer_handle, return NULL if error
 * */
plugin_pointer_handle gfal_plugins_list_handler(gfal_handle, GError** err);

int gfal_plugins_instance(gfal_handle, GError** err);
char** gfal_plugins_get_list(gfal_handle, GError** err);
int gfal_plugins_delete(gfal_handle, GError** err);

int gfal_plugins_has_parameter(gfal_handle handle, const char* nmespace, const char* key, GError** err);

int gfal_plugins_notify_all(gfal_handle handle, const char* nmespace, const char* key, GError** err);

#ifdef __cplusplus
}
#endif // __cplusplus
