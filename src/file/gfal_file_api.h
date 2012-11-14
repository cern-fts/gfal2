#pragma once
#ifndef _GFAL2_FILE_API_
#define _GFAL2_FILE_API_
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

#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <attr/xattr.h>
#include <glib.h>

#include <global/gfal_global.h>
#include <common/gfal_constants.h>

#ifdef __cplusplus
extern "C"
{
#endif 


/*!
    \defgroup file_group GFAL 2.0 generic file API


    GFAL 2.0 file API is the main entry point for
    file/directory operations that does not require
    a strict POSIX standard

       All calls are
         - context specific
         - thread-safe
         - has its internal error report system
*/

/*!
    \addtogroup file_group
	@{
*/

/**
 *  checksum calculation function for a file url
 *
 * @param handle : gfal2 context
 * @param uri : uri of the file
 * @param check_type : string of the checksum type ( \ref GFAL_CHKSUM_MD5, \ref GFAL_CHKSUM_SHA1, .. )
 * @param start_offset : offset in the file where the checksum calculation will start ( 0 : from begining )
 * @param data_length : size of data to compute for the checksum after start_offset ( 0 -: full file )
 * @param checksum_buffer : buffer with checksum string as result
 * @param buffer_length : maximum buffer length
 * @param err : GError error support
 * @return 0 if success, else -1 and err is be set
 *  see gfal2 error system for more information \ref gfal2_error_system
 */
int gfal2_checksum(gfal2_context_t handle, const char* uri, const char* check_type,
                 off_t start_offset, size_t data_length,
                char * checksum_buffer, size_t buffer_length, GError ** err);


/**
 *  check real user's permissions for a file
 *
 * @param handle : gfal2 context
 * @param uri : uri of the file
 * @param amode : mode of the access
 * @param err : GError error support
 * @return 0 if success, else -1 and err MUST be set properly
 *  see gfal2 error system for more information \ref gfal2_error_system
 */
int gfal2_access(gfal2_context_t handle, const char *uri, int amode, GError** err);

/**
 * @brief change file access permissions
 *
 *  gfal2_chmod changes the permissions of a file according to mode
 *
 * @param handle : gfal2 context object
 * @param uri : url of the file or the folder
 * @param mode : right to configure
 * @param err : GError error support
 * @return return 0 if success else -1 and err is be set
 *  see gfal2 error system for more information \ref gfal2_error_system
 */
int gfal2_chmod(gfal2_context_t handle, const char* uri, mode_t mode, GError ** err);

/*!
 * @brief  change the name or location of a file
 *
 *  gfal2_rename will rename olduri to newuri

 * @param handle : gfal2 context object
 * @param olduri : the old url of the file
 * @param newurl : the new url of the file
 * @param err : GError error support
 * @return : return 0 if success, else -1 if errors.
 *  see gfal2 error system for more information \ref gfal2_error_system
*/
int gfal2_rename(gfal2_context_t handle, const char *olduri, const char *newuri, GError ** err);

//
//
//
//
int gfal2_stat(gfal2_context_t handle, const char* uri, struct stat* buff, GError ** err);

int gfal2_lstat(gfal2_context_t handle, const char* uri, struct stat* buff, GError ** err);

int gfal2_mkdir_rec(gfal2_context_t handle,  const char* uri, mode_t mode, GError ** err);


int gfal2_mkdir(gfal2_context_t handle,  const char* uri, mode_t mode, GError ** err);


int gfal2_rmdir(gfal2_context_t handle, const char* uri, GError ** err);


DIR* gfal2_opendir(gfal2_context_t handle, const char* uri, GError ** err);


struct dirent* gfal2_readdir(gfal2_context_t handle, DIR* d, GError ** err);


int gfal2_closedir(gfal2_context_t handle, DIR* d, GError ** err);


int gfal2_open(gfal2_context_t handle, const char * path, int flag, GError ** err);

int gfal2_open2(gfal2_context_t handle, const char * path, int flag, mode_t mode, GError ** err);



int gfal2_creat (gfal2_context_t handle, const char *filename, mode_t mode, GError ** err);

ssize_t gfal2_read(gfal2_context_t handle, int fd, void* buff, size_t s_buff, GError ** err);

ssize_t gfal2_write(gfal2_context_t handle, int fd, const void *buff, size_t s_buff, GError ** err);


int gfal2_close(gfal2_context_t handle, int fd, GError ** err);

int gfal2_symlink(gfal2_context_t handle, const char* oldpath, const char * newpath, GError ** err);

off_t gfal2_lseek (gfal2_context_t handle, int fd, off_t offset, int whence, GError ** err);

ssize_t gfal2_getxattr (gfal2_context_t handle, const char *path, const char *name,
                        void *value, size_t size, GError ** err);


ssize_t gfal2_readlink(gfal2_context_t handle, const char* path, char* buff, size_t buffsiz, GError ** err);

int gfal2_unlink(gfal2_context_t handle, const char* path, GError ** err);

ssize_t gfal2_listxattr (gfal2_context_t handle, const char *path, char *list, size_t size, GError ** err);

int gfal2_setxattr (gfal2_context_t handle, const char *path, const char *name,
               const void *value, size_t size, int flags, GError ** err);



int gfal2_flush(gfal2_context_t handle, int fd, GError ** err);



ssize_t gfal2_pread(gfal2_context_t handle, int fd, void * buffer, size_t count, off_t offset, GError ** err);



ssize_t gfal2_pwrite(gfal2_context_t handle, int fd, const void * buffer, size_t count, off_t offset, GError ** err);


/**
	@} 
    End of the FILE group
*/
#ifdef __cplusplus
}
#endif 


#endif // _GFAL2_FILE_API_
