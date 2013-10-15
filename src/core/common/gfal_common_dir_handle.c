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
 * gfal_common_dir_handle.c
 * file for the directory handle management
 * author Devresse Adrien
 * */

#include <stdlib.h>
#include <errno.h>
#include <glib.h>
#include <pthread.h>
#include "gfal_constants.h" 
#include "gfal_types.h"
#include "gfal_common_filedescriptor.h"
#include "gfal_common_dir_handle.h"



pthread_mutex_t m_dir_container =PTHREAD_MUTEX_INITIALIZER;


// return the singleton of the file descriptor container for the directories
gfal_fdesc_container_handle gfal_dir_handle_container_instance(gfal_descriptors_container* fdescs, GError** err){
	gfal_fdesc_container_handle dir_handle = fdescs->dir_container;
	if(dir_handle == NULL){
		pthread_mutex_lock(&m_dir_container);
		if(fdescs->dir_container == NULL){
			dir_handle = fdescs->dir_container = gfal_file_descriptor_handle_create(NULL);
			if(!dir_handle)
                g_set_error(err, gfal2_get_core_quark(), EIO, "[%s] Error while init directories file descriptor container", __func__);
		}
		pthread_mutex_unlock(&m_dir_container);
	}
	return dir_handle;	
}

void gfal_dir_handle_container_delete(gfal_descriptors_container* fdescs){
	pthread_mutex_lock(&m_dir_container);
	free(fdescs->dir_container);
	fdescs->dir_container = NULL;
	pthread_mutex_unlock(&m_dir_container);
}



