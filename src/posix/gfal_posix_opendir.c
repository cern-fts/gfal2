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
 * @file gfal_posix_opendir.c
 * @brief file for the opendir posix func
 * @author Devresse Adrien
 * @version 2.0
 * @date 25/05/2011
 * */

#include <stdlib.h>
#include <glib.h>

#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_common_plugin.h>


#include "gfal_posix_api.h"
#include "gfal_posix_internal.h"
#include <common/gfal_common_filedescriptor.h>
#include <common/gfal_common_dir_handle.h>
#include <common/gfal_common_errverbose.h>


inline static int gfal_posix_dir_handle_store(gfal_handle handle, gfal_file_handle fhandle, GError** err){
	g_return_val_err_if_fail(handle && fhandle, 0, err, "[gfal_posix_dir_handle_store] handle invalid");
	GError* tmp_err=NULL;
	int key = 0;
	gfal_fdesc_container_handle container= gfal_dir_handle_container_instance(&(handle->fdescs), &tmp_err);
	if(container)
		key = gfal_add_new_file_desc(container, (gpointer) fhandle, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return key;
}

inline DIR* gfal_posix_internal_opendir(const char* name){
	GError* tmp_err=NULL;

	gfal_handle handle;
	gfal_file_handle ret= NULL;

	if((handle = gfal_posix_instance()) == NULL){
		errno = EIO;
		return NULL;
	}

	if(name == NULL){
		g_set_error(&tmp_err, 0, EFAULT, " name is empty");
	}else{
        ret = gfal_plugin_opendirG(handle, name, &tmp_err);
	}
	
	int key = 0;
	if(ret)
		key = gfal_posix_dir_handle_store(handle, ret, &tmp_err);
	
	if(tmp_err){
		gfal_posix_register_internal_error(handle, "[gfal_opendir]", tmp_err);
		errno = tmp_err->code;	
	}else{
		errno=0;
	}
	return GINT_TO_POINTER(key); 	
}



