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
 * @file gfal_posix_parameters.c
 * @brief new file for the parameters management
 * @author Devresse Adrien
 * @date 02/10/2011
 * */

#include <glib.h>
#include <stdlib.h>

#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_common_plugin.h>

#include <common/gfal_common_errverbose.h>
#include <common/gfal_common_parameter.h>

#include "gfal_posix_internal.h"


/*
 * Internal wrapper for posix api to set parameter boolean
 * */
int gfal_set_parameter_boolean_internal(const char* namespace, const char* key, int value){
	GError* tmp_err=NULL;
	gfal_handle handle;

	int ret = -1;
	
	gfal_log(GFAL_VERBOSE_TRACE, "%s ->",__func__);

	if((handle = gfal_posix_instance()) == NULL){
		errno = EIO;
		return -1;
	}
	if(namespace == NULL || key == NULL){
		g_set_error(&tmp_err, 0, EINVAL, "Invalid namespace/key value");
	}else {
		ret = gfal_common_parameter_set_boolean(handle, namespace, key, value, &tmp_err);
	}
	
	if(tmp_err){
		gfal_posix_register_internal_error(handle, "[gfal_set_parameter_boolean]", tmp_err);
	}else
		errno=0;
	gfal_log(GFAL_VERBOSE_TRACE, "%s <-",__func__);
	return ret;	
}

/*
 * Internal wrapper for posix api to set parameter string
 * */
int gfal_set_parameter_string_internal(const char* namespace, const char* key, const char* value){
	GError* tmp_err=NULL;
	gfal_handle handle;

	int ret = -1;
	
	gfal_log(GFAL_VERBOSE_TRACE, "%s ->",__func__);

	if((handle = gfal_posix_instance()) == NULL){
		errno = EIO;
		return -1;
	}
	if(namespace == NULL || key == NULL){
		g_set_error(&tmp_err, 0, EINVAL, "Invalid key value");
	}else {
		ret = gfal_common_parameter_set_string(handle, namespace, key, value, &tmp_err);
	}
	
	if(tmp_err){
		gfal_posix_register_internal_error(handle, "[gfal_set_parameter_string]", tmp_err);
	}else
		errno=0;
	gfal_log(GFAL_VERBOSE_TRACE, "%s <-",__func__);
	return ret;	
}


/*
 * Internal wrapper for posix api to set parameter string
 * */
char* gfal_get_parameter_string_internal(const char* namespace, const char* key){
	GError* tmp_err=NULL;
	gfal_handle handle;

	char* ret = NULL;
	
	gfal_log(GFAL_VERBOSE_TRACE, "%s ->",__func__);

	if((handle = gfal_posix_instance()) == NULL){
		errno = EIO;
		return NULL;
	}
	if(namespace == NULL || key == NULL){
		g_set_error(&tmp_err, 0, EINVAL, "Invalid key value");
	}else {
		ret = gfal_common_parameter_get_string(handle, namespace, key, &tmp_err);
	}
	
	if(tmp_err){
		gfal_posix_register_internal_error(handle, "[gfal_get_parameter_string]", tmp_err);
	}else
		errno=0;
	gfal_log(GFAL_VERBOSE_TRACE, "%s <-",__func__);
	return ret;	
}


/*
 * Internal wrapper for posix api to get parameter boolean
 * */
int gfal_get_parameter_boolean_internal(const char* namespace, const char* key){
	GError* tmp_err=NULL;
	gfal_handle handle;

	int ret = -1;
	
	gfal_log(GFAL_VERBOSE_TRACE, "%s ->",__func__);

	if((handle = gfal_posix_instance()) == NULL){
		errno = EIO;
		return -1;
	}
	
	if(namespace == NULL || key == NULL){
		g_set_error(&tmp_err, 0, EINVAL, "Invalid key value");
	}else {
		ret = gfal_common_parameter_get_boolean(handle, namespace, key, &tmp_err);
	}
	
	if(tmp_err){
		gfal_posix_register_internal_error(handle, "[gfal_get_parameter_boolean]", tmp_err);
		ret =-1;
	}else
		errno=0;
	gfal_log(GFAL_VERBOSE_TRACE, "%s <-",__func__);
	return ret;	
}
