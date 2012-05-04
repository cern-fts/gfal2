#pragma once
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 *  gfal_posix_local_file.c
 *  header file for the local access file map for the gfal_posix call
 *  author Devresse Adrien
 */

#include <sys/types.h>
#include <glib.h>
#include <common/gfal_types.h>
 
int gfal_local_initG(GError** err);

gboolean gfal_check_local_url(const char* path, GError** err);

gboolean gfal_is_local_call(const char * module_name); 
 
int gfal_local_access(const char *path, int amode, GError** err);
 
int gfal_local_chmod(const char* path, mode_t mode, GError** err);
 
int gfal_local_rename(const char* oldpath, const char* newpath, GError** err);
 
int gfal_local_stat(const char* path, struct stat* buf, GError ** err);
 
int gfal_local_lstat(const char* path, struct stat* buf, GError ** err);

int gfal_local_symlink(const char* oldpath, const char* newpath, GError** err);

ssize_t gfal_local_readlink(const char* path, char* buff, size_t buffsiz, GError** err);

int gfal_local_lseek(gfal_file_handle fh, off_t offset, int whence, GError** err);

int gfal_local_mkdir(const char* path, mode_t mode, GError** err);

ssize_t gfal_local_getxattr(const char* path, const char* name, void* buff, size_t s_buff, GError** err);

ssize_t gfal_local_listxattr(const char* path, char* list, size_t s_list, GError** err);

int gfal_local_setxattr(const char* path, const char* name, const void* value, size_t size, int flags, GError** err);

int gfal_local_rmdir(const char* path, GError** err);

struct dirent* gfal_local_readdir(gfal_file_handle d, GError** err);

gfal_file_handle gfal_local_opendir(const char* path, GError** err);

int gfal_local_closedir(gfal_file_handle fh, GError** err);

gfal_file_handle gfal_local_open(const char* path, int flag, mode_t mode, GError** err);

int gfal_local_read(gfal_file_handle fh, void* buff, size_t s_buff, GError** err);

ssize_t gfal_local_pread(gfal_file_handle fh, void* buff, size_t s_buff, off_t offset, GError** err);


int gfal_local_write(gfal_file_handle fh, void* buff, size_t s_buff, GError** err);

ssize_t gfal_local_pwrite(gfal_file_handle fh, void* buff, size_t s_buff, off_t offset, GError** err);

int gfal_local_close(gfal_file_handle fh, GError** err);

int gfal_local_unlink(const char* path, GError** err);
