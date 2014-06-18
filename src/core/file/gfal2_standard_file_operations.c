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

#include <file/gfal_file_api.h>

#include <common/future/glib.h>
#include <common/gfal_constants.h>
#include <common/gfal_types.h>
#include <common/gfal_common_internal.h>
#include <common/gfal_common_plugin.h>
#include <common/gfal_common_err_helpers.h>
#include <logger/gfal_logger.h>

//
// Mapping for the gfal 2.0 standard opeations of the file interface
//
// @author : Devresse Adrien
//
//


//
//
int gfal2_access(gfal2_context_t handle, const char *url, int amode, GError** err){
    int resu = -1;
    GError* tmp_err=NULL;

    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if(url == NULL || handle == NULL){
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "handle or/and url are incorrect arguments");
    }else{
        resu = gfal_plugins_accessG(handle, (char*) url, amode, &tmp_err );
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(((resu)?(-1):0), tmp_err, err);
}

//
//
int gfal2_chmod(gfal2_context_t handle, const char* url, mode_t mode, GError ** err){
    GError* tmp_err=NULL;
    int res= -1;

    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if(url == NULL || handle == NULL){
       g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "handle or/and url are incorrect arguments");
    }else{
       res = gfal_plugin_chmodG(handle, url, mode, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(res, tmp_err, err);
}


//
//
int gfal2_rename(gfal2_context_t handle, const char *olduri, const char *newuri, GError ** err){
    GError* tmp_err = NULL;
    int ret=-1;

    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if( olduri == NULL || newuri == NULL || handle == NULL){
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, " olduri/newuri/handle are incorrect arguments");
    }else{
        ret = gfal_plugin_renameG(handle, olduri, newuri, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(((ret)?-1:0), tmp_err, err);
}


//
//
int gfal2_stat(gfal2_context_t handle, const char* url, struct stat* buff, GError ** err){
    GError* tmp_err = NULL;
    int ret = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if(url == NULL || handle == NULL || buff == NULL){
       g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "handle or/and url or/and buff are incorrect arguments");
    }else{
        ret = gfal_plugin_statG(handle, url, buff, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(ret, tmp_err, err);
}

int gfal2_lstat(gfal2_context_t handle, const char* url, struct stat* buff,
        GError ** err)
{
    GError* tmp_err = NULL;
    int ret = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if (url == NULL || handle == NULL || buff == NULL ) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT,
                "handle or/and url or/and buff are incorrect arguments");
    }
    else {
        if ((ret = gfal_plugin_lstatG(handle, url, buff, &tmp_err))
                != 0 && tmp_err && tmp_err->code == EPROTONOSUPPORT) { // protocol does not support lstat, try to map to stat
            ret = gfal2_stat(handle, url, buff, err);
            g_error_free(tmp_err);
            tmp_err = NULL;
        }
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(ret, tmp_err, err);
}


int gfal2_mkdir(gfal2_context_t handle,  const char* uri, mode_t mode, GError ** err){
    GError* tmp_err=NULL;
    int res= -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if(uri == NULL || handle == NULL){
       g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, " uri is an incorrect argument");
    }else{
       res = gfal_plugin_mkdirp(handle, uri, mode, FALSE, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal2_mkdir_rec(gfal2_context_t handle,  const char* uri, mode_t mode, GError ** err){
    GError* tmp_err=NULL;
    int res= -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if(uri == NULL || handle == NULL){
       g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, " uri is an incorrect argument");
    }else{
       res = gfal_plugin_mkdirp(handle, uri, mode, TRUE, &tmp_err);
       if(tmp_err){
           if(tmp_err->code == EEXIST){
               g_clear_error(&tmp_err);
               res = 0;
           }else if(tmp_err->code == ENOENT){
               gfal_log(GFAL_VERBOSE_TRACE, "execute recusive directory creation for %s", uri);
               GList* stack_uri = NULL;
               char current_uri[GFAL_URL_MAX_LEN];
               g_strlcpy(current_uri, uri, GFAL_URL_MAX_LEN);

               while(tmp_err && tmp_err->code == ENOENT){
                   stack_uri= g_list_prepend(stack_uri, g_strdup(current_uri));
                   g_clear_error(&tmp_err);
                   const size_t s_uri = strlen(current_uri);
                   char* p_uri = current_uri + s_uri -1;
                   while( p_uri > current_uri && *p_uri == '/' ){ // remove trailing '/'
                       *p_uri = '\0';
                        p_uri--;
                   }
                   while( p_uri > current_uri && *p_uri != '/'){ // find the parent directory
                       p_uri--;
                   }
                   if(p_uri > current_uri){
                       *p_uri = '\0';

                        res = gfal_plugin_mkdirp(handle, current_uri, mode, FALSE, &tmp_err);
                        if( res == 0){
                          gfal_log(GFAL_VERBOSE_TRACE, "create directory %s", current_uri);
                        }
                   }

               }

               if(!tmp_err){
                   res = 0;
                   GList* tmp_list = stack_uri;
                   while(tmp_list != NULL
                         && res ==0){
                       res = gfal_plugin_mkdirp(handle, (char*) tmp_list->data, mode, FALSE, &tmp_err);
                       if(res == 0){
                        gfal_log(GFAL_VERBOSE_TRACE, "create directory %s", current_uri);
                       }
                       tmp_list = g_list_next(tmp_list);
                   }

               }

               g_list_free_full(stack_uri, g_free);
           }
       }

    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal2_rmdir(gfal2_context_t handle, const char* uri, GError ** err){
    GError* tmp_err=NULL;
    int res= -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if(uri == NULL || handle == NULL){
       g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, " path is an incorrect argument");
    }else{
       res = gfal_plugin_rmdirG(handle, uri, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal2_symlink(gfal2_context_t handle, const char* olduri, const char * newuri, GError ** err){
    GError* tmp_err = NULL;
    int ret=-1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if( olduri == NULL || newuri == NULL || handle == NULL ){
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, " olduri and/or newuri and/or handle are incorrect arguments");
    }else{
        ret = gfal_plugin_symlinkG(handle, olduri, newuri, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(((ret)?-1:0), tmp_err, err);
}


ssize_t gfal2_getxattr (gfal2_context_t handle, const char *url, const char *name,
                        void *value, size_t size, GError ** err){
    GError* tmp_err=NULL;
    ssize_t res= -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if(url == NULL || handle == NULL || name == NULL ){
       g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "url or/and handle or/and name are incorrect arguments");
    }else{
       res = gfal_plugin_getxattrG(handle, url, name, value, size, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(res, tmp_err, err);
}


ssize_t gfal2_readlink(gfal2_context_t handle, const char* uri, char* buff, size_t buffsiz, GError ** err){
    GError* tmp_err = NULL;
    ssize_t ret = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if(uri ==NULL || buff==NULL || handle == NULL){
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, " uri and/or buff or/and handle are incorrect arguments");
    }else{
        ret = gfal_plugin_readlinkG(handle, uri, buff, buffsiz, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(ret, tmp_err, err);
}

int gfal2_unlink(gfal2_context_t handle, const char* uri, GError ** err){
    GError* tmp_err = NULL;
    ssize_t ret = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if(uri == NULL || handle == NULL){
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, " uri and/or handle are incorrect arguments");
    }else{
        ret = gfal_plugin_unlinkG(handle, uri, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(ret, tmp_err, err);
}

ssize_t gfal2_listxattr (gfal2_context_t handle, const char *url, char *list, size_t size, GError ** err){
    GError* tmp_err=NULL;
    ssize_t res= -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if(url == NULL || handle == NULL  ){
       g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "handle or/and uri or/and list are incorrect arguments");
    }else{
       res = gfal_plugin_listxattrG(handle, url, list, size, &tmp_err);

    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(res, tmp_err, err);
}

int gfal2_setxattr (gfal2_context_t handle, const char *uri, const char *name,
               const void *value, size_t size, int flags, GError ** err){
    GError* tmp_err=NULL;
    int res= -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if(uri == NULL || name == NULL|| handle == NULL){
       g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "uri or/and name or/and handle are an incorrect arguments");
    }else{
       res = gfal_plugin_setxattrG(handle, uri, name, value, size, flags, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(res, tmp_err, err);
}

int gfal2_bring_online(gfal2_context_t handle, const char* uri,
                       time_t pintime, time_t timeout,
                       char* token, size_t tsize,
                       int async,
                       GError ** err){
    GError* tmp_err=NULL;
    int res= -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if(uri == NULL || handle == NULL){
       g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "uri or/and name or/and handle are an incorrect arguments");
    }else{
       res = gfal_plugin_bring_onlineG(handle, uri,
                                       pintime, timeout,
                                       token, tsize,
                                       async,
                                       &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(res, tmp_err, err);
}

int gfal2_bring_online_poll(gfal2_context_t handle, const char* uri,
                            const char* token, GError ** err) {
    GError* tmp_err=NULL;
    int res= -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if(uri == NULL || handle == NULL){
       g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "uri or/and name or/and handle are an incorrect arguments");
    }else{
       res = gfal_plugin_bring_online_pollG(handle, uri,
                                           token, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(res, tmp_err, err);
}

int gfal2_release_file(gfal2_context_t handle, const char* uri,
                       const char* token, GError ** err) {
  GError* tmp_err=NULL;
  int res= -1;
  GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
  if(uri == NULL || handle == NULL){
     g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "uri or/and name or/and handle are an incorrect arguments");
  }else{
     res = gfal_plugin_release_fileG(handle, uri, token, &tmp_err);
  }
  GFAL2_END_SCOPE_CANCEL(handle);
  G_RETURN_ERR(res, tmp_err, err);
}

int gfal2_bring_online_list(gfal2_context_t handle, int nbfiles, const char** uris,
                       time_t pintime, time_t timeout,
                       char* token, size_t tsize,
                       int async,
                       GError ** err){
    GError* tmp_err=NULL;
    int res= -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if(uris == NULL || *uris == NULL || handle == NULL){
       g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "uri or/and name or/and handle are an incorrect arguments");
    }else{
       res = gfal_plugin_bring_online_listG(handle, nbfiles, uris,
                                       pintime, timeout,
                                       token, tsize,
                                       async,
                                       &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(res, tmp_err, err);
}

int gfal2_bring_online_poll_list(gfal2_context_t handle, int nbfiles, const char** uris,
                            const char* token, GError ** err) {
    GError* tmp_err=NULL;
    int res= -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if(uris == NULL || *uris == NULL || handle == NULL){
       g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "uri or/and name or/and handle are an incorrect arguments");
    }else{
       res = gfal_plugin_bring_online_poll_listG(handle, nbfiles, uris,
                                           token, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(res, tmp_err, err);
}

int gfal2_release_file_list(gfal2_context_t handle, int nbfiles, const char** uris,
                       const char* token, GError ** err) {
  GError* tmp_err=NULL;
  int res= -1;
  GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
  if(uris == NULL || *uris == NULL || handle == NULL){
     g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "uri or/and name or/and handle are an incorrect arguments");
  }else{
     res = gfal_plugin_release_file_listG(handle, nbfiles, uris, token, &tmp_err);
  }
  GFAL2_END_SCOPE_CANCEL(handle);
  G_RETURN_ERR(res, tmp_err, err);
}
