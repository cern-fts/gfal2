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

#include <glib.h>
#include <string.h>
#include <stdlib.h>

#include "gfal_common_config.h"
#include "gfal_common_errverbose.h"

static const char* boolean_id = "boolean";
static const char* string_id = "string";


/*
 * gfal_common_config.c
 * Utility fonctions for config management
 * author Devresse Adrien
 * */

void gfal_config_container_init(gfal_handle  handle){
	g_return_if_fail(handle);
	
	g_datalist_init (&handle->st_config.conf);
	handle->st_config.mux = g_mutex_new();	
}


void gfal_config_container_delete(gfal_handle  handle){
	g_return_if_fail(handle);
	
	g_datalist_clear(&handle->st_config.conf);
	g_mutex_free(handle->st_config.mux);	
}

void gfal_config_set_generic(gfal_handle handle, const char* namespace, const char* key, const char* value_type, gpointer value, GDestroyNotify free_func){
	g_return_if_fail(handle && namespace && key);
	
	char* data_key = g_strjoin("#", namespace, value_type, key, NULL);
	g_datalist_id_set_data_full(&handle->st_config.conf, g_quark_from_string (data_key), GINT_TO_POINTER(value), free_func);
	g_free(data_key);
}


gpointer gfal_config_get_generic(gfal_handle handle, const char* namespace, const char* key, const char* value_type, GError** err){
	g_return_val_if_fail(handle && namespace && key, NULL);
	GError * tmp_err=NULL;
	
	char* data_key = g_strjoin("#", namespace, value_type, key, NULL);
	gpointer res = g_datalist_id_get_data (&handle->st_config.conf, g_quark_from_string (data_key));
	g_free(data_key);
	if(res == NULL)
		g_set_error(&tmp_err, 0, ENOENT, "not an existing parameter %s", key);
			
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return res;
}



void gfal_config_set_boolean(gfal_handle handle, const char* namespace, const char* key, gboolean value){
	gboolean* b = g_new(gboolean,1); // dyn allocate to avoid NULL pointer error vs false confusion
	*b = value;
	gfal_config_set_generic(handle, namespace, key, boolean_id, b, &free);
}

gboolean gfal_config_get_boolean(gfal_handle handle, const char* namespace, const char* key, GError** err){
	gboolean* res = gfal_config_get_generic(handle, namespace, key, boolean_id, err);
	return (res)?(*res):FALSE;
}



void gfal_config_set_string(gfal_handle handle, const char* namespace, const char* key, const char* value){
	gfal_config_set_generic(handle, namespace, key, string_id, strdup(value), &free);
}

char* gfal_config_get_string(gfal_handle handle, const char* namespace, const char* key, GError** err){
	char* res = (char*) gfal_config_get_generic(handle, namespace, key, string_id, err);
	if(res != NULL)
		return strdup(res);
	return NULL;
}


