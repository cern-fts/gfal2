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

#include <string.h>
#include <glib/gthread.h>
#include <gfal_api.h>

#include "gfal_posix_api.h"

/**
 * Context per thread
 */
typedef struct _Gfal2ThreadContext {
    gfal2_context_t context;
    GError *error;
} Gfal2ThreadContext;

/** Release the context bound to a thread when the later ends */
static void gfal_posix_free_context(gpointer ptr)
{
    Gfal2ThreadContext *thread_context = (Gfal2ThreadContext*)ptr;
    g_error_free(thread_context->error);
    gfal2_context_free(thread_context->context);
    g_free(thread_context);
}

/** Thread local storage */
static GPrivate *thread_private;

__attribute__((constructor))
static void init_thread_private()
{
    g_thread_init(NULL);
    thread_private = g_private_new(gfal_posix_free_context);
}


/** Return the context for the current thread */
static Gfal2ThreadContext* gfal_posix_get_thread_context()
{
    Gfal2ThreadContext *thread_context = g_private_get(thread_private);
    if (thread_context == NULL) {
        thread_context = g_malloc0(sizeof(Gfal2ThreadContext));
        g_private_set(thread_private, thread_context);
    }
    return thread_context;
}


/** Return the gfal2 handle for the current thread */
gfal2_context_t gfal_posix_get_handle()
{
    errno = 0;
    Gfal2ThreadContext *thread_context = gfal_posix_get_thread_context();
    if (thread_context->context == NULL) {
        thread_context->context = gfal2_context_new(&thread_context->error);
        if (thread_context->error != NULL) {
            errno = thread_context->error->code;
        }
    }
    return thread_context->context;
}


/**
 * Register the last error in the handle and display a warning if an error was registered and not deleted
 */
static void gfal_posix_register_internal_error(const char *prefix, GError *tmp_err)
{
    Gfal2ThreadContext *thread_context = gfal_posix_get_thread_context();

    if (thread_context->error != NULL) {
        gfal2_log(G_LOG_LEVEL_WARNING,
            "[%s] Warning : existing registered error replaced! old error: %s ",
            prefix, thread_context->error->message);
        g_clear_error(&thread_context->error);
    }
    gfal2_propagate_prefixed_error(&thread_context->error, tmp_err, prefix);
    errno = tmp_err->code;
}


void gfal_posix_clear_error()
{
    g_clear_error(&gfal_posix_get_thread_context()->error);
    errno = 0;
}


int gfal_posix_code_error()
{
    GError *err = gfal_posix_get_thread_context()->error;
    if (err == NULL) {
        return 0;
    }
    return err->code;
}


int gfal_posix_check_error()
{
    GError *err = gfal_posix_get_thread_context()->error;
    if (err != NULL) {
        g_printerr("[gfal] %s\n", err->message);
        return 1;
    }
    return 0;
}


char *gfal_posix_strerror_r(char *buff_err, size_t s_err)
{
    GError *err = gfal_posix_get_thread_context()->error;
    if (err == NULL) {
        strerror_r(0, buff_err, s_err);
    }
    else {
        snprintf(buff_err, s_err, "[gfal] %s", err->message);
    }
    return buff_err;
}

// Avoid deprecation warning
static void _gfal_posix_print_error()
{
    GError *err = gfal_posix_get_thread_context()->error;
    if (err != NULL ) {
        g_printerr("[gfal]%s\n", err->message);
    }
    else if (errno != 0) {
        char *sterr = strerror(errno);
        g_printerr("[gfal] errno reported by external lib: %s", sterr);
    }
    else {
        g_printerr("[gfal] No gfal error reported\n");
    }
}


void gfal_posix_print_error()
{
    _gfal_posix_print_error();
}


void gfal_posix_release_error()
{
    _gfal_posix_print_error();
    gfal_posix_clear_error();
}


int gfal_access(const char *path, int mode)
{
    GError *tmp_err = NULL;
    gfal2_context_t handle;

    if ((handle = gfal_posix_get_handle()) == NULL) {
        return -1;
    }

    int ret = gfal2_access(handle, path, mode, &tmp_err);
    if (ret < 0) {
        gfal_posix_register_internal_error(__func__, tmp_err);
    }
    return ret;
}


int gfal_chmod(const char *path, mode_t mode)
{
    GError *tmp_err = NULL;
    gfal2_context_t handle;

    if ((handle = gfal_posix_get_handle()) == NULL) {
        return -1;
    }

    int ret = gfal2_chmod(handle, path, mode, &tmp_err);
    if (tmp_err) {
        gfal_posix_register_internal_error(__func__, tmp_err);
    }
    return ret;
}


int gfal_rename(const char *oldpath, const char *newpath)
{
    GError *tmp_err = NULL;
    gfal2_context_t handle;

    if ((handle = gfal_posix_get_handle()) == NULL) {
        return -1;
    }

    int ret = gfal2_rename(handle, oldpath, newpath, &tmp_err);
    if (tmp_err) {
        gfal_posix_register_internal_error(__func__, tmp_err);
    }
    return ret;
}


int gfal_stat(const char *path, struct stat *buff)
{
    gfal2_context_t handle;
    GError *tmp_err = NULL;

    if ((handle = gfal_posix_get_handle()) == NULL) {
        return -1;
    }

    int ret = gfal2_stat(handle, path, buff, &tmp_err);
    if (tmp_err) {
        gfal_posix_register_internal_error(__func__, tmp_err);
    }
    return ret;
}


int gfal_lstat(const char *path, struct stat *buff)
{
    gfal2_context_t handle;
    GError *tmp_err = NULL;

    if ((handle = gfal_posix_get_handle()) == NULL) {
        return -1;
    }

    int ret = gfal2_lstat(handle, path, buff, &tmp_err);
    if (ret) {
        gfal_posix_register_internal_error(__func__, tmp_err);
    }
    return ret;
}


int gfal_mkdirp(const char *path, mode_t mode)
{
    GError *tmp_err = NULL;
    gfal2_context_t handle;

    if ((handle = gfal_posix_get_handle()) == NULL) {
        return -1;
    }

    int ret = gfal2_mkdir(handle, path, mode, &tmp_err);
    if (tmp_err) {
        gfal_posix_register_internal_error(__func__, tmp_err);
        errno = tmp_err->code;
    }
    return ret;
}


int gfal_mkdir(const char *path, mode_t mode)
{
    return gfal_mkdirp(path, mode);
}


int gfal_rmdir(const char *path)
{
    GError *tmp_err = NULL;
    gfal2_context_t handle;

    if ((handle = gfal_posix_get_handle()) == NULL) {
        return -1;
    }

    int ret = gfal2_rmdir(handle, path, &tmp_err);
    if (tmp_err) {
        gfal_posix_register_internal_error(__func__, tmp_err);
    }
    return ret;
}


DIR *gfal_opendir(const char *name)
{
    GError *tmp_err = NULL;
    gfal2_context_t handle;

    if ((handle = gfal_posix_get_handle()) == NULL) {
        return NULL;
    }

    DIR *dir = gfal2_opendir(handle, name, &tmp_err);

    if (tmp_err) {
        gfal_posix_register_internal_error(__func__, tmp_err);
    }
    return dir;
}


struct dirent *gfal_readdir(DIR *d)
{
    GError *tmp_err = NULL;
    gfal2_context_t handle;

    if ((handle = gfal_posix_get_handle()) == NULL) {
        return NULL;
    }

    struct dirent *res = gfal2_readdir(handle, d, &tmp_err);
    if (tmp_err) {
        gfal_posix_register_internal_error(__func__, tmp_err);
    }
    return res;
}


int gfal_closedir(DIR *d)
{
    GError *tmp_err = NULL;
    gfal2_context_t handle;
    if ((handle = gfal_posix_get_handle()) == NULL) {
        return -1;
    }

    if (d == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "File descriptor is NULL");
        gfal_posix_register_internal_error(__func__, tmp_err);
        return -1;
    }

    int ret = gfal2_closedir(handle, d, &tmp_err);
    if (tmp_err) {
        gfal_posix_register_internal_error(__func__, tmp_err);
    }
    return ret;
}


int gfal_open(const char *path, int flag, ...)
{
    va_list va;
    va_start(va, flag);
    mode_t mode = va_arg(va, mode_t);
    va_end(va);

    GError *tmp_err = NULL;
    gfal2_context_t handle;

    if ((handle = gfal_posix_get_handle()) == NULL) {
        return -1;
    }

    int fd = gfal2_open2(handle, path, flag, mode, &tmp_err);
    if (tmp_err) {
        gfal_posix_register_internal_error(__func__, tmp_err);
    }
    return fd;
}


int gfal_creat(const char *filename, mode_t mode)
{
    return (gfal_open(filename, O_WRONLY | O_CREAT | O_TRUNC, mode));
}


ssize_t gfal_read(int fd, void *buff, size_t s_buff)
{
    GError *tmp_err = NULL;
    gfal2_context_t handle;

    if ((handle = gfal_posix_get_handle()) == NULL) {
        return -1;
    }

    int ret = gfal2_read(handle, fd, buff, s_buff, &tmp_err);
    if (tmp_err) {
        gfal_posix_register_internal_error(__func__, tmp_err);
    }
    return ret;
}


ssize_t gfal_write(int fd, const void *buff, size_t s_buff)
{
    GError *tmp_err = NULL;
    gfal2_context_t handle;

    if ((handle = gfal_posix_get_handle()) == NULL) {
        return -1;
    }

    int ret = gfal2_write(handle, fd, buff, s_buff, &tmp_err);
    if (tmp_err) {
        gfal_posix_register_internal_error(__func__, tmp_err);
    }
    return ret;
}


int gfal_close(int fd)
{
    GError *tmp_err = NULL;
    gfal2_context_t handle;

    if ((handle = gfal_posix_get_handle()) == NULL) {
        return -1;
    }

    int ret = gfal2_close(handle, fd, &tmp_err);
    if (tmp_err) {
        gfal_posix_register_internal_error(__func__, tmp_err);
    }
    return ret;
}


int gfal_symlink(const char *oldpath, const char *newpath)
{
    GError *tmp_err = NULL;
    gfal2_context_t handle;

    if ((handle = gfal_posix_get_handle()) == NULL) {
        return -1;
    }

    int ret = gfal2_symlink(handle, oldpath, newpath, &tmp_err);
    if (tmp_err) {
        gfal_posix_register_internal_error(__func__, tmp_err);
    }
    return ret;
}


off_t gfal_lseek(int fd, off_t offset, int whence)
{
    GError *tmp_err = NULL;
    gfal2_context_t handle;

    if ((handle = gfal_posix_get_handle()) == NULL) {
        return -1;
    }

    int ret = gfal2_lseek(handle, fd, offset, whence, &tmp_err);
    if (tmp_err) {
        gfal_posix_register_internal_error(__func__, tmp_err);
    }
    return ret;
}


ssize_t gfal_getxattr(const char *path, const char *name, void *value,
    size_t size)
{
    GError *tmp_err = NULL;
    gfal2_context_t handle;

    if ((handle = gfal_posix_get_handle()) == NULL) {
        return -1;
    }

    ssize_t ret = gfal2_getxattr(handle, path, name, value, size, &tmp_err);
    if (tmp_err) {
        gfal_posix_register_internal_error(__func__, tmp_err);
    }
    return ret;
}


ssize_t gfal_readlink(const char *path, char *buff, size_t buffsiz)
{
    gfal2_context_t handle;
    GError *tmp_err = NULL;

    if ((handle = gfal_posix_get_handle()) == NULL) {
        return -1;
    }

    ssize_t ret = gfal2_readlink(handle, path, buff, buffsiz, &tmp_err);
    if (tmp_err) {
        gfal_posix_register_internal_error(__func__, tmp_err);
    }
    return ret;
}


int gfal_unlink(const char *path)
{
    gfal2_context_t handle;
    GError *tmp_err = NULL;

    if ((handle = gfal_posix_get_handle()) == NULL) {
        return -1;
    }

    int ret = gfal2_unlink(handle, path, &tmp_err);
    if (tmp_err) {
        gfal_posix_register_internal_error(__func__, tmp_err);
    }
    return ret;
}


ssize_t gfal_listxattr(const char *path, char *list, size_t size)
{
    GError *tmp_err = NULL;
    gfal2_context_t handle;

    if ((handle = gfal_posix_get_handle()) == NULL) {
        return -1;
    }

    ssize_t ret = gfal2_listxattr(handle, path, list, size, &tmp_err);
    if (tmp_err) {
        gfal_posix_register_internal_error(__func__, tmp_err);
    }
    return ret;
}


int gfal_setxattr(const char *path, const char *name, const void *value,
    size_t size, int flags)
{
    GError *tmp_err = NULL;
    gfal2_context_t handle;

    if ((handle = gfal_posix_get_handle()) == NULL) {
        return -1;
    }

    int ret = gfal2_setxattr(handle, path, name, value, size, flags, &tmp_err);
    if (tmp_err) {
        gfal_posix_register_internal_error(__func__, tmp_err);
    }
    return ret;
}


int gfal_removexattr(const char *path, const char *name)
{
    GError *tmp_err = NULL;
    g_set_error(&tmp_err, gfal2_get_core_quark(), ENOSYS, "Not implemented");
    gfal_posix_register_internal_error(__func__, tmp_err);
    return -1;
}


ssize_t gfal_pread(int fd, void *buffer, size_t count, off_t offset)
{
    GError *tmp_err = NULL;
    gfal2_context_t handle;

    if ((handle = gfal_posix_get_handle()) == NULL) {
        return -1;
    }

    ssize_t ret = gfal2_pread(handle, fd, buffer, count, offset, &tmp_err);
    if (tmp_err) {
        gfal_posix_register_internal_error(__func__, tmp_err);
    }
    return ret;
}


ssize_t gfal_pwrite(int fd, const void *buffer, size_t count, off_t offset)
{
    GError *tmp_err = NULL;
    gfal2_context_t handle;

    if ((handle = gfal_posix_get_handle()) == NULL) {
        return -1;
    }

    ssize_t ret = gfal2_pwrite(handle, fd, buffer, count, offset, &tmp_err);
    if (tmp_err) {
        gfal_posix_register_internal_error( __func__, tmp_err);
    }
    return ret;
}


int gfal_flush(int fd)
{
    return 0;
}
