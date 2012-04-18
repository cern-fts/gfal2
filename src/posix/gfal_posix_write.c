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
 * @file gfal_posix_write.c
 * @brief file for the internal write function for the posix interface
 * @author Devresse Adrien
 * @version 2.0
 * @date 05/07/2011
 * */
 
#include <glib.h>
#include <stdlib.h>
#include "../common/gfal_types.h"
#include "../common/gfal_common_filedescriptor.h"
#include "gfal_common_file_handle.h"
#include "gfal_posix_internal.h"
#include "../common/gfal_common_errverbose.h"
#include "../common/gfal_common_file_handle.h"
#include "../common/gfal_common_plugin.h"
#include "gfal_posix_local_file.h"


/**
 *  map the file handle to the correct call
 */ 
int gfal_posix_gfalfilehandle_write(gfal_handle handle, gfal_file_handle fh, void* buff, size_t s_buff, GError** err){
	g_return_val_err_if_fail(handle && fh, -1, err, "[gfal_posix_gfalfilehandle_write] incorrect args");
	GError *tmp_err=NULL;
	int ret = -1;
	if( gfal_is_local_call(fh->module_name) )
		ret = gfal_local_write(fh, buff, s_buff, &tmp_err);
	else
		ret = gfal_plugin_writeG(handle, fh, buff, s_buff, &tmp_err);

	if(tmp_err){
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	}
	return ret;		
}


/**
 * Implementation of the write call
 * 
 */
int gfal_posix_internal_write(int fd, void* buff, size_t s_buff){
	 GError* tmp_err=NULL;
	 gfal_handle handle;
	 int res = -1;

	if((handle = gfal_posix_instance()) == NULL){
		errno = EIO;
		return -1;
	}
	
	if(fd <=0){
		g_set_error(&tmp_err, 0, EBADF, "Incorrect file descriptor");
	}else{
		gfal_fdesc_container_handle container= gfal_file_handle_container_instance(&(handle->fdescs), &tmp_err);	
		const int key = fd;
		gfal_file_handle fh = gfal_file_handle_bind(container, key, &tmp_err);
		if( fh != NULL){
			res = gfal_posix_gfalfilehandle_write(handle, fh, buff, s_buff, &tmp_err);
		}
	}
	if(tmp_err){
		gfal_posix_register_internal_error(handle, "[gfal_write]", tmp_err);
		errno = tmp_err->code;	
	}
	return res; 	
}



/**
 *  map the file handle to the correct call for pwrite
 */ 
int gfal_posix_gfalfilehandle_pwrite(gfal_handle handle, gfal_file_handle fh, void* buff, size_t s_buff, off_t offset, GError** err){
	g_return_val_err_if_fail(handle && fh, -1, err, "[gfal_posix_gfalfilehandle_pwrite] incorrect args");
	GError *tmp_err=NULL;
	int ret = -1;
	if( gfal_is_local_call(fh->module_name) )
		ret = gfal_local_pwrite(fh, buff, s_buff, offset, &tmp_err);
	else 
		ret = gfal_plugin_pwriteG(handle, fh, buff, s_buff, offset, &tmp_err);

	if(tmp_err){
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	}
	return ret;		
}



/**
 * Implementation of the pwrite call
 * 
 */
ssize_t gfal_posix_internal_pwrite(int fd, void* buff, size_t s_buff, off_t offset){
	 GError* tmp_err=NULL;
	 gfal_handle handle;
	 int res = -1;

	if((handle = gfal_posix_instance()) == NULL){
		errno = EIO;
		return -1;
	}
	
	if(fd <=0){
		g_set_error(&tmp_err, 0, EBADF, "Incorrect file descriptor");
	}else{
		gfal_fdesc_container_handle container= gfal_file_handle_container_instance(&(handle->fdescs), &tmp_err);	
		const int key = fd;
		gfal_file_handle fh = gfal_file_handle_bind(container, key, &tmp_err);
		if( fh != NULL){
			res = gfal_posix_gfalfilehandle_pwrite(handle, fh, buff, s_buff, offset, &tmp_err);
		}
	}
	if(tmp_err){
		gfal_posix_register_internal_error(handle, "[gfal_pwrite]", tmp_err);
		errno = tmp_err->code;	
	}
	return res; 	
}
