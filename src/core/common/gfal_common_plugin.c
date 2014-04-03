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
 * gfal_common_plugin.c
 * common part for the plugin management
 * author : Devresse Adrien
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <logger/gfal_logger.h>
#include "gfal_types.h"
#include "gfal_common_plugin.h"
#include "gfal_constants.h"
#include "gfal_common_err_helpers.h"
#include "gfal_common_filedescriptor.h"

#ifndef GFAL_PLUGIN_DIR_DEFAULT
#error "GFAL_PLUGIN_DIR_DEFAULT should be define at compile time"
#endif





/*
 * function to use in order to create a new plugin interface
 *  permit to keep the ABI compatibility
 *  must be use in ALL the plugin's "gfal_plugin_init" functions
 */
gfal_plugin_interface* gfal_plugin_interface_new(){
    size_t s_plugin = sizeof(gfal_plugin_interface)+1;
	gfal_plugin_interface* ret = malloc( s_plugin); // bigger allocation that needed for futur extensions
	memset(ret, 0,s_plugin);
	return ret;
}

plugin_handle gfal_get_plugin_handle(gfal_plugin_interface* cata_list){
    return cata_list->plugin_data;
}

gboolean gfal_feature_is_supported(void * ptr, GQuark scope, const char* func_name, GError** err){
    if(ptr == NULL){
       g_set_error(err, gfal2_get_plugins_quark(),EPROTONOSUPPORT, "[%s] Protocol not supported or path/url invalid", func_name);
       return FALSE;
    }
    return TRUE;
}

//convenience function for safe calls to the plugin checkers
gboolean gfal_plugin_checker_safe(gfal_plugin_interface* cata_list,
        const char* path, plugin_mode call_type, GError** terr)
{
    if (cata_list->check_plugin_url)
        return cata_list->check_plugin_url(cata_list->plugin_data, path, call_type, terr);
    else {
        gfal2_set_error(terr, gfal2_get_plugins_quark(), EPROTONOSUPPORT,
                __func__,
                "unexcepted NULL \
				pointer for a call to the url checker in the \
				plugin %s",
                cata_list->getName());
        return FALSE;
    }
}

//
// Resolve entry point in a plugin and add it to the current plugin list
//
static int gfal_module_init(gfal_handle handle, void* dlhandle, const char* module_name, GError** err){
	GError* tmp_err=NULL;
	static gfal_plugin_interface (*constructor)(gfal_handle,GError**);
	int* n = &handle->plugin_opt.plugin_number;
	int ret =-1;
	constructor= (gfal_plugin_interface (*)(gfal_handle,GError**)) dlsym(dlhandle, GFAL_PLUGIN_INIT_SYM);
	if(constructor == NULL){
        g_set_error(&tmp_err, gfal2_get_plugins_quark(), EINVAL, "No symbol %s found in the plugin %s, failure", GFAL_PLUGIN_INIT_SYM, module_name);
		*n=0;
	}else{	
		handle->plugin_opt.plugin_list[*n] = constructor(handle, &tmp_err);
		handle->plugin_opt.plugin_list[*n].gfal_data = dlhandle;
		if(tmp_err){
            g_prefix_error(&tmp_err, "Unable to load plugin %s : ", module_name);
			*n=0;		
		}else{
			*n+=1;
            gfal_log(GFAL_VERBOSE_NORMAL, "[gfal_module_load] plugin %s loaded with success ", module_name);
			ret=0;
		}
	}
    G_RETURN_ERR(ret, tmp_err, err);
}

// unload each loaded plugin
int gfal_plugins_delete(gfal_handle handle, GError** err){
    g_return_val_err_if_fail(handle, -1, err, "[gfal_plugins_delete] Invalid value of handle");
    const int plugin_number = handle->plugin_opt.plugin_number;
    if(plugin_number > 0){
            int i;
            for(i=0; i< plugin_number; ++i){
                gfal_plugin_interface* p = &(handle->plugin_opt.plugin_list[i]);
                if(p->plugin_delete)
                    p->plugin_delete(gfal_get_plugin_handle(p));
            }

        handle->plugin_opt.plugin_number =0;
    }
    return 0;
}



// return the proper plugin linked to this file handle
gfal_plugin_interface* gfal_plugin_map_file_handle(gfal_handle handle, gfal_file_handle fh, GError** err){
	GError* tmp_err=NULL;
	int i;
	gfal_plugin_interface* cata_list=NULL;
	int n = gfal_plugins_instance(handle, &tmp_err);
	if(n > 0){
		cata_list = handle->plugin_opt.plugin_list;
		for(i=0; i < n; ++i){
			if( strncmp(cata_list[i].getName(),fh->module_name, GFAL_MODULE_NAME_SIZE)==0 )
				return &(cata_list[i]);
		}
        g_set_error(&tmp_err, gfal2_get_plugins_quark(), EBADF, "No gfal_module with the handle name : %s", fh->module_name);
	}else{
        g_set_error(&tmp_err, gfal2_get_plugins_quark(), EINVAL, "No gfal_module loaded");
	}
	if(tmp_err)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
	return cata_list;
}

// external function to get the list of the plugins loaded
char** gfal_plugins_get_list(gfal_handle handle, GError** err){
	GError* tmp_err=NULL;
	char** resu =NULL;
	int n = gfal_plugins_instance(handle, &tmp_err);
	if(n > 0){
		resu =g_new0(char*,n+1);
		int i;
		gfal_plugin_interface* cata_list = handle->plugin_opt.plugin_list;
		for(i=0; i < n; ++i, ++cata_list){
			resu[i] = strndup(cata_list->getName(), GFAL_URL_MAX_LEN);
		}	
	}
	if(tmp_err)
	    gfal2_propagate_prefixed_error(err, tmp_err, __func__);
	return resu;
}


// external function to return a gfal_plugin_interface from a given plugin name
gfal_plugin_interface* gfal_search_plugin_with_name(gfal_handle handle, const char* name, GError** err){
  g_return_val_err_if_fail(name && handle, NULL, err, "must be non NULL value");
  GError* tmp_err=NULL;
  gfal_plugin_interface* resu= NULL;
  int n = gfal_plugins_instance(handle, &tmp_err);
  if(n > 0){
    int i;
    gfal_plugin_interface* cata_list = handle->plugin_opt.plugin_list;
    for(i=0; i < n; ++i, ++cata_list){
      const char* plugin_name = cata_list->getName();
      if(plugin_name != NULL && strcmp(plugin_name, name) ==0){
	  resu = cata_list;
	  break;
      }
    }
    if(resu ==NULL)
      g_set_error(&tmp_err, gfal2_get_plugins_quark(), ENOENT, " No plugin loaded with this name %s", name);
  }

  if(tmp_err)
      gfal2_propagate_prefixed_error(err, tmp_err, __func__);
  return resu;  
}

//  load the gfal_plugins in the listed library
static int gfal_module_load(gfal_handle handle, char* module_name, GError** err){
	void* dlhandle = dlopen(module_name, RTLD_LAZY);
	GError * tmp_err=NULL;
	int ret = -1;
	if (dlhandle==NULL)
        g_set_error(&tmp_err, gfal2_get_plugins_quark(), EINVAL,
                "Unable to open the %s plugin specified in the plugin directory, failure : %s",
                module_name, dlerror());
	else
		ret = gfal_module_init(handle, dlhandle, module_name, &tmp_err);	
	if(tmp_err)
	    gfal2_propagate_prefixed_error(err, tmp_err, __func__);
	return ret;
}

plugin_pointer_handle gfal_plugins_list_handler(gfal_handle handle, GError** err){
	GError* tmp_err=NULL;
	plugin_pointer_handle resu =NULL;
	int n = gfal_plugins_instance(handle, &tmp_err);
	if(n > 0){
		resu =g_new0(struct _plugin_pointer_handle,n+1);
		int i;
        GList * plugin_list = g_list_first(handle->plugin_opt.sorted_plugin);
        i =0;
        while(plugin_list != NULL){
            gfal_plugin_interface* cata_list = plugin_list->data;
			resu[i].dlhandle = cata_list->gfal_data;
			resu[i].plugin_data = cata_list->plugin_data;
            resu[i].plugin_api = cata_list;
			g_strlcpy(resu[i].plugin_name, cata_list->getName(), GFAL_URL_MAX_LEN);
            i++;
            plugin_list = g_list_next(plugin_list);
		}	
	}
	
	G_RETURN_ERR(resu,tmp_err, err);	
}


/*
 * Provide a list of the gfal 2.0 plugins path
 *  return NULL terminated table of plugins
*/
char ** gfal_list_directory_plugins(const char * dir, GError ** err){
	GError * tmp_err=NULL;
	char ** res, ** p_res;
	res = p_res =NULL;
	int n = 0;
	GDir* d = g_dir_open (dir,0, &tmp_err);

	if(d){
	   gchar * d_name = NULL;
	   while( (d_name = (char*) g_dir_read_name(d)) != NULL){
		    if(strstr(d_name, G_MODULE_SUFFIX) != NULL){
				GString * strbuff = g_string_new(dir);
				n++;
				if(n == 1){
					res = malloc(sizeof(char*)*2);
					p_res = res;
				}else{
					res = realloc(res, sizeof(char*)*(n+1));
					p_res = res + n -1;
				}
				gfal_log(GFAL_VERBOSE_TRACE, " [gfal_list_directory_plugins] add plugin to list to load %s%s%s ",
				dir, G_DIR_SEPARATOR_S,  d_name);	 
				g_string_append (strbuff, G_DIR_SEPARATOR_S);  
				g_string_append (strbuff, d_name); 
				*p_res = g_string_free(strbuff, FALSE);						
		   }else{
				gfal_log(GFAL_VERBOSE_TRACE, " [gfal_list_directory_plugins] WARNING : File that is not a plugin in the plugin directory %s%s%s ",
				dir, G_DIR_SEPARATOR_S,  d_name);	 	   
		   }
		}
		if(p_res != NULL){
		   p_res++;		// finish string tab
		   *p_res = NULL;
		  } 
		g_dir_close (d);		
	}

	if(tmp_err) {
	    g_prefix_error(&tmp_err, "Error, gfal 2.0 plugins directory : %s -> ", dir);
		gfal2_propagate_prefixed_error(err, tmp_err, __func__);
	}
	return res;
}

//
char ** gfal_localize_plugins(GError** err){
	GError * tmp_err=NULL;
	char** res = NULL;
	char * gfal_plugin_dir = (char*) g_getenv(GFAL_PLUGIN_DIR_ENV);
	if(gfal_plugin_dir != NULL){
		gfal_log(GFAL_VERBOSE_VERBOSE, "... %s environnement variable specified, \
			try to load the plugins in given dir : %s",GFAL_PLUGIN_DIR_ENV,   gfal_plugin_dir);
	}else{
		/* GFAL_PLUGIN_DIR_DEFAULT defined at compilation time */
		gfal_plugin_dir = GFAL_PLUGIN_DIR_DEFAULT 
						  G_DIR_SEPARATOR_S;
		gfal_log(GFAL_VERBOSE_VERBOSE, "... no %s environnement variable specified, try to load plugins in the default directory : %s",GFAL_PLUGIN_DIR_ENV,   gfal_plugin_dir);		
			
	}
	res = gfal_list_directory_plugins(gfal_plugin_dir, &tmp_err);
	G_RETURN_ERR(res, tmp_err, err);
}

//
int gfal_modules_resolve(gfal_handle handle, GError** err){
	GError* tmp_err = NULL;
	int ret=-1;
	char** tab_args;
	
	if( (tab_args = gfal_localize_plugins(&tmp_err)) != NULL){
		char** p= tab_args;
		while(*p  != NULL ){
			if(**p=='\0')
				break;
			if( gfal_module_load(handle, *p, &tmp_err) != 0){
				ret = -1;
				break;
			}
			gfal_log(GFAL_VERBOSE_VERBOSE, " gfal_plugin loaded succesfully : %s", *p);
			ret =0;
			p++;
		}
		g_strfreev(tab_args);
	}

	if(tmp_err)
	    gfal2_propagate_prefixed_error(err, tmp_err, __func__);
	return ret;
}

//
// compare of the plugin function
//
gint gfal_plugin_compare(gconstpointer a, gconstpointer b){
    gfal_plugin_interface* pa  = (gfal_plugin_interface*)  a;
    gfal_plugin_interface* pb  = (gfal_plugin_interface*)  b;
    return ( pa->priority > pb->priority)?(-1):(((pa->priority == pb->priority)?0:1));
}


//
// Sort plugins by priority
//
int gfal_plugins_sort(gfal_handle handle, GError ** err){
    int i;
    for(i=0; i < handle->plugin_opt.plugin_number;++i){
        handle->plugin_opt.sorted_plugin = g_list_append(handle->plugin_opt.sorted_plugin, &(handle->plugin_opt.plugin_list[i]));
    }
    handle->plugin_opt.sorted_plugin= g_list_sort(handle->plugin_opt.sorted_plugin, &gfal_plugin_compare);

    if(gfal_get_verbose() & GFAL_VERBOSE_TRACE){ // print plugin order
        GString* strbuff = g_string_new (" plugin priority order: ");
        GList* l = handle->plugin_opt.sorted_plugin;

        for(i=0; i < handle->plugin_opt.plugin_number;++i){
            strbuff = g_string_append(strbuff, ((gfal_plugin_interface*) l->data)->getName());
            strbuff = g_string_append(strbuff, " -> ");
            l = g_list_next(l);
        }
        gfal_log(GFAL_VERBOSE_TRACE, "%s", strbuff->str);
        g_string_free(strbuff, TRUE);
    }
    return 0;
}

//
// Instance all plugins for use if it's not the case
// return the number of plugin available
//
int gfal_plugins_instance(gfal_handle handle, GError** err){
	g_return_val_err_if_fail(handle, -1, err, "[gfal_plugins_instance]  invalid value of handle");
	const int plugin_number = handle->plugin_opt.plugin_number;
	if(plugin_number <= 0){
		GError* tmp_err=NULL;
		gfal_modules_resolve(handle, &tmp_err);
		if(tmp_err){
		    gfal2_propagate_prefixed_error(err, tmp_err, __func__);
			handle->plugin_opt.plugin_number = -1;
        }else if(handle->plugin_opt.plugin_number > 0){
            gfal_plugins_sort(handle, &tmp_err);
            if(tmp_err){
                gfal2_propagate_prefixed_error(err, tmp_err, __func__);
                return -1;
            }
        }
		return handle->plugin_opt.plugin_number;	
	}
	return plugin_number;
}




gfal_plugin_interface* gfal_find_plugin(gfal_handle handle, const char * url,
        plugin_mode acc_mode, GError** err)
{
    GError* tmp_err = NULL;
    gboolean compatible = FALSE;
    const int n_plugins = gfal_plugins_instance(handle, &tmp_err);
    if (n_plugins > 0) {
        GList * plugin_list = g_list_first(handle->plugin_opt.sorted_plugin);
        while (plugin_list != NULL ) {
            gfal_plugin_interface* cata_list = plugin_list->data;
            compatible = gfal_plugin_checker_safe(cata_list, url, acc_mode,
                    &tmp_err);
            if (tmp_err)
                break;
            if (compatible)
                return cata_list;
            plugin_list = g_list_next(plugin_list);
        }
    }
    if (tmp_err) {
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
    }
    else {
        gfal2_set_error(err, gfal2_get_plugins_quark(), EPROTONOSUPPORT,
                __func__, "Protocol not supported or path/url invalid");
    }
    return NULL ;
}


 

//  Execute an access function on the first plugin compatible in the plugin list
//  return the result of the first valid plugin for a given URL
//  result of the access method or -1 if error and set GError with the correct value
//  error : EPROTONOSUPPORT means that the URL is not matched by a plugin
int gfal_plugins_accessG(gfal_handle handle, const char* path, int mode, GError** err){
    g_return_val_err_if_fail(handle && path, EINVAL, err, "[gfal_plugins_accessG] Invalid arguments");
    int res=-1;
    GError * tmp_err=NULL;
    gfal_plugin_interface* p = gfal_find_plugin(handle, path, GFAL_PLUGIN_ACCESS, &tmp_err);

    if(p)
        res = p->accessG(gfal_get_plugin_handle(p), path, mode, &tmp_err);

    G_RETURN_ERR(res, tmp_err, err);
}

//  Execute a stat function on the appropriate plugin
int gfal_plugin_statG(gfal_handle handle, const char* path, struct stat* st, GError** err){
	g_return_val_err_if_fail(handle && path, EINVAL, err, "[gfal_plugin_statG] Invalid arguments");
    int ret =-1;
    GError* tmp_err=NULL;
	
    gfal_plugin_interface* p = gfal_find_plugin(handle, path, GFAL_PLUGIN_STAT, &tmp_err);

    if(p)
        ret = p->statG(gfal_get_plugin_handle(p), path, st, &tmp_err);

    G_RETURN_ERR(ret, tmp_err, err);
}


//  Execute a stat function on the appropriate plugin
int gfal_plugin_lstatG(gfal_handle handle, const char* path, struct stat* st, GError** err){
    g_return_val_err_if_fail(handle && path, EINVAL, err, "[gfal_plugin_statG] Invalid arguments");
    int ret =-1;
    GError* tmp_err=NULL;

    gfal_plugin_interface* p = gfal_find_plugin(handle, path, GFAL_PLUGIN_LSTAT, &tmp_err);

    if(p)
        ret = p->lstatG(gfal_get_plugin_handle(p), path, st, &tmp_err);

    G_RETURN_ERR(ret, tmp_err, err);
}


//  Execute a readlink function on the appropriate plugin
ssize_t gfal_plugin_readlinkG(gfal_handle handle, const char* path, char* buff, size_t buffsiz, GError** err){
    g_return_val_err_if_fail(handle && path, EINVAL, err, "[gfal_plugin_readlinkG] Invalid arguments");
    GError* tmp_err=NULL;
    ssize_t resu=-1;

   gfal_plugin_interface* p = gfal_find_plugin(handle, path, GFAL_PLUGIN_READLINK, &tmp_err);

   if(p)
       resu = p->readlinkG(gfal_get_plugin_handle(p), path, buff, buffsiz, &tmp_err);

    G_RETURN_ERR(resu, tmp_err, err);
}



//  Execute a chmod function on the appropriate plugin 
 int gfal_plugin_chmodG(gfal_handle handle, const char* path, mode_t mode, GError** err){
	g_return_val_err_if_fail(handle && path, -1, err, "[gfal_plugin_chmodG] Invalid arguments");	
    GError* tmp_err = NULL;
    int ret = -1;

    gfal_plugin_interface* p = gfal_find_plugin(handle, path, GFAL_PLUGIN_CHMOD, &tmp_err);

    if(p)
        ret = p->chmodG(gfal_get_plugin_handle(p), path, mode, &tmp_err);
	
    G_RETURN_ERR(ret, tmp_err, err);
 }
 

// Execute a rename function on the appropriate plugin 
int gfal_plugin_renameG(gfal_handle handle, const char* oldpath, const char* newpath, GError** err){
	g_return_val_err_if_fail(oldpath && newpath, -1, err, "[gfal_plugin_renameG] invalid value in args oldpath, handle or newpath");
	GError* tmp_err=NULL;
    int ret = -1;
    gfal_plugin_interface *src_p, *dst_p;

    src_p = gfal_find_plugin(handle, oldpath, GFAL_PLUGIN_RENAME, &tmp_err);
    if (src_p) {
        dst_p = gfal_find_plugin(handle, newpath, GFAL_PLUGIN_RENAME, &tmp_err);
        if(src_p == dst_p)
            ret = dst_p->renameG(gfal_get_plugin_handle(dst_p), oldpath, newpath, &tmp_err);
    }
	
    G_RETURN_ERR(ret, tmp_err, err);
}

// Execute a symlink function on the appropriate plugin 
int gfal_plugin_symlinkG(gfal_handle handle, const char* oldpath,
        const char* newpath, GError** err)
{
    g_return_val_err_if_fail(oldpath && newpath, -1, err, "[gfal_plugin_symlinkG] invalid value in args oldpath, handle or newpath");
    GError* tmp_err = NULL;
    int ret = -1;
    gfal_plugin_interface *src_p, *dst_p;

    src_p = gfal_find_plugin(handle, oldpath, GFAL_PLUGIN_SYMLINK, &tmp_err);
    if (src_p) {
        dst_p = gfal_find_plugin(handle, newpath, GFAL_PLUGIN_SYMLINK, &tmp_err);
        if (src_p == dst_p)
            ret = dst_p->symlinkG(gfal_get_plugin_handle(dst_p), oldpath, newpath, &tmp_err);
    }

    G_RETURN_ERR(ret, tmp_err, err);
}

// Execute a mkdir function on the appropriate plugin 
int gfal_plugin_mkdirp(gfal_handle handle, const char* path, mode_t mode, gboolean pflag,  GError** err){
	g_return_val_err_if_fail(handle && path , -1, err, "[gfal_plugin_mkdirp] Invalid argumetns in path or/and handle");
	GError* tmp_err=NULL;	
    int ret = -1;

    gfal_plugin_interface* p = gfal_find_plugin(handle, path, GFAL_PLUGIN_MKDIR, &tmp_err);

    if(p)
        ret = p->mkdirpG(gfal_get_plugin_handle(p), path, mode, pflag, &tmp_err);

    G_RETURN_ERR(ret, tmp_err, err);
}

// Execute a rmdir function on the appropriate plugin 
int gfal_plugin_rmdirG(gfal_handle handle, const char* path, GError** err){
	g_return_val_err_if_fail(handle && path , -1, err, "[gfal_plugin_rmdirp] Invalid arguments in path or/and handle");
	GError* tmp_err=NULL;	
    int ret =-1;
    gfal_plugin_interface* p = gfal_find_plugin(handle, path, GFAL_PLUGIN_RMDIR, &tmp_err);

    if(p)
        ret = p->rmdirG(gfal_get_plugin_handle(p), path,  &tmp_err);

    G_RETURN_ERR(ret, tmp_err, err);
}

// Execute a opendir function on the appropriate plugin 
 gfal_file_handle gfal_plugin_opendirG(gfal_handle handle, const char* name, GError** err){
	g_return_val_err_if_fail(handle && name, NULL, err,  "[gfal_plugin_opendir] invalid value");
	GError* tmp_err=NULL;	
	gfal_file_handle resu=NULL;

    gfal_plugin_interface* p = gfal_find_plugin(handle, name, GFAL_PLUGIN_OPENDIR, &tmp_err);

    if(p)
        resu = p->opendirG(gfal_get_plugin_handle(p), name,  &tmp_err);
	
    G_RETURN_ERR(resu, tmp_err, err);
}

// Execute a closedir function on the appropriate plugin  
int gfal_plugin_closedirG(gfal_handle handle, gfal_file_handle fh, GError** err){
	g_return_val_err_if_fail(handle && fh, -1,err, "[gfal_plugin_closedirG] Invalid args ");	
	GError* tmp_err=NULL;
	int ret = -1;
    gfal_plugin_interface* if_cata = gfal_plugin_map_file_handle(handle, fh, &tmp_err);
	if(!tmp_err)
		ret = if_cata->closedirG(if_cata->plugin_data, fh, &tmp_err);
    G_RETURN_ERR(ret, tmp_err, err);
}

// Execute a open function on the appropriate plugin  
gfal_file_handle gfal_plugin_openG(gfal_handle handle, const char * path, int flag, mode_t mode, GError ** err){
	GError* tmp_err=NULL;
	gfal_file_handle resu =NULL;
	gfal_log(GFAL_VERBOSE_TRACE, " %s ->",__func__);

    gfal_plugin_interface* p = gfal_find_plugin(handle, path, GFAL_PLUGIN_OPEN, &tmp_err);

    if(p)
        resu = p->openG(gfal_get_plugin_handle(p), path, flag, mode, &tmp_err);

    G_RETURN_ERR(resu, tmp_err, err);
}

// Execute a close function on the appropriate plugin  
int gfal_plugin_closeG(gfal_handle handle, gfal_file_handle fh, GError** err){
	g_return_val_err_if_fail(handle && fh, -1,err, "[gfal_plugin_closeG] Invalid args ");	
	GError* tmp_err=NULL;
	int ret = -1;

	gfal_log(GFAL_VERBOSE_TRACE, " <- %s", __func__);

    gfal_plugin_interface* if_cata = gfal_plugin_map_file_handle(handle, fh, &tmp_err);
	if(!tmp_err)
		ret = if_cata->closeG(if_cata->plugin_data, fh, &tmp_err);

    G_RETURN_ERR(ret, tmp_err, err);
}

// Execute a readdir function on the appropriate plugin  
struct dirent* gfal_plugin_readdirG(gfal_handle handle, gfal_file_handle fh, GError** err){
	g_return_val_err_if_fail(handle && fh, NULL,err, "[gfal_plugin_readdirG] Invalid args ");	
	GError* tmp_err=NULL;
	struct dirent* ret = NULL;
    gfal_plugin_interface* if_cata = gfal_plugin_map_file_handle(handle, fh, &tmp_err);
	if(!tmp_err)
		ret = if_cata->readdirG(if_cata->plugin_data, fh, &tmp_err);

    G_RETURN_ERR(ret, tmp_err, err);
}


// Execute a readdir function on the appropriate plugin
struct dirent* gfal_plugin_readdirppG(gfal_handle handle, gfal_file_handle fh, struct stat* st, GError** err){
    g_return_val_err_if_fail(handle && fh, NULL,err, "[gfal_plugin_readdirppG] Invalid args ");
    GError* tmp_err=NULL;
    struct dirent* ret = NULL;
    gfal_plugin_interface* if_cata = gfal_plugin_map_file_handle(handle, fh, &tmp_err);

    if(!tmp_err){
        if(gfal_feature_is_supported(if_cata->readdirppG, g_quark_from_string(GFAL2_PLUGIN_SCOPE) , __func__, &tmp_err))
            ret = if_cata->readdirppG(if_cata->plugin_data, fh, st, &tmp_err);
    }

    G_RETURN_ERR(ret, tmp_err, err);
}




// Execute a getxattr function on the appropriate plugin  
ssize_t gfal_plugin_getxattrG(gfal_handle handle, const char* path, const char*name, void* buff, size_t s_buff, GError** err){
	GError* tmp_err=NULL;
	ssize_t resu = -1;

    gfal_plugin_interface* p = gfal_find_plugin(handle, path, GFAL_PLUGIN_GETXATTR, &tmp_err);

    if(p)
        resu = p->getxattrG(gfal_get_plugin_handle(p), path, name, buff, s_buff, &tmp_err);
	
    G_RETURN_ERR(resu, tmp_err, err);
}

// Execute a listxattr function on the appropriate plugin  
ssize_t gfal_plugin_listxattrG(gfal_handle handle, const char* path, char* list, size_t s_list, GError** err){
	GError* tmp_err=NULL;
	ssize_t resu = -1;

    gfal_plugin_interface* p = gfal_find_plugin(handle, path, GFAL_PLUGIN_LISTXATTR, &tmp_err);

    if(p)
        resu = p->listxattrG(gfal_get_plugin_handle(p), path, list, s_list, &tmp_err);

    G_RETURN_ERR(resu, tmp_err, err);
}


// Execute a setxattr function on the appropriate plugin
int gfal_plugin_setxattrG(gfal_handle handle, const char* path, const char* name, const void* value, size_t size, int flags, GError** err){
    GError* tmp_err=NULL;
    int resu = -1;

    gfal_plugin_interface* p = gfal_find_plugin(handle, path, GFAL_PLUGIN_SETXATTR, &tmp_err);

    if(p)
        resu = p->setxattrG(gfal_get_plugin_handle(p), path, name, value, size, flags, &tmp_err);
    G_RETURN_ERR(resu, tmp_err, err);
}



// Execute a read function on the appropriate plugin  
int gfal_plugin_readG(gfal_handle handle, gfal_file_handle fh, void* buff, size_t s_buff, GError** err){
	g_return_val_err_if_fail(handle && fh && buff && s_buff> 0, -1,err, "[gfal_plugin_readG] Invalid args ");	
	GError* tmp_err=NULL;
	int ret = -1;
    gfal_plugin_interface* if_cata = gfal_plugin_map_file_handle(handle, fh, &tmp_err);
	if(!tmp_err)
		ret = if_cata->readG(if_cata->plugin_data, fh, buff, s_buff,  &tmp_err);
    G_RETURN_ERR(ret, tmp_err, err);
}

// Simulate a pread operation in case of non-parallels read support
// this is slower than a normal pread/pwrite operation 
inline ssize_t gfal_plugin_simulate_preadG(gfal_handle handle, gfal_plugin_interface* if_cata, gfal_file_handle fh, 
						void* buff, size_t s_buff, off_t offset, GError** err){
	GError* tmp_err=NULL;
	ssize_t ret = -1;
	
	gfal_file_handle_lock(fh);		
	ret = if_cata->lseekG(if_cata->plugin_data, fh, offset, SEEK_SET, &tmp_err);
	if(ret == offset){
		ret = if_cata->readG(if_cata->plugin_data, fh, buff, s_buff, &tmp_err);
	}else if( !tmp_err){
        g_set_error(&tmp_err, gfal2_get_plugins_quark(), EOVERFLOW, "Unknown return from plugin_lseek call");
		ret = -1;
	}
	gfal_file_handle_unlock(fh);
	
    G_RETURN_ERR(ret, tmp_err, err);
}

// Execute a pread function on the appropriate plugin  
ssize_t gfal_plugin_preadG(gfal_handle handle, gfal_file_handle fh, void* buff, size_t s_buff, off_t offset, GError** err){
	g_return_val_err_if_fail(handle && fh && buff, -1,err, "[gfal_plugin_preadG] Invalid args ");	
	GError* tmp_err=NULL;
	ssize_t ret = -1;
    gfal_plugin_interface* if_cata = gfal_plugin_map_file_handle(handle, fh, &tmp_err);
	if(!tmp_err){
		if(if_cata->preadG)
			ret = if_cata->preadG(if_cata->plugin_data, fh, buff, s_buff, offset,  &tmp_err);
		else{
			ret = gfal_plugin_simulate_preadG(handle, if_cata, fh, buff, s_buff, offset, &tmp_err);
		}	
	}
    G_RETURN_ERR(ret, tmp_err, err);
}


// Simulate a pread operation in case of non-parallels write support
// this is slower than a normal pread/pwrite operation 
inline ssize_t gfal_plugin_simulate_pwriteG(gfal_handle handle, gfal_plugin_interface* if_cata, gfal_file_handle fh, 
						void* buff, size_t s_buff, off_t offset, GError** err){
	GError* tmp_err=NULL;
	ssize_t ret = -1;
	
	gfal_file_handle_lock(fh);		
	ret = if_cata->lseekG(if_cata->plugin_data, fh, offset, SEEK_SET, &tmp_err);
	if(ret == offset){
		ret = if_cata->writeG(if_cata->plugin_data, fh, buff, s_buff, &tmp_err);
	}else if( !tmp_err){
        g_set_error(&tmp_err, gfal2_get_plugins_quark(), EOVERFLOW, "Unknown return from plugin_lseek call");
		ret = -1;
	}
	gfal_file_handle_unlock(fh);
	
    G_RETURN_ERR(ret, tmp_err, err);
}




// Execute a pwrite function on the appropriate plugin  
ssize_t gfal_plugin_pwriteG(gfal_handle handle, gfal_file_handle fh, void* buff, size_t s_buff, off_t offset, GError** err){
	g_return_val_err_if_fail(handle && fh && buff, -1,err, "[gfal_plugin_pwriteG] Invalid args ");	
	GError* tmp_err=NULL;
	ssize_t ret = -1;
    gfal_plugin_interface* if_cata = gfal_plugin_map_file_handle(handle, fh, &tmp_err);
	if(!tmp_err){
		if(if_cata->pwriteG)
			ret = if_cata->pwriteG(if_cata->plugin_data, fh, buff, s_buff, offset,  &tmp_err);
		else{
			ret = gfal_plugin_simulate_pwriteG(handle, if_cata, fh, buff, s_buff, offset, &tmp_err);
		}	
	}
    G_RETURN_ERR(ret, tmp_err, err);
}



// Execute a lseek function on the appropriate plugin  
int gfal_plugin_lseekG(gfal_handle handle, gfal_file_handle fh, off_t offset, int whence, GError** err){
	g_return_val_err_if_fail(handle && fh , -1,err, "[gfal_plugin_lseekG] Invalid args ");	
	GError* tmp_err=NULL;
	int ret = -1;
    gfal_plugin_interface* if_cata = gfal_plugin_map_file_handle(handle, fh, &tmp_err);
	if(!tmp_err)
		ret = if_cata->lseekG(if_cata->plugin_data, fh, offset, whence,  &tmp_err);
    G_RETURN_ERR(ret, tmp_err, err);
	
}

// Execute a write function on the appropriate plugin  
int gfal_plugin_writeG(gfal_handle handle, gfal_file_handle fh, void* buff, size_t s_buff, GError** err){
	g_return_val_err_if_fail(handle && fh && buff && s_buff> 0, -1,err, "[gfal_plugin_writeG] Invalid args ");	
	GError* tmp_err=NULL;
	int ret = -1;
    gfal_plugin_interface* if_cata = gfal_plugin_map_file_handle(handle, fh, &tmp_err);
	if(!tmp_err)
		ret = if_cata->writeG(if_cata->plugin_data, fh,buff, s_buff, &tmp_err);
    G_RETURN_ERR(ret, tmp_err, err);
}



// Execute a unlink function on the appropriate plugin  
int gfal_plugin_unlinkG(gfal_handle handle, const char* path, GError** err){
	GError* tmp_err=NULL;
    int resu = -1;
    gfal_plugin_interface* p = gfal_find_plugin(handle, path, GFAL_PLUGIN_UNLINK, &tmp_err);

    if(p)
        resu = p->unlinkG(gfal_get_plugin_handle(p), path,  &tmp_err);
    G_RETURN_ERR(resu, tmp_err, err);

}

int gfal_plugin_bring_onlineG(gfal2_context_t handle, const char* uri,
                              time_t pintime, time_t timeout,
                              char* token, size_t tsize,
                              int async,
                              GError ** err){
    GError* tmp_err=NULL;
    int resu = -1;
    gfal_plugin_interface* p = gfal_find_plugin(handle, uri, GFAL_PLUGIN_BRING_ONLINE, &tmp_err);

    if(p)
        resu = p->bring_online(gfal_get_plugin_handle(p), uri,
                                 pintime, timeout,
                                 token, tsize,
                                 async,
                                 &tmp_err);
    G_RETURN_ERR(resu, tmp_err, err);
 }


int gfal_plugin_bring_online_pollG(gfal2_context_t handle, const char* uri,
                                   const char* token, GError ** err) {
    GError* tmp_err=NULL;
    int resu = -1;
    gfal_plugin_interface* p = gfal_find_plugin(handle, uri, GFAL_PLUGIN_BRING_ONLINE, &tmp_err);

    if(p)
        resu = p->bring_online_poll(gfal_get_plugin_handle(p), uri,  token, &tmp_err);
    G_RETURN_ERR(resu, tmp_err, err);
}

int gfal_plugin_release_fileG(gfal2_context_t handle, const char* uri,
                              const char* token, GError ** err) {
    GError* tmp_err=NULL;
    int resu = -1;
    gfal_plugin_interface* p = gfal_find_plugin(handle, uri, GFAL_PLUGIN_BRING_ONLINE, &tmp_err);

    if(p)
        resu = p->release_file(gfal_get_plugin_handle(p), uri,  token, &tmp_err);
    G_RETURN_ERR(resu, tmp_err, err);
}
