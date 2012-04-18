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
 * @file gfal_posix_closedir.c
 * @brief file for the closedir  posix func
 * @author Devresse Adrien
 * @version 2.0
 * @date 27/05/2011
 * */

#include <stdlib.h>
#include <glib.h>
#include "gfal_posix_api.h"
#include "gfal_posix_local_file.h"
#include "gfal_posix_internal.h"
#include <common/gfal_constants.h>
#include <common/gfal_prototypes.h>
#include <common/gfal_common_filedescriptor.h>
#include <common/gfal_common_dir_handle.h>
#include <common/gfal_common_errverbose.h>
#include <common/gfal_common_plugin.h>



static int gfal_posix_dir_handle_delete(gfal_fdesc_container_handle container, int key, GError** err){
	g_return_val_err_if_fail(container, -1, err, "[gfal_posix_dir_handle_delete] invalid args");
	GError *tmp_err=NULL;
	int ret = -1;
	if(container){
		ret = (gfal_remove_file_desc(container, key, &tmp_err))?0:-1;
	}
	if(tmp_err){
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	}else{
		errno = 0;
	}
	return ret;
}

static int gfal_posix_dir_handle_close(gfal_handle handle, gfal_file_handle fh, GError** err){
	g_return_val_err_if_fail(handle && fh, -1, err, "[gfal_posix_gfalfilehandle_close] invalid args");
	GError *tmp_err=NULL;
	int ret = -1;
	if( gfal_is_local_call(fh->module_name) )
		ret = gfal_local_closedir(fh, &tmp_err);
	else
		ret = gfal_plugin_closedirG(handle, fh, &tmp_err);

	if(tmp_err){
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	}
	return ret;
}

int gfal_posix_internal_closedir(DIR* d){
	GError* tmp_err=NULL;
	int ret = -1;
	gfal_handle handle;	
	if((handle = gfal_posix_instance()) == NULL){
		errno = EIO;
		return -1;
	}
	
	if(d == NULL){
		g_set_error(&tmp_err, 0, EFAULT, "File descriptor is NULL");
	}else{
		gfal_fdesc_container_handle container= gfal_dir_handle_container_instance(&(handle->fdescs), &tmp_err);	
		int key = GPOINTER_TO_INT(d);
		gfal_file_handle fh = gfal_file_handle_bind(container, key, &tmp_err);
		if( fh != NULL){
			ret = gfal_posix_dir_handle_close(handle, fh, &tmp_err);
			if(ret==0){
				ret = gfal_posix_dir_handle_delete(container, key, &tmp_err);
			}
		}
	}	
	
	if(tmp_err){
		gfal_posix_register_internal_error(handle, "[gfal_closedir]", tmp_err);
		errno = tmp_err->code;	
	}
	return ret; 		
}
