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

#include "gfal_sftp_plugin.h"
#include "gfal_sftp_connection.h"

// These methods were introduced with version 1.0
#if LIBSSH2_VERSION_MAJOR < 1
#   define libssh2_sftp_tell64 libssh2_sftp_tell
#   define libssh2_sftp_seek64 libssh2_sftp_seek
#endif


struct gfal_sftp_file_s {
    gfal_sftp_handle_t *sftp_handle;
    LIBSSH2_SFTP_HANDLE *file_handle;
};
typedef struct gfal_sftp_file_s gfal_sftp_file_t;


static unsigned long gfal_sftp_std2ssh2_open_flags(int flag)
{
    unsigned long ssh2_flags = 0;
    if (flag & O_RDONLY || flag & O_RDWR) {
        ssh2_flags |= LIBSSH2_FXF_READ;
    }
    if (flag & O_WRONLY || flag & O_RDWR) {
        ssh2_flags |= LIBSSH2_FXF_WRITE;
    }
    if (flag & O_APPEND) {
        ssh2_flags |= LIBSSH2_FXF_APPEND;
    }
    if (flag & O_TRUNC) {
        ssh2_flags |= LIBSSH2_FXF_TRUNC;
    }
    if (flag & O_CREAT) {
        ssh2_flags |= LIBSSH2_FXF_CREAT;
    }
    if (flag & O_EXCL) {
        ssh2_flags |= LIBSSH2_FXF_EXCL;
    }
    return ssh2_flags;
}


gfal_file_handle gfal_sftp_open(plugin_handle plugin_data, const char *url, int flag, mode_t mode, GError **err)
{
    gfal_sftp_context_t *data = (gfal_sftp_context_t*)plugin_data;
    gfal_sftp_handle_t *sftp_handle = gfal_sftp_connect(data, url, err);
    if (!sftp_handle) {
        return NULL;
    }

    gfal_sftp_file_t *fd = g_malloc(sizeof(gfal_sftp_file_t));
    fd->sftp_handle = sftp_handle;

    fd->file_handle = libssh2_sftp_open(sftp_handle->sftp_session, sftp_handle->path,
        gfal_sftp_std2ssh2_open_flags(flag), mode);
    if (!fd->file_handle) {
        gfal_plugin_sftp_translate_error(__func__, sftp_handle, err);
        g_free(fd);
        gfal_sftp_release(data, sftp_handle);
        return NULL;
    }

    return gfal_file_handle_new2(gfal_sftp_plugin_get_name(), fd, NULL, url);
}


int gfal_sftp_close(plugin_handle plugin_data, gfal_file_handle fd, GError **err)
{
    gfal_sftp_context_t *data = (gfal_sftp_context_t*)plugin_data;
    gfal_sftp_file_t *ssh_fd = gfal_file_handle_get_fdesc(fd);

    libssh2_sftp_close(ssh_fd->file_handle);
    gfal_sftp_release(data, ssh_fd->sftp_handle);
    g_free(ssh_fd);

    gfal_file_handle_delete(fd);
    return 0;
}


ssize_t gfal_sftp_read(plugin_handle plugin_data, gfal_file_handle fd, void *buff, size_t count, GError **err)
{
    gfal_sftp_file_t *ssh_fd = gfal_file_handle_get_fdesc(fd);
    ssize_t read = 0;

    // libssh2 may need to read in chunks
    char *buffer = (char*)buff;
    do {
        ssize_t rc = libssh2_sftp_read(ssh_fd->file_handle, buffer + read, count - read);
        if (rc < 0) {
            gfal_plugin_sftp_translate_error(__func__, ssh_fd->sftp_handle, err);
            return rc;
        } else if (rc == 0) {
            break;
        }
        read += rc;
    } while (read < count);

    return read;
}


ssize_t gfal_sftp_write(plugin_handle plugin_data, gfal_file_handle fd, const void *buff, size_t count, GError **err)
{
    gfal_sftp_file_t *ssh_fd = gfal_file_handle_get_fdesc(fd);
    ssize_t rc = libssh2_sftp_write(ssh_fd->file_handle, buff, count);
    if (rc < 0) {
        gfal_plugin_sftp_translate_error(__func__, ssh_fd->sftp_handle, err);
    }
    // normally, one would expect to return the value from libssh2_sftp_write, but as it happens it may be
    // shorter, but still the data is cached on the pipe.
    // See https://www.libssh2.org/libssh2_sftp_write.html
    return count;
}


off_t gfal_sftp_seek(plugin_handle plugin_data, gfal_file_handle fd, off_t offset, int whence, GError **err)
{
    gfal_sftp_file_t *ssh_fd = gfal_file_handle_get_fdesc(fd);
    off_t absolute = 0;
    LIBSSH2_SFTP_ATTRIBUTES attrs;

    switch (whence) {
        case SEEK_SET:
            absolute = offset;
            break;
        case SEEK_CUR:
            absolute = libssh2_sftp_tell64(ssh_fd->file_handle) + offset;
            break;
        case SEEK_END:
            if (libssh2_sftp_fstat(ssh_fd->file_handle, &attrs) < 0) {
                gfal_plugin_sftp_translate_error(__func__, ssh_fd->sftp_handle, err);
                return -1;
            }
            absolute = attrs.filesize + offset;
    }
    libssh2_sftp_seek64(ssh_fd->file_handle, absolute);
    return absolute;
}
