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
    file/directory operations

    All functions report a EPROTONOSUPPORT GError if the url protocol
    does not support this operation.
*/

/*!
    \addtogroup file_group
	@{
*/


////
//// Meta-data operations
////

/// @briefcompute checksum
///
/// compute checksum function for a file url
/// @param context : gfal2 context
/// @param url : url of the file
/// @param check_type : string of the checksum type ( \ref GFAL_CHKSUM_MD5, \ref GFAL_CHKSUM_SHA1, .. )
/// @param start_offset : offset in the file where the checksum calculation will start ( 0 : from begining )
/// @param data_length : size of data to compute for the checksum after start_offset ( 0 -: full file )
/// @param checksum_buffer : buffer with checksum string as result
/// @param buffer_length : maximum buffer length
/// @param err : GError error report
/// @return 0 if success, else -1 and err is be set
///  see gfal2 error system for more information \ref gfal2_error_system
int gfal2_checksum(gfal2_context_t context, const char* url, const char* check_type,
                 off_t start_offset, size_t data_length,
                char * checksum_buffer, size_t buffer_length, GError ** err);

/// @brief permission check
///
///  check real user's permissions for a file
/// @param context : gfal2 context
/// @param url : url of the file
/// @param amode : mode of the access
/// @param err : GError error report
/// @return 0 if success, else -1 and err MUST be set properly
///  see gfal2 error system for more information \ref gfal2_error_system
int gfal2_access(gfal2_context_t context, const char *url, int amode, GError** err);

/// @brief change file access permissions
///
///  change the permissions of a file according to "mode"
/// @param context : gfal2 context
/// @param url : url of the file or the folder
/// @param mode : right to configure
/// @param err : GError error report
/// @return return 0 if success else -1 and err is be set
///  see gfal2 error system for more information \ref gfal2_error_system
int gfal2_chmod(gfal2_context_t context, const char* url, mode_t mode, GError ** err);

/// @brief  change the name or location of a file
///
///  move ( or rename ) the file 'oldurl' to 'newurl'
/// @param context : gfal2 context
/// @param oldurl : the old url of the file
/// @param newurl : the new url of the file
/// @param err : GError error report
/// @return : return 0 if success, else -1 if errors.
///  see gfal2 error system for more information \ref gfal2_error_system
int gfal2_rename(gfal2_context_t context, const char *oldurl, const char *newurl, GError ** err);

///  @brief posix file status
///
///  get meta-data information about the file 'url'
///  @param context : gfal2 context
///  @param url : url of the file
///  @param buff : stat structure filled
///  @param err : GError error report
///
int gfal2_stat(gfal2_context_t context, const char* url, struct stat* buff, GError ** err);


///  @brief posix file status
///
///  get meta-data information about the file 'url'
///  same behavior than \ref gfal2_stat but return information
///  about the link itself if "url" is a symbolic link
///
///  @param context : gfal2 context
///  @param url : url of the file
///  @param buff : stat structure filled
///  @param err : GError error report
///
int gfal2_lstat(gfal2_context_t context, const char* url, struct stat* buff, GError ** err);




///  @brief create directory
///
///  create a directory at the address  'url'
///
///  @param context : gfal2 context
///  @param url : url of the file
///  @param mode : directory file rights
///  @param err : GError error report
///
int gfal2_mkdir(gfal2_context_t context,  const char* url, mode_t mode, GError ** err);


///  @brief create directory
///
///  create a directory at the address  'url'
///  create all the parent drectories  and
///  does not return an error if the directory already exist
///
///  @param context : gfal2 context
///  @param url : url of the file
///  @param mode : directory file rights
///  @param err : GError error report
///
int gfal2_mkdir_rec(gfal2_context_t context,  const char* url, mode_t mode, GError ** err);

///  @brief suppress a directory
///
///  suppress a directory at the address  'url'
///  @param context : gfal2 context
///  @param url : url of the file
///  @param err : GError error report
///
int gfal2_rmdir(gfal2_context_t context, const char* url, GError ** err);


DIR* gfal2_opendir(gfal2_context_t context, const char* url, GError ** err);


struct dirent* gfal2_readdir(gfal2_context_t context, DIR* d, GError ** err);


struct dirent* gfal2_readdirpp(gfal2_context_t context, DIR* d, struct stat* st, GError ** err);



int gfal2_closedir(gfal2_context_t context, DIR* d, GError ** err);




int gfal2_symlink(gfal2_context_t context, const char* oldpath, const char * newpath, GError ** err);

off_t gfal2_lseek (gfal2_context_t context, int fd, off_t offset, int whence, GError ** err);

ssize_t gfal2_getxattr (gfal2_context_t context, const char *path, const char *name,
                        void *value, size_t size, GError ** err);


ssize_t gfal2_readlink(gfal2_context_t context, const char* path, char* buff, size_t buffsiz, GError ** err);

int gfal2_unlink(gfal2_context_t context, const char* path, GError ** err);

ssize_t gfal2_listxattr (gfal2_context_t context, const char *path, char *list, size_t size, GError ** err);

int gfal2_setxattr (gfal2_context_t context, const char *path, const char *name,
               const void *value, size_t size, int flags, GError ** err);

int gfal2_bring_online(gfal2_context_t context, const char* path,
                       time_t pintime, time_t timeout,
                       char* token, size_t tsize,
                       int async,
                       GError ** err);

int gfal2_bring_online_poll(gfal2_context_t context, const char* path,
                            const char* token, GError ** err);

int gfal2_release_file(gfal2_context_t context, const char* path,
                       const char* token, GError ** err);


/////////////////////////////////////////////////////
///////////// R/W operations
/////////////////////////////////////////////////////



int gfal2_open(gfal2_context_t context, const char * path, int flag, GError ** err);

int gfal2_open2(gfal2_context_t context, const char * path, int flag, mode_t mode, GError ** err);



int gfal2_creat (gfal2_context_t context, const char *filename, mode_t mode, GError ** err);

ssize_t gfal2_read(gfal2_context_t context, int fd, void* buff, size_t s_buff, GError ** err);

ssize_t gfal2_write(gfal2_context_t context, int fd, const void *buff, size_t s_buff, GError ** err);


int gfal2_close(gfal2_context_t context, int fd, GError ** err);

int gfal2_flush(gfal2_context_t context, int fd, GError ** err);


ssize_t gfal2_pread(gfal2_context_t context, int fd, void * buffer, size_t count, off_t offset, GError ** err);

ssize_t gfal2_pwrite(gfal2_context_t context, int fd, const void * buffer, size_t count, off_t offset, GError ** err);


/**
	@} 
    End of the FILE group
*/
#ifdef __cplusplus
}
#endif 


#endif // _GFAL2_FILE_API_
