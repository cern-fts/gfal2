/*
 * Copyright (c) CERN 2013-2017
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

#pragma once
#ifndef GFAL_POSIX_API_H_
#define GFAL_POSIX_API_H_

#if !defined(__GFAL2_H_INSIDE__) && !defined(__GFAL2_BUILD__)
#   warning "Direct inclusion of gfal2 headers is deprecated. Please, include only gfal_api.h or gfal_plugins_api.h"
#endif

#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#ifdef __APPLE__
#include <sys/xattr.h>
#else

#include <attr/xattr.h>

#endif

#include <common/gfal_common.h>
#include <common/gfal_constants.h>

#ifdef __cplusplus
extern "C"
{
#endif


/*!
	\defgroup posix_group POSIX-like API
*/

/*!
	\addtogroup posix_group
	@{
*/

/**
 * Warnings about POSIX functions : some protocols do not permit a full POSIX support ( ex : no symlinks in srm:// URLs )
 * In case of a call to a unsupported function, -1 code is return and an error is set to EPROTONOSUPPORT
 */



/**
 * @brief change file access permissions
 *
 *  gfal_chmod changes the permissions of each given file according to mode,
 *  Similar behavior to the POSIX chmod ( man 2 chmod )
 *
 * @param url : url of the file or the folder, can be in all supported protocols (lfn, srm, file, guid,..)
 * @param mode : right to configure
 * @return return 0 if success else -1 if errors occures.
 *  In case of errors, see @ref gfal_posix_check_error() for error management
 */
int gfal_chmod(const char *url, mode_t mode);

/**
 * @brief  change the name or location of a file
 *
 *  gfal_rename will rename the specified files by replacing the first occurrence of from in their name by to.
 *  gfal_rename implies two URL with the same protocols and generally on the same server.
 *  Similar behavior to the POSIX rename ( man 2 rename )
 *
 * @param oldurl : the old url of the file
 * @param newurl : the new url of the file
 * @return : return 0 if success, else -1 if errors.
 *  In case of errors, see @ref gfal_posix_check_error() for error management
*/
int gfal_rename(const char *oldurl, const char *newurl);


/**
 *  @brief  get the file status, follow links
 *
 *  return informations about the given files
 *  Similar behavior to the POSIX stat ( man 2 stat )
 *
 * @param url : url of the file
 * @param st  : pointer to an allocated struct stat
 * @return    : return 0 if success, else -1 if errors.
 *  In case of errors, see @ref gfal_posix_check_error() for error management
 */
int gfal_stat(const char *url, struct stat *st);

/**
 *  @brief  get the file status, does not follow links
 *
 *  return informations about the given files and links
 *  Similar behavior to the POSIX lstat ( man 2 lstat )
 *
 *  In case of protocols without the support of links, GFAL 2.0 \
 *  convert this call to @ref gfal_stat
 *
 *
 * @param url : url of the file
 * @param st  : pointer to an allocated struct stat
 * @return    : return 0 if success, else -1 if errors.
 *  In case of errors, see @ref gfal_posix_check_error() for error management
 */
int gfal_lstat(const char *url, struct stat *st);

/**
 * @brief check user permissions for a file
 *
 *  Similar behavior to the POSIX access  ( man 2 access )
 *
 * @param url : url of the file to access
 * @param amode : access mode to check (R_OK, W_OK, X_OK or F_OK)
 * @return This routine return 0 if the operation was successful, or -1 if error occured.
 *  In case of errors, see @ref gfal_posix_check_error() for error management
 */
int gfal_access(const char *url, int amode);

/**
 * @brief resolve a link destination
 *
 *  Similar behavior to the POSIX readlink ( man 2 readlink )
 *
 *  Some protocols can not support this functionality, \
 *  check the protocol specification for more information.
 *
 * @param url : url of the file to access, can be in supported protocols (lfn, srm, file, guid,..)
 * @param buff : buffer to fill
 * @param buffsiz : maximum size of the link destination
 * @return This routine return the size of the link destination \
 *  if the operation was successful, or -1 if error occured.
 *  In case of errors, see @ref gfal_posix_check_error() for error management
 */
ssize_t gfal_readlink(const char *url, char *buff, size_t buffsiz);

/**
 * @brief create a symbolic link
 *
 *  Similar behavior to the POSIX symlink  ( man 2 symlink )
 *
 *  Some protocols can not support this functionality, \
 *  check the protocol specification for more information.
 *
 * @param oldurl : url of the original file
 * @param newurl : url of the symlink
 * @return This routine return 0 if the operation was successful, or -1 if error occured.
 *  In case of errors, see @ref gfal_posix_check_error() for error management
 */
int gfal_symlink(const char *oldurl, const char *newurl);

/**
 * @brief unlink a file, delete it
 *
 *  Similar behavior to the POSIX unlink  ( man 2 unlink )
 *
 * @param url : url of the file to delete
 * @return This routine return 0 if the operation was successful, or -1 if error occured.
 *  In case of errors, see @ref gfal_posix_check_error() for error management
 */
int gfal_unlink(const char *url);

/**
 * @brief create a directory
 *
 *  Similar behavior to the POSIX mkdir( man 2 mkdir )
 *
 * @param url : url of the directory to create
 * @param mode : initial mode right of the directory
 * @return This routine return 0 if the operation was successful, or -1 if error occured.
 *  In case of errors, see @ref gfal_posix_check_error() for error management
 */
int gfal_mkdir(const char *url, mode_t mode);

/**
 * @brief open a directory
 *
 *  Similar behavior to the POSIX opendir ( man 2 opendir )
 *
 * @param url : url of the directory to list
 * @return This routine return a directory descriptor if the operation was successful, or -1 if error occured.
 *  In case of errors, see @ref gfal_posix_check_error() for error management
 */
DIR *gfal_opendir(const char *url);

/**
 * @brief read a directory
 *
 *  the directory descriptor must be provided by @ref gfal_opendir
 *  Similar behavior to the POSIX readdir  ( man 2 readdir )
 *
 * @param d : descriptor of the directory
 * @return This routine return the directory information if the operation was successful,
 *   NULL if the end of the directory is reached
 *   or  if error occured.
 *  In case of errors, see @ref gfal_posix_check_error() for error management
 */
struct dirent *gfal_readdir(DIR *d);

/**
 * @brief close a directory
 *
 *  Similar behavior to the POSIX closedir  ( man 2 closedir )
 *
 * @param d : descriptor of the directory
 * @return This routine return 0 if the operation was successful, or -1 if error occured.
 *  In case of errors, see @ref gfal_posix_check_error() for error management
 */
int gfal_closedir(DIR *d);

/**
 * @brief delete a directory
 *
 *  Similar behavior to the POSIX rmdir ( man 2 rmdir )
 *
 * @param url : url of the directory to suppress
 * @return This routine return 0 if the operation was successful, or -1 if error occured.
 *  In case of errors, see @ref gfal_posix_check_error() for error management
 */
int gfal_rmdir(const char *url);

/**
 *  @brief creat a file
 *
 *  Similar behavior to the POSIX creat  ( man 2 creat )
 *  gfal_creat is similar to @ref gfal_open with the flags O_CREAT|O_WRONLY|O_TRUNC
 *
 * @param url : url of the file to creat
 * @param mode : mode of the file.
 * @return This routine return a valid file descriptor if the operation is a success
 *  or -1 if error occured.
 *  In case of errors, see @ref gfal_posix_check_error() for error management
 */
int gfal_creat(const char *url, mode_t mode);

/**
 * @brief open a file
 *
 *  Similar behavior to the POSIX open  ( man 2 open )
 *  gfal_open supports the same flags than the POSIX open call
 *
 *  O_TRUNC
 *		  If  the file already exists and is a regular file and the open mode allows writing (i.e., is O_RDWR or O_WRONLY) it will be truncated to length 0.  If the file is a FIFO or termi‐
 *		  nal device file, the O_TRUNC flag is ignored.  Otherwise the effect of O_TRUNC is unspecified.
 *
 *  O_APPEND
 *		  The  file  is opened in append mode.  Before each write(2), the file offset is positioned at the end of the file, as if with lseek(2).  O_APPEND may lead to corrupted files on NFS
 *		  file systems if more than one process appends data to a file at once.  This is because NFS does not support appending to a file, so the client kernel has  to  simulate  it,  which
 *		  can't be done without a race condition.
 *
 *   O_CREAT
 *            If the file does not exist it will be created.  The owner (user ID) of the file is set to the effective user ID of the process.  The group ownership (group ID) is  set  either  to
 *            the  effective  group  ID of the process or to the group ID of the parent directory (depending on file system type and mount options, and the mode of the parent directory, see the
 *             mount options bsdgroups and sysvgroups described in mount(8)).
 *
 *             mode specifies the permissions to use in case a new file is created.  This argument must be supplied when O_CREAT is specified in flags; if O_CREAT is not specified, then mode  is
 *             ignored.   The  effective  permissions  are  modified  by  the process's umask in the usual way: The permissions of the created file are (mode & ~umask).  Note that this mode only
 *             applies to future accesses of the newly created file; the open() call that creates a read-only file may well return a read/write file descriptor.
 *
 *
 *
 *       O_DIRECT
 *             Try to minimize cache effects of the I/O to and from this file.  In general this will degrade performance, but it is useful in special situations, such  as  when  applications  do
 *             their  own  caching.   File I/O is done directly to/from user space buffers.  The O_DIRECT flag on its own makes at an effort to transfer data synchronously, but does not give the
 *             guarantees of the O_SYNC that data and necessary metadata are transferred.  To guarantee synchronous I/O the O_SYNC must be used in addition to O_DIRECT.  See NOTES below for fur‐
 *             ther discussion.
 *
 *      O_LARGEFILE
 *             (LFS) Allow files whose sizes cannot be represented in an off_t (but can be represented in an off64_t) to be opened.  The _LARGEFILE64_SOURCE macro must be defined (before includ‐
 *             ing  any  header  files)  in  order  to  obtain this definition.  Setting the _FILE_OFFSET_BITS feature test macro to 64 (rather than using O_LARGEFILE) is the preferred method of
 *             obtaining method of accessing large files on 32-bit systems
 *
 *
 * @param url : url of the file to open
 * @param flags : flags to use ( similar to open )
 * @param mode (optional) : mode of the file in case of a new file.
 * @return This routine return a valid file descriptor if the operation is a success
 *  or -1 if error occured.
 *  In case of errors, see @ref gfal_posix_check_error() for error management
 */
int gfal_open(const char *url, int flags, ...);

/**
 * @brief reposition read/write file offset
 *
 *  Similar behavior to the POSIX lseek ( man 2 lseek )
 *
 * @param fd : gfal file descriptor of the file
 * @param off : offset to set
 * @param flags : offset mode
 * @return This routine return the current offset if the operation is a success
 *  or -1 if error occured.
 *  In case of errors, see @ref gfal_posix_check_error() for error management
 */
off_t gfal_lseek(int fd, off_t off, int flags);

/**
 * @brief close a gfal file descriptor
 *
 *  Similar behavior to the POSIX close  ( man 2 close )
 *
 * @param fd : gfal file descriptor of the file
 * @return This routine return 0 if the operation is a success
 *  or -1 if error occured.
 *  In case of errors, see @ref gfal_posix_check_error() for error management
 */
int gfal_close(int fd);

/**
 * @brief read  from a gfal file descriptor
 *
 *  Similar behavior to the POSIX read  ( man 2 read )
 *
 * @param fd : gfal file descriptor of the file
 * @param buff : buffer for the read content
 * @param size : maximum size to read
 * @return On  success,  the  number  of  bytes read is returned
 *   (zero indicates end of file), and the file position is advanced
 *   by this number.  It is not an error if this number is
 *   smaller than the number of bytes requested.In case of errors,
 *   see @ref gfal_posix_check_error() for error management
 *  In case of errors, see @ref gfal_posix_check_error() for error management
 */
ssize_t gfal_read(int fd, void *buff, size_t size);

/**
 * @brief write  from a gfal file descriptor
 *
 *  Similar behavior to the POSIX write  ( man 2 write )
 *
 * @param fd : gfal file descriptor of the file
 * @param buff : buffer for the write content
 * @param size : maximum size to write
 * @return On success, the  number of bytes written is returned.
 *   In case of errors, -1 is returned
 *   see @ref gfal_posix_check_error() for error management
 */
ssize_t gfal_write(int fd, const void *buff, size_t size);

/**
 * @brief flush the given file descriptor
 *
 *  flush the current fiel descriptor, clear the cache  \
 * and commit the changes.
 *  This call is reserved fora futur usage.
 *
 * @param fd : gfal file descriptor of the file
 * @return On success, return 0.
 *   In case of errors, -1 is returned
 *   see @ref gfal_posix_check_error() for error management
 */
int gfal_flush(int fd);

/**
 * @brief parallel read from a gfal file descriptor
 *
 *  Similar behavior to the POSIX pread  ( man 2 pread )
 *
 *  In gfal 2.0, this call is thread-safe and allows the usage of \
 *  parallels read at the same time on the same file descriptor.
 *
 *  This is the recommended way to execute parallels transfers.
 *
 *  This gfal_pread is emulated on filesystem that does not support it
 *  explicitely ( ex : rfio )
 *
 * @param fd : gfal file descriptor of the file
 * @param buff : buffer for the read content
 * @param size : maximum size to read
 * @param offset : offset where the read start
 * @return On  success,  the  number  of  bytes read is returned
 *   (zero indicates end of file), and the file position is advanced
 *   by this number.  It is not an error if this number is
 *   smaller than the number of bytes requested.In case of errors,
 *   see @ref gfal_posix_check_error() for error management
 *  In case of errors, see @ref gfal_posix_check_error() for error management
 */
ssize_t gfal_pread(int fd, void *buff, size_t size, off_t offset);

/**
 * @brief parallel write from a gfal file descriptor
 *
 *  Similar behavior to the POSIX pwrite  ( man 2 pwrite )
 *
 *  In gfal 2.0, this function is thread-safe and allows the usage of
 *  parallels write at the same time on the same file descriptor.
 *
 *  This is the recommended way to execute parallels transfers.
 *
 *  This gfal_pwrite is emulated on filesystem that does not support it
 *  explicitely ( ex : rfio )
 *
 * @param fd : gfal file descriptor of the file
 * @param buff : buffer for the write content
 * @param size : maximum size to write
 * @param offset : offset where the write start
 * @return On  success,  the  number  of  bytes read is returned
 *   (zero indicates end of file), and the file position is advanced
 *   by this number.  It is not an error if this number is
 *   smaller than the number of bytes requested.In case of errors,
 *   see @ref gfal_posix_check_error() for error management
 *  In case of errors, see @ref gfal_posix_check_error() for error management
 */
ssize_t gfal_pwrite(int fd, const void *buff, size_t size, off_t offset);

/**
 * @brief retrieve an extended attribute value
 *
 *  similar to the getxattr function
 *
 * In gfal 2.0, this is the standard way to interact \
 *   with advanced file properties
 *
 *  ex : gfal_getxattr("srm://myurl/endpoint", "user.replicas",
 * 						buff, 256);
 *
 * @param url : url of the file
 * @param name : key of the extended attribute
 * @param value : buffer for the extended attribute value
 * @param size : maximum size of the buffer
 * @return On success, return the size of the extended attribute.
 *   In case of errors, -1 is returned
 *   see @ref gfal_posix_check_error() for error management
 */
ssize_t gfal_getxattr(const char *url, const char *name,
    void *value, size_t size);

/**
 * @brief retrieve a list of the extended attributes availables
 *
 *  similar to the etxattr function
 *
 *
 * @param url : url of the file
 * @param list : buffer for the list of availables attributes concatened.
 * @param size : maximum size of the buffer
 * @return On success, return the size of the list.
 *   In case of errors, -1 is returned
 *   see @ref gfal_posix_check_error() for error management
 */
ssize_t gfal_listxattr(const char *url, char *list, size_t size);

/**
 * @brief define an extended attribute value
 *
 *  similar to the listxattr function
 *
 *  In gfal 2.0, this is the standard way to interact \
 *   with advanced file properties
 *
 *  ex : gfal_setxattr("srm://myurl/endpoint", "user.status",
 * 						"ONLINE", strlen("ONLINE"),0);
 *       // ->  simple brings_online
 *
 * @param url : url of the file
 * @param name : key of the extended attribute
 * @param value : buffer for the extended attribute value
 * @param size : size of the attribute value
 * @param flags : mode
 * @return On success, return 0 .
 *   In case of errors, -1 is returned
 *   see @ref gfal_posix_check_error() for error management
 */

int gfal_setxattr(const char *url, const char *name,
    const void *value, size_t size, int flags);

/**
 * @brief delete a extended attribute value
 *
 *  similar to the removexattr function
 *
 *
 * @param url : url of the file
 * @param name : key of the extended attribute
 * @return On success, return 0.
 *   In case of errors, -1 is returned
 *   see @ref gfal_posix_check_error() for error management
 */
int gfal_removexattr(const char *url, const char *name);

/**
 * safe and easy error management :
 * -> if last error
 *     -> print on stderr, return 1
 * -> else do nothing
 * @return 0 if no error, 1 else
 */
GFAL2_DEPRECATED_NOALT int gfal_posix_check_error();

/**
 * clear the last error  \
 * provided for convenience.
 */
void gfal_posix_clear_error();

/**
 * display the last error  to the standard output \
 * and clear it.
 * provided for convenience.
 */
GFAL2_DEPRECATED_NOALT void gfal_posix_release_error();

/**
 * Get the string representation of the last error
 * @param buff_err : buffer where string representation will be stored.
 * @param s_err : maximum message size
 * @return return pointer to buff_err for convenience
 */
char *gfal_posix_strerror_r(char *buff_err, size_t s_err);

/**
 *  display the last error recorded to the standard output,
 *  provided for convenience.
 */
GFAL2_DEPRECATED_NOALT void gfal_posix_print_error();

/**
 * return the code associated with the last error.
 * In case of POSIX calls and when possible, this code is similar to the local errno
 * @return last error code or 0 if nothing
 */
int gfal_posix_code_error();

/**
 * get a string representation of the gfal2 version
 */
const char *gfal2_version();

/**
 * @brief get context for advanced operation
 * Return the gfal2 context used for POSIX operations
 * Allow to do advanced operation ( config, checksum, transfer ) on this context
 * @warning Delete this context leads to undefined behavior.
 */
gfal2_context_t gfal_posix_get_handle();

/**
	@}
	End of the POSIX group
*/
#ifdef __cplusplus
}
#endif


#endif /* GFAL_POSIX_API_H_ */
