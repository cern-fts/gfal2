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
 * gfal_common_filedescriptor.c
 * @file for the file descriptor management
 * author Devresse Adrien
 * */

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include "gfal_common_errverbose.h"
#include "gfal_types.h"
#include "gfal_common_filedescriptor.h"


// generate a new unique key
static int gfal_file_key_generatorG(gfal_fdesc_container_handle fhandle, GError** err){
	g_return_val_err_if_fail(fhandle, 0, err, "[gfal_file_descriptor_generatorG] Invalid  arg file handle");
	int ret= rand();
	GHashTable* c = fhandle->container;
	if(g_hash_table_size(c) > G_MAXINT/2 ){
		g_set_error(err, 0, EMFILE, " [%s] too many files open", __func__);
		ret = 0;
	}else {
		while(ret ==0 || g_hash_table_lookup(c, GINT_TO_POINTER(ret)) != NULL){
			ret = rand();
		}
	}
	return ret;
}

/*
 * Add the given file handle to the and return a file descriptor
 * return the associated key if success else 0 and set err 
 */
int gfal_add_new_file_desc(gfal_fdesc_container_handle fhandle, gpointer pfile, GError** err){
	g_return_val_err_if_fail(fhandle && pfile, 0, err, "[gfal_add_new_file_desc] Invalid  arg fhandle and/or pfile");
	pthread_mutex_lock(&(fhandle->m_container));
	GError* tmp_err=NULL;
	GHashTable* c = fhandle->container;
	int key = gfal_file_key_generatorG(fhandle, &tmp_err);
	if(key !=0){
		g_hash_table_insert(c, GINT_TO_POINTER(key), pfile);
	} 
	if(tmp_err){
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	}
	pthread_mutex_unlock(&(fhandle->m_container));
	return key;
}

//return the associated file handle for the given file descriptor or NULL if the key is not present and err is set
gpointer gfal_get_file_desc(gfal_fdesc_container_handle fhandle, int key, GError** err){
	pthread_mutex_lock(&(fhandle->m_container));
	GHashTable* c = fhandle->container;	
	gpointer p =  g_hash_table_lookup(c, GINT_TO_POINTER(key));
	if(!p)
		g_set_error(err,0, EBADF, "[%s] bad file descriptor",__func__);
	pthread_mutex_unlock(&(fhandle->m_container));
	return p;
}

// remove the associated file handle associated with the given file descriptor
// return true if success else false
gboolean gfal_remove_file_desc(gfal_fdesc_container_handle fhandle, int key, GError** err){
	pthread_mutex_lock(&(fhandle->m_container));
	GHashTable* c = fhandle->container;	
	gboolean p =  g_hash_table_remove(c, GINT_TO_POINTER(key));
	if(!p)
		g_set_error(err,0, EBADF, "[%s] bad file descriptor",__func__);
	pthread_mutex_unlock(&(fhandle->m_container));
	return p;	  
 }
 
 

//create a new file descriptor container with the given destroyer function to an element of the container
 gfal_fdesc_container_handle gfal_file_descriptor_handle_create(GDestroyNotify destroyer){
	  gfal_fdesc_container_handle d = calloc(1, sizeof(struct _gfal_file_descriptor_container));
	  d->container = g_hash_table_new_full(NULL, NULL, NULL, destroyer);
	  pthread_mutex_init(&(d->m_container),NULL); 
	  return d;	 
 }
 
 
 /*
 *  Create a new gfal_file_handle
 *  a gfal_handle is a rich file handle with additional information for the internal purpose
 *  @param id_module : id of the module which create the handle ( <10 -> gfal internal module, >=10 : plugin, plugin, external module ( ex : lfc )
 *  @param fdesc : original descriptor
 *  @warning need to be free manually
 * */
gfal_file_handle gfal_file_handle_new(const char*  module_name, gpointer fdesc){
	gfal_file_handle f = g_new(struct _gfal_file_handle_,1);
	g_strlcpy(f->module_name, module_name, GFAL_MODULE_NAME_SIZE);
	f->lock = g_mutex_new ();
	f->offset = 0;
	f->fdesc = fdesc;
	f->ext_data = NULL;
	return f;
}

/*
 * same than gfal_file_handle but with external data storage support
 */
gfal_file_handle gfal_file_handle_ext_new(const char*  module_name, gpointer fdesc, gpointer ext_data){
	gfal_file_handle f = gfal_file_handle_new(module_name, fdesc);
	f->ext_data = ext_data;
	return f;
}
 

/**
* return the file descriptor of this gfal file handle
*/
gpointer gfal_file_handle_get_fdesc(gfal_file_handle fh){
	return fh->fdesc;
}

/**
* return the user data of this gfal file descriptor
*/
gpointer gfal_file_handle_get_user_data(gfal_file_handle fh){
	return fh->ext_data;
}

 /*
 * 
 * return the file handle associated with the file_desc
 * @warning does not free the handle
 * 
 * */
gfal_file_handle gfal_file_handle_bind(gfal_fdesc_container_handle h, int file_desc, GError** err){
	g_return_val_err_if_fail(file_desc, 0, err, "[gfal_dir_handle_bind] invalid dir descriptor");
	GError* tmp_err = NULL;
	gfal_file_handle resu=NULL;
	resu = gfal_get_file_desc(h, file_desc, &tmp_err);		
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return resu;
}


void gfal_file_handle_lock(gfal_file_handle fh){
	g_assert(fh);
	g_mutex_lock(fh->lock);
}

void gfal_file_handle_unlock(gfal_file_handle fh){
	g_assert(fh);
	g_mutex_unlock(fh->lock);
}


//  Delete a gfal_file handle 
void gfal_file_handle_delete(gfal_file_handle fh){
	if(fh){
		g_mutex_free(fh->lock);	
		free(fh);
	}
}



