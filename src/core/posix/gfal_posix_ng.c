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
 * @file gfal_posix_ng.c
 * @brief new file for the posix interface
 * @author Devresse Adrien
 * @date 09/05/2011
 * */

#include <string.h>

#include <common/gfal_constants.h>

#include "gfal_posix_api.h"
#include "gfal_posix_internal.h"



int gfal_access(const char *path, int amode)
{
    return gfal_posix_internal_access(path, amode);
}


int gfal_chmod(const char* path, mode_t mode)
{
    return gfal_posix_internal_chmod(path, mode);
}


int gfal_rename(const char *oldpath, const char *newpath)
{
    return gfal_posix_internal_rename(oldpath, newpath);
}


int gfal_stat(const char* path, struct stat* buff)
{
    return gfal_posix_internal_stat(path, buff);
}


int gfal_lstat(const char* path, struct stat* buff)
{
    return gfal_posix_internal_lstat(path, buff);
}


int gfal_mkdirp(const char* path, mode_t mode)
{
    return gfal_posix_internal_mkdir(path, mode);
}


int gfal_mkdir(const char* path, mode_t mode)
{
    return gfal_mkdirp(path, mode);
}


int gfal_rmdir(const char* path)
{
    return gfal_posix_internal_rmdir(path);
}

DIR* gfal_opendir(const char* name)
{
    return gfal_posix_internal_opendir(name);
}


struct dirent* gfal_readdir(DIR* d)
{
    return gfal_posix_internal_readdir(d);
}


int gfal_closedir(DIR* d)
{
    return gfal_posix_internal_closedir(d);
}


int gfal_open(const char * path, int flag, ...)
{
    mode_t mode = S_IRWXU | S_IRGRP | S_IROTH;
    va_list va;
    va_start(va, flag);
    mode = va_arg(va, mode_t);
    va_end(va);
    return gfal_posix_internal_open(path, flag, mode);
}


int gfal_creat(const char *filename, mode_t mode)
{
    return (gfal_open(filename, O_WRONLY | O_CREAT | O_TRUNC, mode));
}


ssize_t gfal_read(int fd, void* buff, size_t s_buff)
{
    return (ssize_t) gfal_posix_internal_read(fd, buff, s_buff);
}


ssize_t gfal_write(int fd, const void *buff, size_t s_buff)
{
    return (ssize_t) gfal_posix_internal_write(fd, (void*) buff, s_buff);
}


int gfal_close(int fd)
{
    return gfal_posix_internal_close(fd);
}


int gfal_symlink(const char* oldpath, const char * newpath)
{
    return gfal_posix_internal_symlink(oldpath, newpath);
}


off_t gfal_lseek(int fd, off_t offset, int whence)
{
    return gfal_posix_internal_lseek(fd, offset, whence);
}


ssize_t gfal_getxattr(const char *path, const char *name, void *value,
        size_t size)
{
    return gfal_posix_internal_getxattr(path, name, value, size);
}


ssize_t gfal_readlink(const char* path, char* buff, size_t buffsiz)
{
    return gfal_posix_internal_readlink(path, buff, buffsiz);
}


int gfal_unlink(const char* path)
{
    return gfal_posix_internal_unlink(path);
}


ssize_t gfal_listxattr(const char *path, char *list, size_t size)
{
    return gfal_posix_internal_listxattr(path, list, size);
}


int gfal_setxattr(const char *path, const char *name, const void *value,
        size_t size, int flags)
{
    return gfal_posix_internal_setxattr(path, name, value, size, flags);
}


int gfal_removexattr(const char *path, const char *name)
{
    return -1;
}


void gfal_posix_print_error()
{
    GError* err = NULL;
    if (gfal_posix_instance() == NULL) {
        g_printerr(
                "[gfal] Initialisation error gfal_posix_instance() failure\n");
    }
    else if ((err = *gfal_posix_get_last_error()) != NULL) {
        g_printerr("[gfal]%s \n", err->message);
    }
    else if (errno != 0) {
        char* sterr = strerror(errno);
        g_printerr("[gfal] errno reported by external lib : %s", sterr);
    }
    else {
        g_printerr("[gfal] No gfal error reported\n");
    }
}


int gfal_flush(int fd)
{
    return 0;
}


void gfal_posix_release_error()
{
    gfal_posix_print_error();
    gfal_posix_clear_error();
}


void gfal_posix_clear_error()
{
    g_clear_error(gfal_posix_get_last_error());
    errno = 0;
}


int gfal_posix_code_error()
{
    GError* err = NULL;
    const int ret =
            ((err = *gfal_posix_get_last_error()) != NULL) ? err->code : 0;
    return ret;
}


int gfal_posix_check_error()
{
    GError* err = NULL;
    if ((err = *gfal_posix_get_last_error()) != NULL) {
        g_printerr("[gfal] %s\n", err->message);
        return 1;
    }
    return 0;
}


char* gfal_posix_strerror_r(char* buff_err, size_t s_err)
{
    GError** last_error = gfal_posix_get_last_error();
    if (last_error == NULL || *last_error == NULL) {
        gfal2_log(G_LOG_LEVEL_DEBUG, "copy string NULL error");
        g_strlcpy(buff_err, "[gfal] No Error reported", s_err);
    }
    else {
        g_strlcpy(buff_err, "[gfal]", s_err);
        g_strlcat(buff_err, (*last_error)->message, s_err);
    }
    return buff_err;
}


ssize_t gfal_pread(int fd, void * buffer, size_t count, off_t offset)
{
    return gfal_posix_internal_pread(fd, buffer, count, offset);
}


ssize_t gfal_pwrite(int fd, const void * buffer, size_t count, off_t offset)
{
    return gfal_posix_internal_pwrite(fd, (void*) buffer, count, offset);
}


gfal2_context_t gfal_posix_get_context()
{
    return gfal_posix_instance();
}
