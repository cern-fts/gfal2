/*
 * Copyright (c) CERN 2013-2017
 *
 * Copyright (c) Members of the EMI Collaboration. 2010-2013
 * See  http://www.eu-emi.eu/partners for details on the copyright
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

#pragma once
#ifndef GFAL_FILE_API_H_
#define GFAL_FILE_API_H_

#if !defined(__GFAL2_H_INSIDE__) && !defined(__GFAL2_BUILD__)
#   warning "Direct inclusion of gfal2 headers is deprecated. Please, include only gfal_api.h or gfal_plugins_api.h"
#endif

#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef __APPLE__
#include <sys/xattr.h>
#else
#include <attr/xattr.h>
#endif
#include <glib.h>

#include <common/gfal_common.h>
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

/**
 * @briefcompute checksum
 *
 * Compute checksum function for a file url
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param url : url of the file
 * @param check_type : string of the checksum type ( \ref GFAL_CHKSUM_MD5, \ref GFAL_CHKSUM_SHA1, .. )
 * @param start_offset : offset in the file where the checksum calculation will start ( 0 : from begining )
 * @param data_length : size of data to compute for the checksum after start_offset ( 0 -: full file )
 * @param checksum_buffer : buffer with checksum string as result
 * @param buffer_length : maximum buffer length
 * @param err : GError error report
 * @return 0 if success, else -1 and err is be set
 * See gfal2 error system for more information \ref gfal2_error_system
 */
int gfal2_checksum(gfal2_context_t context, const char* url, const char* check_type,
                 off_t start_offset, size_t data_length,
                char * checksum_buffer, size_t buffer_length, GError ** err);

/**
 * @brief permission check
 *
 * Check real user's permissions for a file
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param url : url of the file
 * @param amode : mode of the access
 * @param err : GError error report
 * @return 0 if success, else -1 and err MUST be set properly
 * See gfal2 error system for more information \ref gfal2_error_system
 */
int gfal2_access(gfal2_context_t context, const char *url, int amode, GError** err);

/**
 * @brief change file access permissions
 *
 * Change the permissions of a file according to "mode"
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param url : url of the file or the folder
 * @param mode : right to configure
 * @param err : GError error report
 * @return return 0 if success else -1 and err is be set
 * See gfal2 error system for more information \ref gfal2_error_system
 */
int gfal2_chmod(gfal2_context_t context, const char* url, mode_t mode, GError ** err);

/**
 * @brief  change the name or location of a file
 *
 * Move ( or rename ) the file 'oldurl' to 'newurl'
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param oldurl : the old url of the file
 * @param newurl : the new url of the file
 * @param err : GError error report
 * @return : return 0 if success, else -1 if errors.
 * See gfal2 error system for more information \ref gfal2_error_system
 */
int gfal2_rename(gfal2_context_t context, const char *oldurl, const char *newurl, GError ** err);

/**
 * @brief posix file status
 *
 * Get meta-data information about the file 'url'
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param url : url of the file
 * @param buff : stat structure filled
 * @param err : GError error report
 */
int gfal2_stat(gfal2_context_t context, const char* url, struct stat* buff, GError ** err);

/**
 * @brief posix file status
 *
 * Get meta-data information about the file 'url'
 * same behavior than \ref gfal2_stat but return information
 * about the link itself if "url" is a symbolic link
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param url : url of the file
 * @param buff : stat structure filled
 * @param err : GError error report
 */
int gfal2_lstat(gfal2_context_t context, const char* url, struct stat* buff, GError ** err);

/**
 * @brief create directory
 *
 * Create a directory at the address  'url'
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param url : url of the file
 * @param mode : directory file rights
 * @param err : GError error report
 */
int gfal2_mkdir(gfal2_context_t context,  const char* url, mode_t mode, GError ** err);

/**
 * @brief create directory
 *
 * Create a directory at the address  'url'
 * Create all the parent drectories  and
 * does not return an error if the directory already exist
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param url : url of the file
 * @param mode : directory file rights
 * @param err : GError error report
 */
int gfal2_mkdir_rec(gfal2_context_t context,  const char* url, mode_t mode, GError ** err);

/**
 * @brief suppress a directory
 *
 * Suppress a directory at the address  'url'
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param url : url of the file
 * @param err : GError error report
 * @return 0 if success, negative value if error, set err properly in case of error
 */
int gfal2_rmdir(gfal2_context_t context, const char* url, GError ** err);

/**
 * @brief open a directory for content listing
 *
 * Return a directory handle for content listing
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param url : url of the file
 * @param err : GError error report
 * @return a directory handle in case of success or NULL if error occurs, set err properly in case of error
 */
DIR* gfal2_opendir(gfal2_context_t context, const char* url, GError ** err);

/**
 * @brief return the next directory entry
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param d : directory handle created by \ref gfal2_opendir
 * @param err : GError error report
 * @return pointer to a dirent struct if success, NULL if end of listing or error,
 *  set err properly in case of error
 */
struct dirent* gfal2_readdir(gfal2_context_t context, DIR* d, GError ** err);

/**
 * @brief return the next directory entry in addition of the entry meta-data
 *
 * readdirpp get both of the directory entry informations and the stat
 * informations in one operation, improving the performance in case of remote file system
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param d : directory handle created by \ref gfal2_opendir
 * @param st : the file stats will be stored here
 * @param err : GError error report
 * @return pointer to a dirent struct and configure st with the meta-data information if success, NULL if end of listing or error, set err properly in case of error
 */
struct dirent* gfal2_readdirpp(gfal2_context_t context, DIR* d, struct stat* st, GError ** err);

/**
 * @brief close a directory handle
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param d : directory handle created by \ref gfal2_opendir
 * @param err : GError error report
 * @return 0 if success, negative value if error, set err properly in case of error
 */
int gfal2_closedir(gfal2_context_t context, DIR* d, GError ** err);

/**
 * @brief create a symbolic link
 *
 * Symbolic links are not supported by all protocols, in case of non-supported feature
 * GFAL2 always return an error and set err to the code EPROTONOSUPPORT
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param oldurl : origin file
 * @param newurl : symbolic link path
 * @param err : GError error report
 * @return 0 if success, negative value if error, set err properly in case of error
 */
int gfal2_symlink(gfal2_context_t context, const char* oldurl, const char* newurl, GError ** err);

/**
 * @brief read a symbolic link value, provide the linked file path
 *
 * Symbolic links are not supported by all protocols, in case of non-supported feature
 * GFAL2 always return an error and set err to the code EPROTONOSUPPORT.
 * gfal2_readlink follows the POSIX behavior and does not add a null byte at the end of the buffer if the link is truncated.
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param url : path of the symbolic link to read
 * @param buff : buffer for symbolic link value
 * @param buffsiz : maximum number of bytes to write
 * @param err : GError error report
 * @return size of the link value in bytes if success, negative value if error. set err properly in case of error
 */
ssize_t gfal2_readlink(gfal2_context_t context, const char* url, char* buff, size_t buffsiz, GError ** err);

/**
 * @brief Delete a file entry
 *
 * Does not work for Collections or directory.
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param url : path of the file to delete
 * @param err : GError error report
 * @return 0 if success, -1 if error. set err properly in case of error
 */
int gfal2_unlink(gfal2_context_t context, const char* url, GError ** err);

/**
 * @brief list extended attributes of a resource.
 *
 * Extended attributes keys are concatenated in the buffer and separated by a null character
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param url : path of the resource
 * @param list : buffer for the extended attribute keys
 * @param size : maximum size of the buffer to write
 * @param err : GError error report
 * @return the size of the concatenated xattr keys in bytes if success, -1 if error. set err properly in case of errors.
 */
ssize_t gfal2_listxattr (gfal2_context_t context, const char *url, char *list, size_t size, GError ** err);

/**
 * @brief get an extended attribute value of a resource.
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param url : path of the resource
 * @param name : key of the extended attribute
 * @param value : buffer for the extended attribute value
 * @param size : maximum size of the buffer to write
 * @param err : GError error report
 * @return the size of the xattr value in bytes if success, -1 if error. set err properly in case of errors.
 */
ssize_t gfal2_getxattr (gfal2_context_t context, const char *url, const char *name,
                        void *value, size_t size, GError ** err);

/**
 *  @brief set an extended attribute value of a resource.
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param url : path of the resource
 * @param name : key of the extended attribute to define/set
 * @param value : new value of the xattr
 * @param size : size of the data
 * @param flags:  xattr flag, XATTR_CREATE specifies a pure create, which fails if the named attribute exists already.  XATTR_REPLACE specifies a pure
 *       replace operation, which fails if the named attribute does not already exist.  By default (no flags), the extended attribute will be created if need be, or will  simply  replace  the  value  if  the
 *       attribute exists.
 *
 * @param err : GError error report
 * @return 0 if success, -1 if error. set err properly in case of errors.
 */
int gfal2_setxattr (gfal2_context_t context, const char *url, const char *name,
               const void *value, size_t size, int flags, GError ** err);

/**
 * @brief Bring online a file
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param url : url of the file
 * @param pintime : pin time
 * @param timeout : timeout
 * @param token : The token will be put in the buffer pointed by this
 * @param tsize:  The size of the buffer pointed by token
 * @param async: Asynchronous request (does not block if != 0)
 * @param err : GError error report
 * @return 0 if the request has been queued, > 0 if the file is pinned, < 0 on error
 */
int gfal2_bring_online(gfal2_context_t context, const char* url,
                       time_t pintime, time_t timeout,
                       char* token, size_t tsize,
                       int async,
                       GError ** err);

/**
 * @brief Check for a bring online request
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param url : url of the file
 * @param token : As set by gfal2_bring_online
 * @param err : GError error report
 * @return 0 if the request is queued, > 0 if the file is pinned, < 0 on error
 */
int gfal2_bring_online_poll(gfal2_context_t context, const char* url,
                            const char* token, GError ** err);

/**
 * @brief Release a file
 */
int gfal2_release_file(gfal2_context_t context, const char* url,
                       const char* token, GError ** err);

/**
 * @brief Check QoS classes available
 */
const char* gfal2_qos_check_classes(gfal2_context_t context, const char *url, const char* type, GError ** err);

/**
 * @brief Check Qos of File
 */
const char* gfal2_check_file_qos(gfal2_context_t context, const char *fileUrl, GError ** err);

/**
 * @brief Check Qos of File
 */
const char* gfal2_check_available_qos_transitions(gfal2_context_t context, const char *qosClassUrl, GError ** err);

/**
 * @brief Bring online a file
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param nbfiles : number of files
 * @param urls : urls of files
 * @param pintime : pin time
 * @param timeout : timeout
 * @param token : The token will be put in the buffer pointed by this
 * @param tsize : The size of the buffer pointed by token
 * @param async: Asynchronous request (does not block if != 0)
 * @param errors : Preallocated array of nbfiles pointers to GError. User must allocate and free.
 * @return 0 if the request has been queued, > 0 if the file is pinned, < 0 on error
 * @note  Even if the result is > 0, you need to check each individual file status
 */
int gfal2_bring_online_list(gfal2_context_t context, int nbfiles, const char* const* urls,
                       time_t pintime, time_t timeout,
                       char* token, size_t tsize,
                       int async,
                       GError ** errors);

/**
 * @brief Check for a bring online request
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param nbfiles : number of files
 * @param urls : urls of files
 * @param token : As set by gfal2_bring_online
 * @param errors : Preallocated array of nbfiles pointers to GError. User must allocate and free.
 * @return 0 if the request is queued, > 0 if the file is pinned, < 0 on error
 * @note  Even if the result is > 0, you need to check each individual file status
 */
int gfal2_bring_online_poll_list(gfal2_context_t context, int nbfiles, const char* const* urls,
                            const char* token, GError ** errors);

/**
 * @brief Release a file
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param nbfiles : number of files
 * @param urls   : paths of the files to delete
 * @param token  : the token from the bring online request
 * @param errors : Preallocated array of nbfiles pointers to GError. User must allocate and free.
 * @note  Even if the result is > 0, you need to check each individual file status
 */
int gfal2_release_file_list(gfal2_context_t context, int nbfiles, const char* const* urls,
                       const char* token, GError ** errors);

/**
 * @brief Perform a bulk deletion
 *
 * Does not work for Collections or directories.
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param nbfiles : number of files
 * @param urls    : paths of the files to delete
 * @param errors  : Pre-allocated array with nbfiles pointers to errors.
 *                  It is the user's responsability to allocate and free.
 * @return 0 if success, -1 if error. set err properly in case of error
 * @note The plugin tried will be the one that matches the first url
 * @note If bulk deletion is not supported, gfal2_unlink will be called nbfiles times
 */
int gfal2_unlink_list(gfal2_context_t context, int nbfiles, const char* const* urls, GError ** errors);

/**
 * @brief abort a list of files
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param nbfiles : number of files
 * @param urls   : paths of the files to delete
 * @param token  : the token from the bring online request
 * @param errors : Preallocated array of nbfiles pointers to GError. User must allocate and free.
 * @note  Even if the result is > 0, you need to check each individual file status
 */
int gfal2_abort_files(gfal2_context_t context, int nbfiles, const char* const* urls, const char* token, GError ** errors);

/**
 * @brief Open a file, return GFAL2 file descriptor
 *
 * gfal_open supports the same flags than the POSIX open call
 *
 * O_TRUNC
 *		  If  the file already exists and is a regular file and the open mode allows writing (i.e., is O_RDWR or O_WRONLY) it will be truncated to length 0.  If the file is a FIFO or termi‐
 *		  nal device file, the O_TRUNC flag is ignored.  Otherwise the effect of O_TRUNC is unspecified.
 *
 * O_APPEND
 *		  The  file  is opened in append mode.  Before each write(2), the file offset is positioned at the end of the file, as if with lseek(2).  O_APPEND may lead to corrupted files on NFS
 *		  file systems if more than one process appends data to a file at once.  This is because NFS does not support appending to a file, so the client kernel has  to  simulate  it,  which
 *		  can't be done without a race condition.
 *
 * O_CREAT
 *        If the file does not exist it will be created.  The owner (user ID) of the file is set to the effective user ID of the process.  The group ownership (group ID) is  set  either  to
 *        the  effective  group  ID of the process or to the group ID of the parent directory (depending on file system type and mount options, and the mode of the parent directory, see the
 *        mount options bsdgroups and sysvgroups described in mount(8)).
 *
 *        mode specifies the permissions to use in case a new file is created.  This argument must be supplied when O_CREAT is specified in flags; if O_CREAT is not specified, then mode  is
 *        ignored.   The  effective  permissions  are  modified  by  the process's umask in the usual way: The permissions of the created file are (mode & ~umask).  Note that this mode only
 *        applies to future accesses of the newly created file; the open() call that creates a read-only file may well return a read/write file descriptor.
 *
 * O_DIRECT
 *        Try to minimize cache effects of the I/O to and from this file.  In general this will degrade performance, but it is useful in special situations, such  as  when  applications  do
 *        their  own  caching.   File I/O is done directly to/from user space buffers.  The O_DIRECT flag on its own makes at an effort to transfer data synchronously, but does not give the
 *        guarantees of the O_SYNC that data and necessary metadata are transferred.  To guarantee synchronous I/O the O_SYNC must be used in addition to O_DIRECT.  See NOTES below for fur‐
 *        ther discussion.
 *
 * O_LARGEFILE
 *        (LFS) Allow files whose sizes cannot be represented in an off_t (but can be represented in an off64_t) to be opened.  The _LARGEFILE64_SOURCE macro must be defined (before includ‐
 *        ing  any  header  files)  in  order  to  obtain this definition.  Setting the _FILE_OFFSET_BITS feature test macro to 64 (rather than using O_LARGEFILE) is the preferred method of
 *        obtaining method of accessing large files on 32-bit systems
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param url : url of the file to open
 * @param flags : flags to use ( similar to open )
 * @param err : GError error report
 * @return This routine return a valid file descriptor if the operation is a success
 *  or -1 if error occured. In case of Error, err is set properly
 */
int gfal2_open(gfal2_context_t context, const char * url, int flags, GError ** err);

/**
 *
 * Same than \ref gfal2_open but allow to specify the default right of the file
 */
int gfal2_open2(gfal2_context_t context, const char * url, int flag, mode_t mode, GError ** err);

/**
 * \ref gfal2_creat is equivalent to \ref gfal2_open2 with flags equal to O_CREAT|O_WRONLY|O_TRUNC.
 */
int gfal2_creat (gfal2_context_t context, const char *filename, mode_t mode, GError ** err);

/**
 * @brief read data from a GFAL2 file descriptor
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param fd : GFAL2 file descriptor of the file
 * @param buff : buffer for read data
 * @param s_buff : maximum size to read
 * @param err : GError error report*
 * @return On  success,  the  number  of  bytes read is returned
 *   (zero indicates end of file), and the file position is advanced
 *   by this number.  It is not an error if this number is
 *   smaller than the number of bytes requested. or a negative value is returned when an error occurs
 *  In case of Error, err is set properly
 */
ssize_t gfal2_read(gfal2_context_t context, int fd, void* buff, size_t s_buff, GError ** err);

/**
 * @brief write data to a GFAL2 file descriptor
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param fd : GFAL2 file descriptor of the file
 * @param buff : buffer with data to write
 * @param s_buff : maximum size to write
 * @param err : GError error report*
 * @return On  success,  the  number  of  bytes written is returned
 *   (zero indicates end of file), and the file position is advanced
 *   by this number.  It is not an error if this number is
 *   smaller than the number of bytes requested. or a negative value is returned when an error occurs.
 *  In case of Error, err is set properly
 */
ssize_t gfal2_write(gfal2_context_t context, int fd, const void *buff, size_t s_buff, GError ** err);

/**
 * @brief move the file cursor
 *
 * Move the file cursor of the GFAL2 file descriptor fd to offset
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param fd : file descriptor
 * @param offset : new position of the cursor
 * @param whence : directive to follow, can be SEEK_SET, SEEK_CUR or SEEK_END
 * @param err : GError error report
 * @return new cursor position if success, -1 if failure, set err properly in case of error.
 */
off_t gfal2_lseek (gfal2_context_t context, int fd, off_t offset, int whence, GError ** err);

/**
 * @brief close a file GFAL2 descriptor
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param fd : file descriptor
 * @param err : GError error report
 * @return new cursor position if success, -1 if failure, set err properly in case of error.
 */
int gfal2_close(gfal2_context_t context, int fd, GError ** err);

/**
 * @brief flush all buffered data for the given file descriptor
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param fd : file descriptor
 * @param err : GError error report
 * @return new cursor position if success, -1 if failure, set err properly in case of error.
 */
int gfal2_flush(gfal2_context_t context, int fd, GError ** err);

/**
 * @brief read from file descriptor at a given offset
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param fd : file descriptor
 * @param buffer:  buffer for read data
 * @param count : maximum size to read
 * @param offset : operation offset
 * @param err : GError error report
 * @return number of read bytes, -1 on failure, set err properly in case of error.
 */
ssize_t gfal2_pread(gfal2_context_t context, int fd, void * buffer, size_t count, off_t offset, GError ** err);

/**
 * @brief write to file descriptor at a given offset
 *
 * @param context : gfal2 handle, see \ref gfal2_context_new
 * @param fd : file descriptor
 * @param buffer:  buffer for read data
 * @param count : maximum size to write
 * @param offset : operation offset
 * @param err : GError error report
 * @return number of written bytes, -1 on failure, set err properly in case of error.
 */
ssize_t gfal2_pwrite(gfal2_context_t context, int fd, const void * buffer, size_t count, off_t offset, GError ** err);

/**
	@}
    End of the FILE group
*/
#ifdef __cplusplus
}
#endif


#endif /* GFAL_FILE_API_H_ */
