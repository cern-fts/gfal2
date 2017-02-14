/*
 * Copyright (c) CERN 2013-2015
 *
 * Copyright (c) Members of the EMI Collaboration. 2010-2013
 *  See  http://www.eu-emi.eu/partners for details on the copyright
 *  holders.
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

#include <common/gfal_handle.h>
#include <common/gfal_common_err_helpers.h>
#include <common/gfal_common_filedescriptor.h>
#include <common/gfal_cancel.h>


/*
 *  store a gfal_file_handle in the base, in a key/value model
 *  the key, else 0 if error occured and err is set correctly
 */
static int gfal_rw_file_handle_store(gfal2_context_t handle, gfal_file_handle fhandle, GError **err)
{
    g_return_val_err_if_fail(handle && fhandle, -1, err, "[gfal_rw_file_handle_store] invalid args");
    GError *tmp_err = NULL;
    int key = 0;
    key = gfal_add_new_file_desc(handle->fdescs, (gpointer) fhandle, &tmp_err);
    G_RETURN_ERR(key, tmp_err, err);
}


int gfal2_open(gfal2_context_t handle, const char *uri, int flag, GError **err)
{
    return gfal2_open2(handle, uri, flag, (S_IRWXU | S_IRGRP | S_IROTH), err);
}


int gfal2_open2(gfal2_context_t handle, const char *uri, int flag, mode_t mode, GError **err)
{
    GError *tmp_err = NULL;
    gfal_file_handle fhandle = NULL;
    int key = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    gfal2_log(G_LOG_LEVEL_DEBUG, "%s ->", __func__);

    if (uri == NULL || handle == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "name is empty");
    }
    else {
        fhandle = gfal_plugin_openG(handle, uri, flag, mode, &tmp_err);
    }

    if (fhandle) {
        key = gfal_rw_file_handle_store(handle, fhandle, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(key, tmp_err, err);
}


int gfal2_creat(gfal2_context_t handle, const char *filename, mode_t mode, GError **err)
{
    return (gfal2_open2(handle, filename, (O_WRONLY | O_CREAT | O_TRUNC), mode, err));
}


ssize_t gfal2_read(gfal2_context_t handle, int fd, void *buff, size_t s_buff, GError **err)
{
    GError *tmp_err = NULL;
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if (fd <= 0 || handle == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EBADF, "Incorrect file descriptor or incorrect handle");
    }
    else {
        const int key = fd;
        gfal_file_handle fh = gfal_file_handle_bind(handle->fdescs, key, &tmp_err);
        if (fh != NULL) {
            res = gfal_plugin_readG(handle, fh, buff, s_buff, &tmp_err);
        }
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal2_close(gfal2_context_t handle, int fd, GError **err)
{
    GError *tmp_err = NULL;
    int ret = -1;

    if (fd <= 0 || handle == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EBADF, "Incorrect file descriptor or incorrect handle");
    }
    else {
        int key = GPOINTER_TO_INT(fd);
        gfal_file_handle fh = gfal_file_handle_bind(handle->fdescs, key, &tmp_err);
        if (fh != NULL) {
            ret = gfal_plugin_closeG(handle, fh, &tmp_err);
            if (ret == 0) {
                ret = (gfal_remove_file_desc(handle->fdescs, key, &tmp_err)) ? 0 : -1;
            }
        }
    }
    G_RETURN_ERR(ret, tmp_err, err);
}


off_t gfal2_lseek(gfal2_context_t handle, int fd, off_t offset, int whence, GError **err)
{
    GError *tmp_err = NULL;
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if (fd <= 0 || handle == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EBADF, "Incorrect file descriptor");
    }
    else {
        const int key = fd;
        gfal_file_handle fh = gfal_file_handle_bind(handle->fdescs, key, &tmp_err);
        if (fh != NULL) {
            res = gfal_plugin_lseekG(handle, fh, offset, whence, &tmp_err);
        }
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal2_flush(gfal2_context_t handle, int fd, GError **err)
{
    return 0;
}


ssize_t gfal2_pread(gfal2_context_t handle, int fd, void *buff, size_t s_buff, off_t offset, GError **err)
{
    GError *tmp_err = NULL;
    ssize_t res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if (fd <= 0 || handle == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EBADF, "Incorrect file descriptor or incorrect handle");
    }
    else {
        const int key = fd;
        gfal_file_handle fh = gfal_file_handle_bind(handle->fdescs, key, &tmp_err);
        if (fh != NULL) {
            res = gfal_plugin_preadG(handle, fh, buff, s_buff, offset, &tmp_err);
        }
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(res, tmp_err, err);
}


ssize_t gfal2_write(gfal2_context_t handle, int fd, const void *buff, size_t s_buff, GError **err)
{
    GError *tmp_err = NULL;
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if (fd <= 0 || handle == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EBADF, "Incorrect file descriptor or incorrect handle");
    }
    else {
        const int key = fd;
        gfal_file_handle fh = gfal_file_handle_bind(handle->fdescs, key, &tmp_err);
        if (fh != NULL) {
            res = gfal_plugin_writeG(handle, fh, (void *) buff, s_buff, &tmp_err);
        }
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(res, tmp_err, err);
}


ssize_t gfal2_pwrite(gfal2_context_t handle, int fd, const void *buff, size_t s_buff, off_t offset, GError **err)
{
    GError *tmp_err = NULL;
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    if (fd <= 0) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EBADF, "Incorrect file descriptor");
    }
    else {
        const int key = fd;
        gfal_file_handle fh = gfal_file_handle_bind(handle->fdescs, key, &tmp_err);
        if (fh != NULL) {
            res = gfal_plugin_pwriteG(handle, fh, (void *) buff, s_buff, offset, &tmp_err);
        }
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(res, tmp_err, err);
}
