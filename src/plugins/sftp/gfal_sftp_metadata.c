/*
 * Copyright (c) CERN 2016
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

#include <uri/gfal2_uri.h>
#include "gfal_sftp_plugin.h"
#include "gfal_sftp_connection.h"

void gfal_sftp_fill_stat(struct stat *st, LIBSSH2_SFTP_ATTRIBUTES *attrs)
{
    if (attrs->flags & LIBSSH2_SFTP_ATTR_SIZE) {
        st->st_size = attrs->filesize;
    }
    if (attrs->flags & LIBSSH2_SFTP_ATTR_UIDGID) {
        st->st_uid = attrs->uid;
        st->st_gid = attrs->gid;
    }
    if (attrs->flags & LIBSSH2_SFTP_ATTR_PERMISSIONS) {
        st->st_mode = attrs->permissions;
    }
    if (attrs->flags & LIBSSH2_SFTP_ATTR_ACMODTIME) {
        st->st_atime = attrs->atime;
        st->st_mtime = attrs->mtime;
    }
}


int gfal_sftp_stat(plugin_handle plugin_data, const char *url, struct stat *buf, GError **err)
{
    gfal_sftp_context_t *data = (gfal_sftp_context_t*)plugin_data;
    gfal_sftp_handle_t *sftp_handle = gfal_sftp_connect(data, url, err);
    if (!sftp_handle) {
        return -1;
    }

    LIBSSH2_SFTP_ATTRIBUTES attrs;
    int rc = libssh2_sftp_stat(sftp_handle->sftp_session, sftp_handle->path, &attrs);

    if (rc < 0) {
        gfal_plugin_sftp_translate_error(__func__, sftp_handle, err);
    } else {
        gfal_sftp_fill_stat(buf, &attrs);
    }

    gfal_sftp_release(data, sftp_handle);
    return rc;
}


int gfal_sftp_unlink(plugin_handle plugin_data, const char *url, GError **err)
{
    gfal_sftp_context_t *data = (gfal_sftp_context_t*)plugin_data;
    gfal_sftp_handle_t *sftp_handle = gfal_sftp_connect(data, url, err);
    if (!sftp_handle) {
        return -1;
    }

    int rc = libssh2_sftp_unlink(sftp_handle->sftp_session, sftp_handle->path);
    if (rc < 0) {
        gfal_plugin_sftp_translate_error(__func__, sftp_handle, err);
    }

    gfal_sftp_release(data, sftp_handle);
    return rc;
}


int gfal_sftp_rename(plugin_handle plugin_data, const char *oldurl, const char *urlnew, GError **err)
{
    gfal_sftp_context_t *data = (gfal_sftp_context_t*)plugin_data;
    gfal_sftp_handle_t *sftp_handle = gfal_sftp_connect(data, oldurl, err);
    if (!sftp_handle) {
        return -1;
    }

    int rc = -1;
    gfal2_uri *new_parsed = gfal2_parse_uri(urlnew, err);
    if (new_parsed) {
        rc = libssh2_sftp_rename(sftp_handle->sftp_session, sftp_handle->path, new_parsed->path);
        if (rc < 0) {
            gfal_plugin_sftp_translate_error(__func__, sftp_handle, err);
            if ((*err)->code == 4) {
                (*err)->code = EISDIR;
            }
        }
    }

    gfal2_free_uri(new_parsed);
    gfal_sftp_release(data, sftp_handle);
    return rc;
}


int gfal_sftp_mkdir(plugin_handle plugin_data, const char *url, mode_t mode, gboolean rec_flag, GError **err)
{
    gfal_sftp_context_t *data = (gfal_sftp_context_t*)plugin_data;
    gfal_sftp_handle_t *sftp_handle = gfal_sftp_connect(data, url, err);
    if (!sftp_handle) {
        return -1;
    }

    int rc = libssh2_sftp_mkdir(sftp_handle->sftp_session, sftp_handle->path, mode);
    if (rc < 0) {
        gfal_plugin_sftp_translate_error(__func__, sftp_handle, err);
        if ((*err)->code == 4) {
            (*err)->code = EEXIST;
        }
    }

    gfal_sftp_release(data, sftp_handle);
    return rc;
}


int gfal_sftp_rmdir(plugin_handle plugin_data, const char *url, GError **err)
{
    LIBSSH2_SFTP_ATTRIBUTES attrs;
    gfal_sftp_context_t *data = (gfal_sftp_context_t*)plugin_data;
    gfal_sftp_handle_t *sftp_handle = gfal_sftp_connect(data, url, err);
    if (!sftp_handle) {
        return -1;
    }

    int rc = libssh2_sftp_rmdir(sftp_handle->sftp_session, sftp_handle->path);
    if (rc < 0) {
        gfal_plugin_sftp_translate_error(__func__, sftp_handle, err);
        // Need to patch some error codes
        switch ((*err)->code) {
            case 4:
                (*err)->code = ENOTEMPTY;
                break;
            case 3:
                (*err)->code = EACCES;
                break;
            case 2:
                // Some times return ENOENT when actually it exists, but it is a file
                if (libssh2_sftp_stat(sftp_handle->sftp_session, sftp_handle->path, &attrs) == 0) {
                    (*err)->code = ENOTDIR;
                }
                break;
        }
    }

    gfal_sftp_release(data, sftp_handle);
    return rc;
}


int gfal_sftp_symlink(plugin_handle plugin_data, const char *oldurl, const char *urlnew, GError **err)
{
    gfal_sftp_context_t *data = (gfal_sftp_context_t*)plugin_data;
    gfal_sftp_handle_t *sftp_handle = gfal_sftp_connect(data, oldurl, err);
    if (!sftp_handle) {
        return -1;
    }

    int rc = -1;
    gfal2_uri *new_parsed = gfal2_parse_uri(urlnew, err);
    if (new_parsed) {
        rc = libssh2_sftp_symlink(sftp_handle->sftp_session, sftp_handle->path, new_parsed->path);
        if (rc < 0) {
            gfal_plugin_sftp_translate_error(__func__, sftp_handle, err);
        }
    }

    gfal2_free_uri(new_parsed);
    gfal_sftp_release(data, sftp_handle);
    return rc;
}


ssize_t gfal_sftp_readlink(plugin_handle plugin_data, const char *url, char *buff, size_t buffsiz, GError **err)
{
    gfal_sftp_context_t *data = (gfal_sftp_context_t*)plugin_data;
    gfal_sftp_handle_t *sftp_handle = gfal_sftp_connect(data, url, err);
    if (!sftp_handle) {
        return -1;
    }

    int rc = libssh2_sftp_readlink(sftp_handle->sftp_session, sftp_handle->path, buff, buffsiz);
    if (rc < 0) {
        gfal_plugin_sftp_translate_error(__func__, sftp_handle, err);
    }

    gfal_sftp_release(data, sftp_handle);
    return rc;
}


int	gfal_sftp_chmod(plugin_handle plugin_data, const char * url, mode_t mode, GError** err)
{
    gfal_sftp_context_t *data = (gfal_sftp_context_t*)plugin_data;
    gfal_sftp_handle_t *sftp_handle = gfal_sftp_connect(data, url, err);
    if (!sftp_handle) {
        return -1;
    }

    LIBSSH2_SFTP_ATTRIBUTES attrs = {0};
    attrs.flags = LIBSSH2_SFTP_ATTR_PERMISSIONS;
    attrs.permissions = mode;

    int rc = libssh2_sftp_stat_ex(sftp_handle->sftp_session,
        sftp_handle->path, strlen(sftp_handle->path),
        LIBSSH2_SFTP_SETSTAT, &attrs);
    if (rc < 0) {
        gfal_plugin_sftp_translate_error(__func__, sftp_handle, err);
    }

    gfal_sftp_release(data, sftp_handle);
    return rc;
}
