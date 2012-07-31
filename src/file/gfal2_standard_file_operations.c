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

#include <common/gfal_constants.h>
#include <common/gfal_types.h>
#include <common/gfal_common_plugin.h>
#include <common/gfal_common_errverbose.h>



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

    if(url == NULL || handle == NULL){
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "handle or/and url are incorrect arguments");
    }else{
        resu = gfal_plugins_accessG(handle, (char*) url, amode, &tmp_err );
    }
    G_RETURN_ERR(((resu)?(-1):0), tmp_err, err);
}

//
//
int gfal2_chmod(gfal2_context_t handle, const char* url, mode_t mode, GError ** err){
    GError* tmp_err=NULL;
    int res= -1;

    if(url == NULL || handle == NULL){
       g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "handle or/and url are incorrect arguments");
    }else{
       res = gfal_plugin_chmodG(handle, url, mode, &tmp_err);
    }
    G_RETURN_ERR(res, tmp_err, err);
}


//
//
int gfal2_rename(gfal2_context_t handle, const char *olduri, const char *newuri, GError ** err){
    GError* tmp_err = NULL;
    int ret=-1;

    if( olduri == NULL || newuri == NULL || handle == NULL){
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, " olduri/newuri/handle are incorrect arguments");
    }else{
        ret = gfal_plugin_renameG(handle, olduri, newuri, &tmp_err);
    }

    G_RETURN_ERR(((ret)?-1:0), tmp_err, err);
}


//
//
int gfal2_stat(gfal2_context_t handle, const char* url, struct stat* buff, GError ** err){
    GError* tmp_err = NULL;
    int ret = -1;
    if(url == NULL || handle == NULL || buff == NULL){
       g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "handle or/and url or/and buff are incorrect arguments");
    }else{
        ret = gfal_plugin_statG(handle, url, buff, &tmp_err);
    }
    G_RETURN_ERR(ret, tmp_err, err);
}

int gfal2_lstat(gfal2_context_t handle, const char* url, struct stat* buff, GError ** err){
    GError* tmp_err = NULL;
    int ret = -1;
    if(url == NULL || handle == NULL || buff == NULL){
       g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "handle or/and url or/and buff are incorrect arguments");
    }else{
        ret = gfal_plugin_lstatG(handle, url, buff, &tmp_err);
    }
    G_RETURN_ERR(ret, tmp_err, err);
}


int gfal2_mkdir(gfal2_context_t handle,  const char* uri, mode_t mode, GError ** err){
    GError* tmp_err=NULL;
    int res= -1;

    if(uri == NULL || handle == NULL){
       g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, " uri is an incorrect argument");
    }else{
       res = gfal_plugin_mkdirp(handle, uri, mode, FALSE, &tmp_err);
    }
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal2_rmdir(gfal2_context_t handle, const char* uri, GError ** err){
    GError* tmp_err=NULL;
    int res= -1;

   if(uri == NULL || handle == NULL){
       g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, " path is an incorrect argument");
   }else{
       res = gfal_plugin_rmdirG(handle, uri, &tmp_err);
   }
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal2_symlink(gfal2_context_t handle, const char* olduri, const char * newuri, GError ** err){
    GError* tmp_err = NULL;
    int ret=-1;

    if( olduri == NULL || newuri == NULL || handle == NULL ){
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, " olduri and/or newuri and/or handle are incorrect arguments");
    }else{
        ret = gfal_plugin_symlinkG(handle, olduri, newuri, &tmp_err);
    }

    G_RETURN_ERR(((ret)?-1:0), tmp_err, err);
}


ssize_t gfal2_getxattr (gfal2_context_t handle, const char *url, const char *name,
                        void *value, size_t size, GError ** err){
    GError* tmp_err=NULL;
    ssize_t res= -1;

    if(url == NULL || handle == NULL || name == NULL ){
       g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "url or/and handle or/and name are incorrect arguments");
    }else{
       res = gfal_plugin_getxattrG(handle, url, name, value, size, &tmp_err);
    }
    G_RETURN_ERR(res, tmp_err, err);
}


ssize_t gfal2_readlink(gfal2_context_t handle, const char* uri, char* buff, size_t buffsiz, GError ** err){
    GError* tmp_err = NULL;
    ssize_t ret = -1;

    if(uri ==NULL || buff==NULL || handle == NULL){
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, " uri and/or buff or/and handle are incorrect arguments");
    }else{
        ret = gfal_plugin_readlinkG(handle, uri, buff, buffsiz, &tmp_err);
    }
    G_RETURN_ERR(ret, tmp_err, err);
}

int gfal2_unlink(gfal2_context_t handle, const char* uri, GError ** err){
    GError* tmp_err = NULL;
    ssize_t ret = -1;

    if(uri == NULL || handle == NULL){
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, " uri and/or handle are incorrect arguments");
    }else{
        ret = gfal_plugin_unlinkG(handle, uri, &tmp_err);
    }
    G_RETURN_ERR(ret, tmp_err, err);
}

ssize_t gfal2_listxattr (gfal2_context_t handle, const char *url, char *list, size_t size, GError ** err){
    GError* tmp_err=NULL;
    ssize_t res= -1;

    if(url == NULL || handle == NULL || list == NULL ){
       g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "handle or/and uri or/and list are incorrect arguments");
    }else{
       res = gfal_plugin_listxattrG(handle, url, list, size, &tmp_err);

    }
    G_RETURN_ERR(res, tmp_err, err);
}

int gfal2_setxattr (gfal2_context_t handle, const char *uri, const char *name,
               const void *value, size_t size, int flags, GError ** err){
    GError* tmp_err=NULL;
    int res= -1;

    if(uri == NULL || name == NULL|| handle ==NULL){
       g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "uri or/and name or/and handle are an incorrect arguments");
    }else{
       res = gfal_plugin_setxattrG(handle, uri, name, value, size, flags, &tmp_err);;
    }
    G_RETURN_ERR(res, tmp_err, err);
}




