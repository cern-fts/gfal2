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
 * @file gfal_posix_readdir.c
 * @brief file for the readdir posix func
 * @author Devresse Adrien
 * @version 2.0
 * @date 27/05/2011
 * */

#include <stdlib.h>
#include <glib.h>
#include <dirent.h>

#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_common_plugin.h>


#include "gfal_posix_api.h"
#include "gfal_posix_internal.h"
#include "../common/gfal_common_filedescriptor.h"
#include "../common/gfal_common_dir_handle.h"
#include "../common/gfal_common_errverbose.h"
#include "gfal_posix_local_file.h"


/**
 *  map the file handle to the correct call
 */ 
inline static struct dirent* gfal_posix_gfalfilehandle_readdir(gfal_handle handle, gfal_file_handle fh, GError** err){
	g_return_val_err_if_fail(handle && fh, NULL, err, "[gfal_posix_gfalfilehandle_readdir] incorrect args");
	GError *tmp_err=NULL;
	struct dirent* ret = NULL;
	if( gfal_is_local_call(fh->module_name) )
		ret = gfal_local_readdir(fh, &tmp_err);
	else
		ret = gfal_plugin_readdirG(handle, fh, &tmp_err);

	if(tmp_err){
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	}
	return ret;		
}

/**
 * Implementation of the readdir functions
 * 
 */
inline struct dirent* gfal_posix_internal_readdir(DIR* dir){
	 GError* tmp_err=NULL;
	 gfal_handle handle;
	 struct dirent* res= NULL;

	if((handle = gfal_posix_instance()) == NULL){
		errno = EIO;
		return NULL;
	}
	
	if(dir == NULL){
		g_set_error(&tmp_err, 0, EBADF, "Incorrect file descriptor");
	}else{
		gfal_fdesc_container_handle container= gfal_dir_handle_container_instance(&(handle->fdescs), &tmp_err);	
		const int key = GPOINTER_TO_INT(dir);
		gfal_file_handle fh = gfal_file_handle_bind(container, key, &tmp_err);
		if( fh != NULL){
			res = gfal_posix_gfalfilehandle_readdir(handle, fh, &tmp_err);
		}
	}
	if(tmp_err){
		gfal_posix_register_internal_error(handle, "[gfal_readdir]", tmp_err);
		errno = tmp_err->code;	
	}
	return res; 	
}


