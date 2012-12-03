#pragma once
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
 
 
 /**
 * @file gfal_posix_local_file.c
 * @brief header file for the internal func of the posix interface
 * @author Devresse Adrien
 * @version 2.0
 * @date 06/05/2011
 * */

#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_common_errverbose.h>
#include <file/gfal_file_api.h>

gfal_handle gfal_posix_instance();

GError** gfal_posix_get_last_error();
 
int gfal_posix_internal_access (const char *path, int amode);

int gfal_posix_internal_chmod(const char* path, mode_t mode);

int gfal_posix_internal_rename(const char* oldpath, const char* newpath);

int gfal_posix_internal_stat(const char* path, struct stat* buf);

int gfal_posix_internal_lstat(const char* path, struct stat* buf);

ssize_t gfal_posix_internal_readlink(const char* path, char* buff, size_t buffsiz);

int gfal_posix_internal_mkdir(const char* path, mode_t mode);

int gfal_posix_internal_rmdir(const char* path);

DIR* gfal_posix_internal_opendir(const char* name);

int gfal_posix_internal_open(const char* path, int flag, mode_t mode);

int gfal_posix_internal_lseek(int fd, off_t offset, int whence);

int gfal_posix_internal_read(int fd, void* buff, size_t s_buff);

int gfal_posix_internal_write(int fd, void* buff, size_t s_buff);

ssize_t gfal_posix_internal_pwrite(int fd, void* buff, size_t s_buff, off_t offset);

ssize_t gfal_posix_internal_pread(int fd, void* buff, size_t s_buff, off_t offset);

int gfal_posix_internal_close(int fd);

int gfal_posix_internal_closedir(DIR* d);

ssize_t gfal_posix_internal_unlink(const char* path);

ssize_t gfal_posix_internal_getxattr (const char *path, const char *name,
                        void *value, size_t size);
ssize_t gfal_posix_internal_listxattr (const char *path, char *list, size_t size);

int gfal_posix_internal_setxattr (const char *path, const char *name,
			   const void *value, size_t size, int flags);

struct dirent* gfal_posix_internal_readdir(DIR* dir);

int gfal_posix_internal_symlink(const char * oldpath, const char *newpath);

void gfal_posix_register_internal_error(gfal_handle handle, const char* prefix, GError * tmp_err);
 
 
int gfal_get_parameter_boolean_internal(const char* namespace, const char* key);

int gfal_set_parameter_boolean_internal(const char* namespace, const char* key, int value);

int gfal_set_parameter_string_internal(const char* namespace, const char* key, const char* value);

char* gfal_get_parameter_string_internal(const char* namespace, const char* key);

