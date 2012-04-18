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
 * @file gfal_common_parameter.c
 * @brief file for internal settings set/get
 * @author Devresse Adrien
 * @version 2.0
 * @date 02/10/2011
 * */


#include "gfal_common_parameter.h"
#include "gfal_common_plugin.h"
#include "gfal_common_errverbose.h"
#include "gfal_common_config.h"
#include "mds/gfal_common_mds.h"

typedef struct _gfal_internal_parameter{
	char key[GFAL_URL_MAX_LEN];
} gfal_internal_parameter;

// list of core parameters
static gfal_internal_parameter _gfal_core_parameters[] = { { "no_bdii"}, {"infosys"}, {"global_conn_timeout"}, {"conf_version"}, {"plugin_list"} };
static int _gfal_core_parameters_number = sizeof(_gfal_core_parameters)/sizeof(gfal_internal_parameter);

static char* gfal_parameter_core_namespace = "core";

static int gfal_common_core_parameter_lookup(const char* key){
	int i;
	for(i=0; i < _gfal_core_parameters_number; ++i)
		if(strcmp(_gfal_core_parameters[i].key, key) == 0)
			return 1;
	return 0;
}



static int gfal_common_notify_core(gfal_handle handle, const char* namespace, const char* key, GError** err){
	GError* tmp_err=NULL;
	int ret = -1;
	if( strcmp(namespace, gfal_parameter_core_namespace) == 0 && strcmp(key, _gfal_core_parameters[0].key) ==0 ){ // no bdii
		gboolean b = gfal_common_parameter_get_boolean(handle, namespace, key, &tmp_err);
		if(!tmp_err){
			gfal_set_nobdiiG(handle, b);
			ret = 0;
		}
	} else if( strcmp(namespace, gfal_parameter_core_namespace) == 0 && strcmp(key, _gfal_core_parameters[1].key) ==0 ){ // infosys
		char* infosys = gfal_common_parameter_get_string(handle, namespace, key, &tmp_err);
		if(!tmp_err){
			gfal_mds_set_infosys(handle, infosys, &tmp_err);
			ret = 0;
			g_free(infosys);
		}	
	}else
		ret = 0;
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;
}


/**
 * setter for string value
 * */
int gfal_common_parameter_set_string(gfal_handle handle, const char* namespace, const char* key, const char* value, GError** err){
	g_mutex_lock(handle->st_config.mux);
	GError* tmp_err=NULL;
	int ret = -1;
	if( ( gfal_common_core_parameter_lookup(key) == 1)
		|| (gfal_plugins_has_parameter(handle, namespace, key, &tmp_err) ==1))
	{
		gfal_config_set_string(handle, namespace, key, value);
		ret =0;
		
	}else if(!tmp_err){ // NO keys found
		g_set_error(&tmp_err, 0, ENOENT, "%s is not a valid parameter to set", key);
	}	
	g_mutex_unlock(handle->st_config.mux);	
	if(ret ==0)
		ret= gfal_common_notify_core(handle, namespace, key, &tmp_err);
	if(ret == 0)
		ret= gfal_plugins_notify_all(handle, namespace, key, &tmp_err);
	
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;	
}

/**
 * getter for string value
 */
char* gfal_common_parameter_get_string(gfal_handle handle, const char* namespace, const char* key, GError** err){
	g_mutex_lock(handle->st_config.mux);
	GError* tmp_err=NULL;
	char* ret = gfal_config_get_string(handle, namespace, key, &tmp_err);
	
	g_mutex_unlock(handle->st_config.mux);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;
}



gboolean gfal_common_parameter_get_boolean(gfal_handle handle, const char* namespace, const char* key, GError** err){
	g_mutex_lock(handle->st_config.mux);
	GError* tmp_err=NULL;
	gboolean b = gfal_config_get_boolean(handle, namespace, key, &tmp_err);
	
	g_mutex_unlock(handle->st_config.mux);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return b;
}


int gfal_common_parameter_set_boolean(gfal_handle handle, const char* namespace, const char* key, gboolean value, GError** err){
	g_mutex_lock(handle->st_config.mux);
	GError* tmp_err=NULL;
	int ret = -1;
	if( (gfal_common_core_parameter_lookup(key) == 1)
		||  (gfal_plugins_has_parameter(handle, namespace, key, &tmp_err) ==1))
	{
		gfal_config_set_boolean(handle, namespace, key, value);
		ret =0;
		
	}	
	g_mutex_unlock(handle->st_config.mux);	
	if(ret ==0)
		ret= gfal_common_notify_core(handle, namespace, key, &tmp_err);
	if(ret == 0)
		ret = gfal_plugins_notify_all(handle, namespace, key, &tmp_err);
	
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;	
	
}


