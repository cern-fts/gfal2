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
 * gfal_common_all.c
 * core file for the utility function of gfal common part
 * author Devresse Adrien
 * */

#include <dlfcn.h>
#include <regex.h>
#include <stdlib.h>



#include "gfal_prototypes.h"
#include "gfal_types.h"
#include "gfal_common_plugin.h"
#include "gfal_common_errverbose.h"
#include "gfal_common_dir_handle.h"
#include "gfal_common_file_handle.h"
#include "gfal_common_config.h"

#define XVERSION_STR(x) #x
#define VERSION_STR(x) XVERSION_STR(x)

/* the version should be set by a "define" at the makefile level */
static const char *gfalversion = VERSION_STR(VERSION);

 /*
 * initiate a gfal's context with default parameters for use
 */
gfal_handle gfal_initG (GError** err)
{
	GError* tmp_err=NULL;
	gfal_handle handle = g_new0(struct gfal_handle_,1);// clear allocation of the struct and set defautl options
	if(handle == NULL){
		errno= ENOMEM;
		g_set_error(err,0,ENOMEM, "[gfal_initG] bad allocation, no more memory free");
		return NULL;
	}
	if(!g_thread_supported ())
		g_thread_init(NULL);
	handle->no_bdii_check=FALSE;
	handle->plugin_opt.plugin_number= 0;
	
	gfal_plugins_instance(handle, &tmp_err); // load and instanciate all the plugins
	if(tmp_err){
		g_free(handle);
		handle = NULL;	
	}else{
		
	}
	
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return handle;
}

//free a gfal's handle, safe if null
void gfal_handle_freeG (gfal_handle handle){
	if(handle == NULL)
		return;
	gfal_plugins_delete(handle, NULL);
	gfal_dir_handle_container_delete(&(handle->fdescs));
	
	g_free(handle);
	handle = NULL;
}


/*
 * convert glist of surl char* to char**
 * return NULL if error or pointer to char**
 */
char** gfal_GList_to_tab(GList* surls){
	int surl_size = g_list_length(surls);
	int i;
	char **resu = surl_size?((char**)calloc(surl_size+1, sizeof(char*))):NULL;
	for(i=0;i<surl_size; ++i){
		resu[i]= surls->data;
		surls = g_list_next(surls);
	}
	return resu;
}

//convert glist of int to a table of int
int* gfal_GList_to_tab_int(GList* int_list){
	int int_size = g_list_length(int_list);
	int i;
	int *resu = int_size?((int*)calloc(int_size+1, sizeof(int))):NULL;
	for(i=0;i<int_size; ++i){
		resu[i]= GPOINTER_TO_INT(int_list->data);
		int_list = g_list_next(int_list);
	}
	return resu;
}


/*
 *  open dynamycally a list of symbols
 *  resolve all symbols from sym_list to flist with the associated dlhandle
 *  set GError properly if error
 *  
 * */
int resolve_dlsym_listG(void* handle, void*** flist, const char** sym_list, int num, GError** err){
	if(num >0){
		void* sym = dlsym(handle, *sym_list);
		if(!sym){
			g_set_error(err,0, EBADR, "[resolve_dlsym_listG] Unable to resolve symbol %s, dlsym Error : %s ",*sym_list, dlerror());
			return -1;
		}
		**flist= sym;
		return resolve_dlsym_listG(handle, flist+1, sym_list+1, num-1, err);
	}else 
		return num;
	
}

//return a string of the current gfal version
char *gfal_version(){
    return (char*) gfalversion;
}



//check the validity of a result for a "access" standard call
 gboolean is_valid_access_result(int status){
	 switch(status){
		 case 0:
		 case ENOENT:
		 case EACCES:
		 case ELOOP:
		 case ENAMETOOLONG:
		 case ENOTDIR:
		 case EROFS:{
			return TRUE;
		 }
		 default:{
			return FALSE;		 
		}
	}
	 
}


 

 


