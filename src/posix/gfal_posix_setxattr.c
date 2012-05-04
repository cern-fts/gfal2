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
 * @file gfal_posix_setxattr.c
 * @brief file for the internal function of the setxattr function
 * @author Devresse Adrien
 * @date 28/09/2011
 * 
 **/
 
#include <stdio.h>
#include <errno.h>
#include <glib.h>

#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_common_plugin.h>


#include "gfal_posix_internal.h"
#include "gfal_posix_local_file.h"
 

/*
 *  set a value to a extended attribute
 * */ 
int gfal_posix_internal_setxattr (const char *path, const char *name,
			   const void *value, size_t size, int flags){
	 GError* tmp_err=NULL;
	 gfal_handle handle;
	 int res= -1;

	if((handle = gfal_posix_instance()) == NULL){
		errno = EIO;
		return -1;
	}
	
	if(path == NULL || name == NULL){
		g_set_error(&tmp_err, 0, EFAULT, " path or/and name are an incorrect arguments");
	}else{
		if( gfal_check_local_url(path, NULL) == TRUE){
			res = gfal_local_setxattr(path, name, value, size, flags, &tmp_err);
		}else{
			res = gfal_plugin_setxattrG(handle, path, name, value, size, flags, &tmp_err);;
		}

	}
	if(tmp_err){
		gfal_posix_register_internal_error(handle, "[gfal_setxattr]", tmp_err);
		errno = tmp_err->code;	
	}
	return res; 							
							
							
}
                        
                     
 
 
