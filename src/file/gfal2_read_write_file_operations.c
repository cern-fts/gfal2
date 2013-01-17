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
#include <common/gfal_common_internal.h>
#include <common/gfal_common_plugin.h>
#include <common/gfal_common_errverbose.h>
#include <common/gfal_common_filedescriptor.h>
#include <common/gfal_common_file_handle.h>



//
// Mapping for the gfal 2.0  open/read/write/close opeations of the file interface
//
// @author : Devresse Adrien
//
//


/*
 *  store a gfal_file_handle in the base, in a key/value model
 *  the key, else 0 if error occured and err is set correctly
 */
static int gfal_rw_file_handle_store(gfal_handle handle, gfal_file_handle fhandle, GError** err){
    g_return_val_err_if_fail( handle && fhandle, -1, err, "[gfal_rw_file_handle_store] invalid args");
    GError* tmp_err=NULL;
    int key=0;
    gfal_fdesc_container_handle container= gfal_file_handle_container_instance(&(handle->fdescs), &tmp_err);
    if(container)
        key = gfal_add_new_file_desc(container, (gpointer) fhandle, &tmp_err);
   G_RETURN_ERR(key, tmp_err, err);
}


int gfal2_open(gfal2_context_t handle, const char * uri, int flag, GError ** err){
    return gfal2_open2(handle, uri, flag, (S_IRWXU | S_IRGRP | S_IROTH), err);
}

int gfal2_open2(gfal2_context_t handle, const char * uri, int flag, mode_t mode, GError ** err){
    GError* tmp_err=NULL;
    gfal_file_handle fhandle=NULL;
    int key = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    gfal_log(GFAL_VERBOSE_TRACE, "%s ->",__func__);

    if(uri == NULL || handle == NULL){
        g_set_error(&tmp_err, 0, EFAULT, "name is empty");
    }else{
        fhandle = gfal_plugin_openG(handle, uri, flag, mode, &tmp_err);
    }

    if(fhandle)
        key = gfal_rw_file_handle_store(handle, fhandle, &tmp_err);
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(key, tmp_err, err);
}



int gfal2_creat (gfal2_context_t handle, const char *filename, mode_t mode, GError ** err){
    return (gfal2_open2 (handle, filename, (O_WRONLY|O_CREAT|O_TRUNC), mode, err));
}

/*
 *  map the file handle to the correct call
 */
inline int gfal_rw_gfalfilehandle_read(gfal_handle handle, gfal_file_handle fh, void* buff, size_t s_buff, GError** err){
    g_return_val_err_if_fail(handle && fh, -1, err, "[gfal_rw_gfalfilehandle_read] incorrect args");
    GError *tmp_err=NULL;
    int ret = gfal_plugin_readG(handle, fh, buff, s_buff, &tmp_err);

    G_RETURN_ERR(ret, tmp_err, err);
}

ssize_t gfal2_read(gfal2_context_t handle, int fd, void* buff, size_t s_buff, GError ** err){
    GError* tmp_err=NULL;
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if(fd <=0 || handle == NULL){
       g_set_error(&tmp_err, 0, EBADF, "Incorrect file descriptor or incorrect handle");
    }else{
       gfal_fdesc_container_handle container= gfal_file_handle_container_instance(&(handle->fdescs), &tmp_err);
       const int key = fd;
       gfal_file_handle fh = gfal_file_handle_bind(container, key, &tmp_err);
       if( fh != NULL){
           res = gfal_rw_gfalfilehandle_read(handle, fh, buff, s_buff, &tmp_err);
       }
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(res, tmp_err, err);
}


static int gfal_rw_file_handle_delete(gfal_fdesc_container_handle container, int key, GError** err){
    g_return_val_err_if_fail(container, -1, err, "[gfal_rw_dir_handle_delete] invalid args");
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

static int gfal_rw_file_handle_close(gfal_handle handle, gfal_file_handle fhandle, GError ** err){
    GError* tmp_err=NULL;
    int ret = -1;

    ret = gfal_plugin_closeG(handle, fhandle, &tmp_err);

    if(tmp_err){
        g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
    }
    return ret;

}


int gfal2_close(gfal2_context_t handle, int fd, GError ** err){
    GError* tmp_err=NULL;
    int ret = -1;

    if(fd <=0 || handle == NULL){
       g_set_error(&tmp_err, 0, EBADF, "Incorrect file descriptor or incorrect handle");
    }else{
        gfal_fdesc_container_handle container= gfal_file_handle_container_instance(&(handle->fdescs), &tmp_err);
        int key = GPOINTER_TO_INT(fd);
        gfal_file_handle fh = gfal_file_handle_bind(container, key, &tmp_err);
        if( fh != NULL){
            ret = gfal_rw_file_handle_close(handle, fh, &tmp_err);
            if(ret==0){
                ret = gfal_rw_file_handle_delete(container, key, &tmp_err);
            }
        }
    }
    G_RETURN_ERR(ret, tmp_err, err);
}


static int gfal_rw_gfalfilehandle_lseek(gfal_handle handle, gfal_file_handle fh, off_t offset, int whence, GError** err){
    GError *tmp_err=NULL;
    int ret  = gfal_plugin_lseekG(handle, fh, offset, whence, &tmp_err);
    G_RETURN_ERR(ret, tmp_err, err);
}

off_t gfal2_lseek (gfal2_context_t handle, int fd, off_t offset, int whence, GError ** err){
    GError* tmp_err=NULL;
    int res= -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if(fd <=0 || handle == NULL){
        g_set_error(&tmp_err, 0, EBADF, "Incorrect file descriptor");
    }else{
        gfal_fdesc_container_handle container= gfal_file_handle_container_instance(&(handle->fdescs), &tmp_err);
        const int key = fd;
        gfal_file_handle fh = gfal_file_handle_bind(container, key, &tmp_err);
        if( fh != NULL){
            res = gfal_rw_gfalfilehandle_lseek(handle, fh, offset, whence, &tmp_err);
        }
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal2_flush(gfal2_context_t handle, int fd, GError ** err){
    return 0;
}

inline ssize_t gfal_rw_gfalfilehandle_pread(gfal_handle handle, gfal_file_handle fh, void* buff,
                        size_t s_buff, off_t offset, GError** err){
    GError *tmp_err=NULL;
    ssize_t ret = gfal_plugin_preadG(handle, fh, buff, s_buff, offset, &tmp_err);

   G_RETURN_ERR(ret, tmp_err, err);
}

ssize_t gfal2_pread(gfal2_context_t handle, int fd, void * buff, size_t s_buff, off_t offset, GError ** err){
    GError* tmp_err=NULL;
    ssize_t res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if(fd <=0 || handle == NULL){
       g_set_error(&tmp_err, 0, EBADF, "Incorrect file descriptor or incorrect handle");
    }else{
       gfal_fdesc_container_handle container= gfal_file_handle_container_instance(&(handle->fdescs), &tmp_err);
       const int key = fd;
       gfal_file_handle fh = gfal_file_handle_bind(container, key, &tmp_err);
       if( fh != NULL){
           res = gfal_rw_gfalfilehandle_pread(handle, fh, buff, s_buff, offset, &tmp_err);
       }
   }
   GFAL2_END_SCOPE_CANCEL(handle);
   G_RETURN_ERR(res, tmp_err, err);
}


int gfal_rw_gfalfilehandle_write(gfal_handle handle, gfal_file_handle fh, const void* buff, size_t s_buff, GError** err){
    GError *tmp_err=NULL;
    int ret = gfal_plugin_writeG(handle, fh, (void*) buff, s_buff, &tmp_err);
   G_RETURN_ERR(ret, tmp_err, err);
}

ssize_t gfal2_write(gfal2_context_t handle, int fd, const void *buff, size_t s_buff, GError ** err){
    GError* tmp_err=NULL;
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if(fd <=0 || handle == NULL){
       g_set_error(&tmp_err, 0, EBADF, "Incorrect file descriptor or incorrect handle");
    }else{
        gfal_fdesc_container_handle container= gfal_file_handle_container_instance(&(handle->fdescs), &tmp_err);
        const int key = fd;
        gfal_file_handle fh = gfal_file_handle_bind(container, key, &tmp_err);
        if( fh != NULL){
            res = gfal_rw_gfalfilehandle_write(handle, fh, buff, s_buff, &tmp_err);
        }
   }
   GFAL2_END_SCOPE_CANCEL(handle);
   G_RETURN_ERR(res, tmp_err, err);
}


/*
 *  map the file handle to the correct call for pwrite
 */
int gfal_rw_gfalfilehandle_pwrite(gfal_handle handle, gfal_file_handle fh, const void* buff, size_t s_buff, off_t offset, GError** err){
    g_return_val_err_if_fail(handle && fh, -1, err, "[gfal_posix_gfalfilehandle_pwrite] incorrect args");
    GError *tmp_err=NULL;
    int ret = gfal_plugin_pwriteG(handle, fh, (void*) buff, s_buff, offset, &tmp_err);
   G_RETURN_ERR(ret, tmp_err, err);
}

ssize_t gfal2_pwrite(gfal2_context_t handle, int fd, const void * buff, size_t s_buff, off_t offset, GError ** err){
    GError* tmp_err=NULL;
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if(fd <=0){
       g_set_error(&tmp_err, 0, EBADF, "Incorrect file descriptor");
    }else{
       gfal_fdesc_container_handle container= gfal_file_handle_container_instance(&(handle->fdescs), &tmp_err);
       const int key = fd;
       gfal_file_handle fh = gfal_file_handle_bind(container, key, &tmp_err);
       if( fh != NULL){
           res = gfal_rw_gfalfilehandle_pwrite(handle, fh, buff, s_buff, offset, &tmp_err);
       }
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(res, tmp_err, err);
}


