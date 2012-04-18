#pragma once
#ifndef _GFAL2_POSIX_API_
#define _GFAL2_POSIX_API_
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
/**
 * @file gfal_posix_api
 * @brief main header file for API of the posix lib
 * @author Devresse Adrien
 * @version 2.0.1
 * @date 30/09/2011
 * */


#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <attr/xattr.h>

#include <common/gfal_constants.h>

#ifdef __cplusplus
extern "C"
{
#endif 

 




/**
	\defgroup posix_group all POSIX style function
*/

/**
	\addtogroup posix_group
	@{
*/


int gfal_chmod(const char* path, mode_t mode);
int gfal_rename (const char *oldpath, const char * newpath);

// access and stat purpose
int gfal_stat (const char *, struct stat *);
int gfal_lstat (const char *, struct stat *);
int gfal_access (const char *, int);
ssize_t gfal_readlink(const char* path, char* buff, size_t buffsiz);
int gfal_symlink(const char* oldpath, const char * newpath);


//classical read/write operations
int gfal_creat (const char *, mode_t);
int gfal_open(const char * path, int flag, ...);
off_t gfal_lseek (int, off_t, int);
int gfal_close (int);
int gfal_read (int, void *, size_t);
int gfal_write (int, const void *, size_t);

// pipelined calls, for vector read/write
ssize_t gfal_pread(int fd, void * buffer, size_t count, off_t offset);
ssize_t gfal_pwrite(int fd, const void * buffer, size_t count, off_t offset);

// advanced use purpose ( set properties, guid, replicas )
ssize_t gfal_getxattr (const char *path, const char *name,
                        void *value, size_t size);
ssize_t gfal_listxattr (const char *path, char *list, size_t size);

int gfal_setxattr (const char *path, const char *name,
			   const void *value, size_t size, int flags);
			   
int gfal_removexattr(const char *path, const char *name);





// directory management purpose
int gfal_mkdir (const char *, mode_t);
DIR *gfal_opendir (const char *);
int gfal_closedir (DIR *);
struct dirent *gfal_readdir (DIR *);


int gfal_rmdir (const char *);
int gfal_unlink (const char *);


// error management
int gfal_posix_check_error();
void gfal_posix_clear_error();
void gfal_posix_release_error();
char* gfal_posix_strerror_r(char* buff_err, size_t s_err);
void gfal_posix_print_error();
int gfal_posix_code_error();


// define the verbose mode
int gfal_set_verbose (int);
char *gfal_version();

// sync
int gfal_flush(int fd);



int gfal_set_parameter_string(const char* namespc, const char* key, const char* value);
char* gfal_get_parameter_string(const char* namespc, const char* key);

int gfal_set_parameter_string_list(const char* namespc, const char* key, const char* value[], size_t len);
char** gfal_get_parameter_string_list(const char* namespc, const char* key, size_t* s_buff);

int gfal_set_parameter_int(const char* namespc, const char* key, int value);
int gfal_get_parameter_int(const char* namespc, const char* key);


int gfal_get_parameter_boolean(const char* namespc, const char* key);
int gfal_set_parameter_boolean(const char* namespc, const char* key, int bool_value);


/**
	@} 
	End of the POSIX group
*/
#ifdef __cplusplus
}
#endif 


#endif // _GFAL2_POSIX_API_
