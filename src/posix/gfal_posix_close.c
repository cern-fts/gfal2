/* 
* Copyright @ Members of the EMI Collaboration, 2010.
* See www.eu-emi.eu for details on the copyright holders.
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


/*
 * @file gfal_posix_close.c
 * @brief file for the internal close function for the posix interface
 * @author Devresse Adrien
 * @version 2.0
 * @date 31/05/2011
 * */

#include <glib.h>
#include <errno.h>


#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_common_plugin.h>
#include <common/gfal_common_filedescriptor.h>
#include <common/gfal_common_errverbose.h>
#include <common/gfal_common_file_handle.h>


#include "gfal_posix_internal.h"

 


static int gfal_posix_file_handle_delete(gfal_fdesc_container_handle container, int key, GError** err){
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
 
static int gfal_posix_file_handle_close(gfal_handle handle, gfal_file_handle fhandle, GError ** err){
	g_return_val_err_if_fail(handle && fhandle, -1, err, "[gfal_posix_file_handle_close] invalid args");
	GError* tmp_err=NULL;
	int ret = -1;

    ret = gfal_plugin_closeG(handle, fhandle, &tmp_err);
		
	if(tmp_err){
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	}
	return ret;
	
} 

int gfal_posix_internal_close(int fd){
	GError* tmp_err=NULL;
	int ret = -1;
	gfal_handle handle;	
	if((handle = gfal_posix_instance()) == NULL){
		errno = EIO;
		return -1;
	}
	
	if(fd == 0){
		g_set_error(&tmp_err, 0, EFAULT, "File descriptor is NULL");
	}else{
		gfal_fdesc_container_handle container= gfal_file_handle_container_instance(&(handle->fdescs), &tmp_err);	
		int key = GPOINTER_TO_INT(fd);
		gfal_file_handle fh = gfal_file_handle_bind(container, key, &tmp_err);
		if( fh != NULL){
			ret = gfal_posix_file_handle_close(handle, fh, &tmp_err);
			if(ret==0){
				ret = gfal_posix_file_handle_delete(container, key, &tmp_err);
			}
		}		
	}
	if(tmp_err){
		gfal_posix_register_internal_error(handle, "[gfal_close]", tmp_err);
		errno = tmp_err->code;
	}
	return ret;
}
