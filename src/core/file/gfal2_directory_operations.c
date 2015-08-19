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

#include <regex.h>
#include <string.h>
#include <file/gfal_file_api.h>

#include <common/future/glib.h>
#include <common/gfal_constants.h>
#include <common/gfal_types.h>
#include <common/gfal_common_plugin.h>
#include <common/gfal_common_internal.h>
#include <common/gfal_common_err_helpers.h>
#include <common/gfal_common_filedescriptor.h>
#include <common/gfal_common_dir_handle.h>
#include <logger/gfal_logger.h>
#include <cancel/gfal_cancel.h>

//
// Mapping for the gfal2  opendir/readdir/closedir operations
//
// @author : Devresse Adrien

#ifdef __APPLE__ 
static char* mempcpy(char* dst, const char* src, size_t len)
{
    memcpy(src, dst, len);
    return dst + len;
}
#endif


inline static int gfal_rw_dir_handle_store(gfal2_context_t handle,
        gfal_file_handle fhandle, GError** err)
{
    g_return_val_err_if_fail(handle && fhandle, 0, err,
            "[gfal_rw_dir_handle_store] handle invalid");
    GError* tmp_err = NULL;
    int key = 0;
    gfal_fdesc_container_handle container = gfal_dir_handle_container_instance(
            &(handle->fdescs), &tmp_err);
    if (container)
        key = gfal_add_new_file_desc(container, (gpointer) fhandle, &tmp_err);
    G_RETURN_ERR(key, tmp_err, err);
}


DIR* gfal2_opendir(gfal2_context_t handle, const char* name, GError ** err)
{
    GError* tmp_err = NULL;
    gfal_file_handle ret = NULL;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, NULL, err);
    if (name == NULL || handle == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT,
                "uri  or/and handle are NULL");
    }
    else {
        ret = gfal_plugin_opendirG(handle, name, &tmp_err);
    }

    int key = 0;
    if (ret)
        key = gfal_rw_dir_handle_store(handle, ret, &tmp_err);
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(GINT_TO_POINTER(key), tmp_err, err);
}


inline static struct dirent* gfal_rw_gfalfilehandle_readdir(
        gfal2_context_t handle, gfal_file_handle fh, GError** err)
{
    g_return_val_err_if_fail(handle && fh, NULL, err,
            "[gfal_posix_gfalfilehandle_readdir] incorrect args");
    GError *tmp_err = NULL;
    struct dirent* ret = gfal_plugin_readdirG(handle, fh, &tmp_err);
    G_RETURN_ERR(ret, tmp_err, err);
}


struct dirent* gfal2_readdir(gfal2_context_t handle, DIR* dir, GError ** err)
{
    GError* tmp_err = NULL;
    struct dirent* res = NULL;
    GFAL2_BEGIN_SCOPE_CANCEL(handle, NULL, err);
    if (dir == NULL || handle == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT,
                "file descriptor or/and handle are NULL");
    }
    else {
        gfal_fdesc_container_handle container =
                gfal_dir_handle_container_instance(&(handle->fdescs), &tmp_err);
        const int key = GPOINTER_TO_INT(dir);
        gfal_file_handle fh = gfal_file_handle_bind(container, key, &tmp_err);
        if (fh != NULL) {
            res = gfal_rw_gfalfilehandle_readdir(handle, fh, &tmp_err);
        }
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(res, tmp_err, err);
}


static size_t gfal_rw_get_root_length(const char* surl)
{
    regex_t rx;
    regmatch_t matches[1];
    regcomp(&rx, "(\\w+://[^/]*/)", REG_EXTENDED);
    regexec(&rx, surl, 1, matches, 0);
    return matches[0].rm_eo - matches[0].rm_so;
}

inline static struct dirent* gfal_rw_gfalfilehandle_readdirpp(gfal2_context_t context, gfal_file_handle fh, struct stat* st, GError** err)
{
    g_return_val_err_if_fail(context && fh, NULL, err, "[gfal_posix_gfalfilehandle_readdirpp] incorrect args");
    GError *tmp_err=NULL;
    struct dirent* ret = gfal_plugin_readdirppG(context, fh, st, &tmp_err);

    // try to simulate readdirpp
    if (tmp_err && tmp_err->code == EPROTONOSUPPORT && fh->path != NULL ) {
        g_clear_error(&tmp_err);
        ret = gfal_rw_gfalfilehandle_readdir(context, fh, &tmp_err);
        if (!tmp_err && ret != NULL ) {
            const size_t s_path = strlen(fh->path);
            const size_t s_d_name = strlen(ret->d_name);
            char buffer[s_d_name + s_path + 2];

            if (ret->d_name[0] != '/') {
                char* p = mempcpy(buffer, fh->path, s_path);
                *p = '/';
                p = mempcpy(++p, ret->d_name, s_d_name);
                *p = '\0';
            }
            else {
                size_t root_len = gfal_rw_get_root_length(fh->path);
                if (root_len > 0)
                    mempcpy(buffer, fh->path, root_len);
                mempcpy(buffer + root_len, ret->d_name, sizeof(buffer) - root_len);
            }

            if (gfal2_stat(context, buffer, st, &tmp_err) < 0) {
                ret = NULL;
            }
        }
    }

    G_RETURN_ERR(ret, tmp_err, err);
}


struct dirent* gfal2_readdirpp(gfal2_context_t context, DIR* dir,
        struct stat* st, GError ** err)
{
    GError* tmp_err = NULL;
    struct dirent* res = NULL;
    GFAL2_BEGIN_SCOPE_CANCEL(context, NULL, err);
    if (dir == NULL || context == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT,
                "file descriptor or/and handle are NULL");
    }
    else {
        gfal_fdesc_container_handle container =
                gfal_dir_handle_container_instance(&(context->fdescs), &tmp_err);
        const int key = GPOINTER_TO_INT(dir);
        gfal_file_handle fh = gfal_file_handle_bind(container, key, &tmp_err);
        if (fh != NULL) {
            res = gfal_rw_gfalfilehandle_readdirpp(context, fh, st, &tmp_err);
        }
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}


static int gfal_rw_dir_handle_delete(gfal_fdesc_container_handle container,
        int key, GError** err)
{
    g_return_val_err_if_fail(container, -1, err,
            "[gfal_posix_dir_handle_delete] invalid args");
    GError *tmp_err = NULL;
    int ret = -1;
    if (container) {
        ret = (gfal_remove_file_desc(container, key, &tmp_err)) ? 0 : -1;
    }
    G_RETURN_ERR(ret, tmp_err, err);
}


static int gfal_rw_dir_handle_close(gfal2_context_t handle, gfal_file_handle fh,
        GError** err)
{
    g_return_val_err_if_fail(handle && fh, -1, err,
            "[gfal_posix_gfalfilehandle_close] invalid args");
    GError *tmp_err = NULL;
    int ret = -1;

    ret = gfal_plugin_closedirG(handle, fh, &tmp_err);
    G_RETURN_ERR(ret, tmp_err, err);
}


int gfal2_closedir(gfal2_context_t handle, DIR* d, GError ** err)
{
    GError* tmp_err = NULL;
    int ret = -1;

    if (d == NULL || handle == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT,
                "file descriptor or/and handle are NULL");
    }
    else {
        gfal_fdesc_container_handle container =
                gfal_dir_handle_container_instance(&(handle->fdescs), &tmp_err);
        int key = GPOINTER_TO_INT(d);
        gfal_file_handle fh = gfal_file_handle_bind(container, key, &tmp_err);
        if (fh != NULL) {
            ret = gfal_rw_dir_handle_close(handle, fh, &tmp_err);
            if (ret == 0) {
                ret = gfal_rw_dir_handle_delete(container, key, &tmp_err);
            }
        }
    }

    G_RETURN_ERR(ret, tmp_err, err);
}
