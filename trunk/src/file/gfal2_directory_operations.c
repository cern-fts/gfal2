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
#include <common/gfal_common_filedescriptor.h>
#include <common/gfal_common_dir_handle.h>



//
// Mapping for the gfal 2.0  opendir/readdir/closedir operations
//
// @author : Devresse Adrien
//
//

inline static int gfal_rw_dir_handle_store(gfal_handle handle, gfal_file_handle fhandle, GError** err){
    g_return_val_err_if_fail(handle && fhandle, 0, err, "[gfal_rw_dir_handle_store] handle invalid");
    GError* tmp_err=NULL;
    int key = 0;
    gfal_fdesc_container_handle container= gfal_dir_handle_container_instance(&(handle->fdescs), &tmp_err);
    if(container)
        key = gfal_add_new_file_desc(container, (gpointer) fhandle, &tmp_err);
    G_RETURN_ERR(key, tmp_err, err);
}


DIR* gfal2_opendir(gfal2_context_t handle, const char* name, GError ** err){
    GError* tmp_err=NULL;
    gfal_file_handle ret= NULL;

    if(name == NULL || handle ==NULL){
        g_set_error(&tmp_err, 0, EFAULT, "uri  or/and handle are NULL");
    }else{
        ret = gfal_plugin_opendirG(handle, name, &tmp_err);
    }

    int key = 0;
    if(ret)
        key = gfal_rw_dir_handle_store(handle, ret, &tmp_err);

    G_RETURN_ERR( GINT_TO_POINTER(key), tmp_err, err);
}


//
//
inline static struct dirent* gfal_rw_gfalfilehandle_readdir(gfal_handle handle, gfal_file_handle fh, GError** err){
    g_return_val_err_if_fail(handle && fh, NULL, err, "[gfal_posix_gfalfilehandle_readdir] incorrect args");
    GError *tmp_err=NULL;
    struct dirent* ret = gfal_plugin_readdirG(handle, fh, &tmp_err);
    G_RETURN_ERR(ret, tmp_err, err);
}

struct dirent* gfal2_readdir(gfal2_context_t handle, DIR* dir, GError ** err){
    GError* tmp_err=NULL;
    struct dirent* res= NULL;

   if(dir == NULL || handle ==NULL){
       g_set_error(&tmp_err, 0, EFAULT, "file descriptor  or/and handle are NULL");
   }else{
       gfal_fdesc_container_handle container= gfal_dir_handle_container_instance(&(handle->fdescs), &tmp_err);
       const int key = GPOINTER_TO_INT(dir);
       gfal_file_handle fh = gfal_file_handle_bind(container, key, &tmp_err);
       if( fh != NULL){
           res = gfal_rw_gfalfilehandle_readdir(handle, fh, &tmp_err);
       }
   }
    G_RETURN_ERR(res, tmp_err, err);
}



//
//
static int gfal_rw_dir_handle_delete(gfal_fdesc_container_handle container, int key, GError** err){
    g_return_val_err_if_fail(container, -1, err, "[gfal_posix_dir_handle_delete] invalid args");
    GError *tmp_err=NULL;
    int ret = -1;
    if(container){
        ret = (gfal_remove_file_desc(container, key, &tmp_err))?0:-1;
    }
    G_RETURN_ERR(ret, tmp_err, err);
}

//
//
static int gfal_rw_dir_handle_close(gfal_handle handle, gfal_file_handle fh, GError** err){
    g_return_val_err_if_fail(handle && fh, -1, err, "[gfal_posix_gfalfilehandle_close] invalid args");
    GError *tmp_err=NULL;
    int ret = -1;

    ret = gfal_plugin_closedirG(handle, fh, &tmp_err);
    G_RETURN_ERR(ret, tmp_err, err);
}


//
//
int gfal2_closedir(gfal2_context_t handle, DIR* d, GError ** err){
    GError* tmp_err=NULL;
    int ret = -1;

    if(d == NULL || handle ==NULL){
        g_set_error(&tmp_err, 0, EFAULT, "file descriptor  or/and handle are NULL");
    }else{
        gfal_fdesc_container_handle container= gfal_dir_handle_container_instance(&(handle->fdescs), &tmp_err);
        int key = GPOINTER_TO_INT(d);
        gfal_file_handle fh = gfal_file_handle_bind(container, key, &tmp_err);
        if( fh != NULL){
            ret = gfal_rw_dir_handle_close(handle, fh, &tmp_err);
            if(ret==0){
                ret = gfal_rw_dir_handle_delete(container, key, &tmp_err);
            }
        }
    }

    G_RETURN_ERR(ret, tmp_err, err);
}
