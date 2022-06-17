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
#ifndef _GFAL_COMMON_PLUGIN_INTERFACE_H_
#define _GFAL_COMMON_PLUGIN_INTERFACE_H_

#if !defined(__GFAL2_H_INSIDE__) && !defined(__GFAL2_BUILD__)
#   warning "Direct inclusion of gfal2 headers is deprecated. Please, include only gfal_api.h"
#endif

#include "gfal_common.h"
#include "gfal_constants.h"
#include "gfal_file_handle.h"
#include <transfer/gfal_transfer_plugins.h>

#include <glib.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Helper for the versioned plugin names
 */
#define GFAL2_PLUGIN_VERSIONED(plugin, version) plugin "-" version

/**
 * classical data access plugin
 */
#define GFAL_PLUGIN_PRIORITY_DATA 0;      /**< The plugin provides IO */
#define GFAL_PLUGIN_PRIORITY_CATALOG 100; /**< The plugin provides namespace operations */
#define GFAL_PLUGIN_PRIORITY_CACHE 200;   /**< The plugin provides a cache */


typedef struct _gfal_plugin_interface gfal_plugin_interface;
typedef gpointer plugin_handle;

/**
 * Plugin check type
 */
typedef enum _plugin_mode {
    GFAL_PLUGIN_ALL=0,
    GFAL_PLUGIN_ACCESS,
    GFAL_PLUGIN_CHMOD,
    GFAL_PLUGIN_RENAME,
    GFAL_PLUGIN_SYMLINK,
    GFAL_PLUGIN_STAT,
    GFAL_PLUGIN_LSTAT,
    GFAL_PLUGIN_MKDIR,
    GFAL_PLUGIN_RMDIR,
    GFAL_PLUGIN_OPENDIR,	 /**< concat of opendir readdir, closedir*/
    GFAL_PLUGIN_OPEN, 		 /**< concat of open read, close*/
    GFAL_PLUGIN_RESOLVE_GUID,
    GFAL_PLUGIN_GETXATTR,
    GFAL_PLUGIN_SETXATTR,
    GFAL_PLUGIN_LISTXATTR,
    GFAL_PLUGIN_READLINK,
    GFAL_PLUGIN_UNLINK,
    GFAL_PLUGIN_CHECKSUM,
    GFAL_PLUGIN_MKDIR_REC,
    GFAL_PLUGIN_BRING_ONLINE,
    GFAL_PLUGIN_ARCHIVE,
    GFAL_PLUGIN_QOS_CHECK_CLASSES,
    GFAL_PLUGIN_CHECK_FILE_QOS,
    GFAL_PLUGIN_CHECK_QOS_AVAILABLE_TRANSITIONS,
    GFAL_PLUGIN_CHECK_TARGET_QOS,
    GFAL_PLUGIN_CHANGE_OBJECT_QOS,
    GFAL_PLUGIN_TOKEN
} plugin_mode;

/**
 * Check modes for transfers
 */
typedef enum _gfal_url2_check {
    GFAL_FILE_COPY,
    GFAL_BULK_COPY
} gfal_url2_check;

/**
 * Prototype of the plugins entry point
 *
 *  return gfal_plugin_interface must be allocated with \êef gfal_plugin_interface_new
 *
 * @param handle : gfal2_context_t of the current call
 * @param err : Error report in case of fatal error while the plugin load.
 *
 * */
typedef gfal_plugin_interface* (*gfal_plugin_init_t)(gfal2_context_t handle, GError** err);


/**
 * @struct _gfal_plugin_interface
 *
 *  Main interface that MUST be returned the entry point function "gfal_plugin_init" of each GFAL 2.0 plugin.
 *  the minimum calls are : getName, plugin_delete, check_plugin_url
 *  all the unused function pointers must be set to NULL
 */
struct _gfal_plugin_interface {
	 //! @cond
     // internal gfal data: do not modify it
	void * gfal_data;
	//! @endcond

    // plugin management

	/**
     *  plugin reserved pointer, free to use for plugin's internal data, passed to any function
	 *
	 * */
	plugin_handle plugin_data;
    /**
     * plugin priority
     *  SHOULD be defined to GFAL_PLUGIN_PRIORITY_DATA by default
     * */
    int priority;
	/**
	 *  MANDATORY : return a the string id of the plugin.
	 *  string id must be short, constant and unique ( ex : "plugin_gridftp" )
	 *  @return the constant identity string
	 * */
	const char* (*getName)();
	/**
	 * OPTIONAL : Last call before the unload of the plugin for the associated handle
	 *        You can use this to clear your internal context
	 * @param plugin_data : internal plugin data
	 */
	void (*plugin_delete)(plugin_handle plugin_data);

	/**
	 *  OPTIONAL: Check the availability of the given operation on this plugin for the given URL
	 *
	 *  @warning This function is a key function of GFAL 2.0, It MUST be as fast as possible.
	 *  @param plugin_data : internal plugin data
	 *  @param url : URL to check for the protocol compatibility
	 *  @param operation :  operation to check
	 *  @param err : error handle, should be used ONLY in case of major failure.
	 *  @return must return TRUE if the operation is compatible with this plugin, else FALSE
	 */
	gboolean (*check_plugin_url)(plugin_handle plugin_data, const char* url, plugin_mode operation, GError** err);

    // FILE API

	/**
	 *  OPTIONAL : gfal_access function  support
	 *  @param plugin_data : internal plugin data
	 *  @param url : URL for access checking
	 *  @param mode : mode to check ( see man 2 access )
	 *  @param err : Error report, the code field of err should be set to errno value when possible
	 *  @return 0 or -1 if error occurs,
	 *          err MUST be set in case of error
	 * */
	int (*accessG)(plugin_handle plugin_data, const char* url, int mode, GError** err);

	/**
	 *  OPTIONAL : gfal_chmod function  support
	 *  @param plugin_data : internal plugin data
	 *  @param url : URL of the file
	 *  @param mode : mode to set
	 *  @param err : Error report, the code field of err should be set to errno value when possible
	 *  @return 0 or -1 if error occurs,
	 *          err MUST be set in case of error
	 * */
	int	(*chmodG)(plugin_handle plugin_data, const char * url, mode_t mode, GError** err);
	/**
	 *  OPTIONAL : gfal_rename function  support
	 *  @param plugin_data : internal plugin data
	 *  @param oldurl : old url of the file
	 *  @param urlnew : new url of the file
	 *  @param err : Error report, the code field of err should be set to errno value when possible
	 *  @return 0 or -1 if error occurs,
	 *          err MUST be set in case of error
	 * */
	int	(*renameG)(plugin_handle plugin_data, const char * oldurl, const char * urlnew, GError** err);
	/**
	 *  OPTIONAL : gfal_symlink function  support
	 *  @param plugin_data : internal plugin data
	 *  @param oldurl : old url of the file
	 *  @param urlnew : symlink to create
	 *  @param err : Error report, the code field of err should be set to errno value when possible
	 *  @return 0 or -1 if error occurs,
	 *          err MUST be set in case of error
	 * */
	int (*symlinkG)(plugin_handle plugin_data, const char* oldurl, const char* newold, GError** err);

	/**
	 *  OPTIONAL : gfal_stat function  support
	 *  @param plugin_data : internal plugin data
	 *  @param url : url to stat
	 *  @param buf : informations of the file
	 *  @param err : Error report, the code field of err should be set to errno value when possible
	 *  @return 0 or -1 if error occurs,
	 *          err MUST be set in case of error
	 * */
	int (*statG)(plugin_handle plugin_data , const char* url, struct stat *buf, GError** err);

	/**
	 *  OPTIONAL : gfal_lstat function  support
	 * 			In case of non support for this function, calls to @ref gfal_lstat are mapped to @ref gfal_stat.
	 *
	 *  @param plugin_data : internal plugin data
	 *  @param url : url to stat
	 *  @param buf : informations of the file
	 *  @param err : Error report, the code field of err should be set to errno value when possible
	 *  @return 0 or -1 if error occurs,
	 *          err MUST be set in case of error
	 * */
	int (*lstatG)(plugin_handle plugin_data, const char* url, struct stat *buf, GError** err);
	/**
	 *  OPTIONAL : gfal_readlink function  support
	 *
	 *  @param plugin_data : internal plugin data
	 *  @param url : url to stat
	 *  @param buff : buffer for the readlink result
	 *  @param size_t : buffsize maximum size to fill in the buffer
	 *  @param err : Error report, the code field of err should be set to errno value when possible
	 *  @return number of bytes in buff in case of success or -1 if error occurs,
	 *          err MUST be set in case of error
	 * */
	ssize_t (*readlinkG)(plugin_handle plugin_data, const char* url, char* buff, size_t buffsiz, GError** );

	/**
	 *  OPTIONAL : gfal_opendir function  support
	 *
	 *  @param plugin_data : internal plugin data
	 *  @param url : url of directory to list
	 *  @param err : Error report, the code field of err should be set to errno value when possible
	 *  @return gfa_file_handle in case of success or NULL if error,
	 *          err MUST be set in case of error
	 * */
	 gfal_file_handle (*opendirG)(plugin_handle plugin_data, const char* url, GError** err);

	/**
	 *  MANDATORY IF OPENDIR : gfal_closedir function  support
	 *
	 *  @param plugin_data : internal plugin data
	 *  @param dir_desc : directory descriptor to use
	 *  @param err : Error report, the code field of err should be set to errno value when possible
	 *  @return 0 in case of success or -1 if error,
	 *          err MUST be set in case of error
	 * */
	 int (*closedirG)(plugin_handle plugin_data, gfal_file_handle dir_desc, GError** err);

	/**
	 *  MANDATORY IF OPENDIR : gfal_readdir function  support
	 *
	 *  @param plugin_data : internal plugin data
	 *  @param dir_desc : directory descriptor to use
	 *  @param err : Error report, the code field of err should be set to errno value when possible
	 *  @return dirent information in case of success or NULL if end of listing or error,
	 *          err MUST be set in case of error
	 * */
	struct dirent* (*readdirG)(plugin_handle plugin_data, gfal_file_handle dir_desc, GError** err);

	/**
	 *  OPTIONAL : gfal_mkdir function  support
	 *
	 *  @param plugin_data : internal plugin data
	 *  @param url : url of the directory to create
     *  @param mode : right mode of the created directory
     *  @param rec_flag : recursive mode, if enabled the plugin MUST create the parent directories if needed,
     *       if the rec_flag is not supported by this plugin, the plugin MUST return a negative value and set the GError errcode to ENOENT
	 *  @param err : Error report, the code field of err should be set to errno value when possible
	 *  @return 0 in case of success or -1 if error occurs,
	 *          err MUST be set in case of error
	 * */
    int (*mkdirpG)(plugin_handle plugin_data, const char* url, mode_t mode, gboolean rec_flag, GError** err);
	/**
	 *  OPTIONAL : gfal_rmdir function  support
	 *
	 *  @param plugin_data : internal plugin data
	 *  @param url : url of the directory to delete
	 *  @param err : Error report, the code field of err should be set to errno value when possible
	 *  @return 0 in case of success or -1 if error occurs,
	 *          err MUST be set in case of error
	 * */
	int (*rmdirG)(plugin_handle plugin_data, const char* url, GError** err);

	 // basic file operations
	/**
	 *  OPTIONAL : gfal_open function  support
	 *
	 *  @param plugin_data : internal plugin data
	 *  @param url : url of the directory to open
	 *  @param flag : open flags
	 *  @param mode : mode of the file, in case of creation
	 *  @param err : Error report, the code field of err should be set to errno value when possible
	 *  @return gfal_file_handle in case of success or NULL if error occurs,
	 *          err MUST be set in case of error
	 * */
	 gfal_file_handle (*openG)(plugin_handle plugin_data, const char* url, int flag, mode_t mode, GError**);

	/**
	 *  MANDATORY IF OPEN : gfal_read function  support
	 *
	 * */
	 ssize_t (*readG)(plugin_handle, gfal_file_handle fd, void* buff, size_t count, GError**);

	/**
	 *  MANDATORY IF OPEN : gfal_write function  support
	 *
	 * */
	 ssize_t (*writeG)(plugin_handle, gfal_file_handle fd, const void* buff, size_t count, GError**);

	/**
	 *  MANDATORY IF OPEN : gfal_close function  support
	 *
	 * */
	 int (*closeG)(plugin_handle, gfal_file_handle fd, GError **);

	/**
	 *  MANDATORY IF OPEN : gfal_lseek function  support
	 *
	 * */
	 off_t (*lseekG)(plugin_handle, gfal_file_handle fd, off_t offset, int whence, GError** err);

	 // vector operations
	/**
	 *  OPTIONAL : gfal_pread function  support
	 *
	 *  Allow fast parallels read support, If not implemented, this function is simulated by GFAL 2.0
	 *
	 * */
	 ssize_t (*preadG)(plugin_handle, gfal_file_handle fd, void* buff, size_t count, off_t offset, GError**);
	/**
	 *  OPTIONAL : gfal_pwriteG function  support
	 *
	 *  Allow fast parallels write support, If not implemented, this function is simulated by GFAL 2.0
	 *
	 * */
	 ssize_t (*pwriteG)(plugin_handle, gfal_file_handle fd, const void* buff, size_t count, off_t offset, GError**);


	 // remove operations
	/**
	 *  OPTIONAL : gfal_unlink function  support
	 *
	 *  @param plugin_data : internal plugin data
     *  @param url : url of the file
	 *  @param err : Error report, the code field of err should be set to errno value when possible
	 *  @return 0 in case of success or -1 if error occurs,
	 *          err MUST be set in case of error
	 * */
	 int (*unlinkG)(plugin_handle plugin_data, const char* url, GError** err);

	 // advanced attributes management
	/**
	 *  OPTIONAL : gfal_getxattr function  support
	 *
	 *  @param plugin_data : internal plugin data
     *  @param url : url of the file
	 *  @param key : key of the attribute to get
	 *  @param buff : buffer for the attribute content
	 *  @param s_buff : maximum buffer size
	 *  @param err : Error report, the code field of err should be set to errno value when possible
	 *  @return size of the attribute in case of success or -1 if error occurs,
	 *          err MUST be set in case of error
	 * */
	 ssize_t (*getxattrG)(plugin_handle plugin_data, const char* url, const char* key,
							void* buff, size_t s_buff, GError** err);
	/**
	 *  OPTIONAL : gfal_listxattr function  support
	 *
	 *  @param plugin_data : internal plugin data
     *  @param url : url of the file
	 *  @param list : buffer for the list attribute content
	 *  @param s_buff : maximum buffer size
	 *  @param s_list : Error report, the code field of err should be set to errno value when possible
	 *  @return size of the list in case of success or -1 if error occurs,
	 *          err MUST be set in case of error
	 * */
	 ssize_t (*listxattrG)(plugin_handle plugin_data, const char* url, char* list, size_t s_list, GError** err);

	/**
	 *  OPTIONAL : gfal_setxattr function  support
	 *
	 *  @param plugin_data : internal plugin data
     *  @param url : url of the file
	 *  @param key : key of the attribute to set
	 *  @param buff : buffer for the attribute content
	 *  @param s_buff : maximum buffer size
	 *  @param flas : set/get flags
	 *  @param err : Error report, the code field of err should be set to errno value when possible
	 *  @return 0 or -1 if error occurs,
	 *          err MUST be set in case of error
	 * */
	 int (*setxattrG)(plugin_handle plugin_data, const char* url, const char* key,
					const void* buff , size_t s_buff, int flags, GError** err);

     /**
      *  OPTIONAL : checksum calculation function support ( transfer consistency check, gfal_checksum )
      *
      * @param plugin_data : internal plugin data
      * @param url : url of the file
      * @param check_type : string of the checksum type ( \ref GFAL_CHKSUM_MD5, \ref GFAL_CHKSUM_SHA1, .. )
      * @param start_offset : offset in the file where the checksum calculation will start ( 0 : from begining )
      * @param data_length : size of data to compute for the checksum after start_offset ( 0 -: full file )
      * @param checksum_buffer : buffer with checksum string as result
      * @param buffer_length : maximum buffer length
      * @param err : GError error support
      * @return dynamically allocated checksum string if success, else NULL and err MUST be set
      *  error code MUST be ENOSYS in case of :
      *         - checksum calculation with offset is not supported
      *         - the specified checksum algorithm is not supported
      */
     int (*checksum_calcG)(plugin_handle data, const char* url, const char* check_type,
                            char * checksum_buffer, size_t buffer_length,
                            off_t start_offset, size_t data_length,
                            GError ** err);

     // TRANSFER API
     /**
      *  OPTIONAL:  if transfer support,
      *  should return TRUE if the plugin is able to execute third party transfer from src to dst url
      *
      */
     int(*check_plugin_url_transfer)(plugin_handle plugin_data, gfal2_context_t, const char* src, const char* dst, gfal_url2_check check);

     /**
       *
       *  OPTIONAL:  if transfer support,
       *  Execute a filecopy operation for the given parameters
       *  @param plugin_data : internal plugin context
       *  @param gfal2_context_t context : gfal 2 handle
       *  @param params: parameters for the current transfer, see gfalt_params calls
       *  @param src : source file to copy
       *  @param dst : destination file
       *  @param err : GError err report
       *
    */
     int (*copy_file)(plugin_handle plugin_data, gfal2_context_t context, gfalt_params_t params, const char* src, const char* dst, GError** );

     /**
      *
      * OPTIONAL: Requests to stage a file to the fist layer on a hierarchical SE.
      * @param plugin_data : internal plugin context
      * @param url : The url of the file
      * @param pintime : Time the file should stay in the cache
      * @param timeout : Operation timeout
      * @param token Where to put the retrieved token.
      * @param tsize The size of the buffer pointed by token.
      * @param async If true (!= 0), the call will not block. The caller will need
      *              to use bring_online_poll later.
      * @param err:  GError error support
      * @return      -1 on error (and err is set). 0 on success. 1 if the file has been staged.
      */
     int (*bring_online)(plugin_handle plugin_data, const char* url,
                         time_t pintime, time_t timeout,
                         char* token, size_t tsize,
                         int async,
                         GError** err);

     /**
      *
      * OPTIONAL: Requests to stage a file to the fist layer on a hierarchical SE.
      * @param plugin_data : internal plugin context
      * @param url : The url of the file
      * @param metadata : Staging metadata
      * @param pintime : Time the file should stay in the cache
      * @param timeout : Operation timeout
      * @param token Where to put the retrieved token.
      * @param tsize The size of the buffer pointed by token.
      * @param async If true (!= 0), the call will not block. The caller will need
      *              to use bring_online_poll later.
      * @param err:  GError error support
      * @return      -1 on error (and err is set). 0 on success. 1 if the file has been staged.
      */
     int (*bring_online_v2)(plugin_handle plugin_data,
                         const char* url, const char* metadata,
                         time_t pintime, time_t timeout,
                         char* token, size_t tsize,
                         int async,
                         GError** err);

     /**
      * OPTIONAL: Polling the bring_online request (mandatory if bring online is supported)
      * @param url   The same URL as was passed to bring_online_async
      * @param token The token as returned by bring_online_async
      * @return      -1 on error (and err is set). 0 on success. 1 if the file has been staged.
      */
     int (*bring_online_poll)(plugin_handle plugin_data, const char* url,
                              const char* token, GError** err);

     /**
      * OPTIONAL: Releases a previously staged file (mandatory if bring online is supported)
      * @param plugin_data : internal plugin context
      * @param url :  The url of the file
      * @param token: The request token. If NULL,
      * @param err:   GError error support
      */
     int (*release_file)(plugin_handle plugin_data, const char* url,
                         const char* token,
                         GError** err);

     /**
      *  OPTIONAL : gfal_readdirpp function support
      *             Allow directory listing + get meta-data in one operation
      *
      *  @param plugin_data : internal plugin data
      *  @param dir_desc : directory descriptor to use
      *  @param st : struct stat to fill
      *  @param err : Error report, the code field of err should be set to errno value when possible
      *  @return dirent information in case of success or NULL if end of listing or error,
      *          err MUST be set in case of error
      * */
     struct dirent* (*readdirppG)(plugin_handle plugin_data, gfal_file_handle dir_desc, struct stat* st, GError** err);


     /**
      *
      * OPTIONAL: Requests to stage a file to the fist layer on a hierarchical SE.
      * @param plugin_data : internal plugin context
      * @param nbfiles : number of files
      * @param urls : The urls of the files
      * @param pintime : Time the file should stay in the cache
      * @param timeout : Operation timeout
      * @param token Where to put the retrieved token.
      * @param tsize The size of the buffer pointed by token.
      * @param async If true (!= 0), the call will not block. The caller will need
      *              to use bring_online_poll later.
      * @param err:  GError error support
      * @return      -1 on error (and err is set). 0 on success. 1 if the file has been staged.
      */
     int (*bring_online_list)(plugin_handle plugin_data, int nbfiles, const char* const* urls,
                         time_t pintime, time_t timeout,
                         char* token, size_t tsize,
                         int async,
                         GError** err);

     /**
      *
      * OPTIONAL: Requests to stage a file to the fist layer on a hierarchical SE.
      * @param plugin_data : internal plugin context
      * @param nbfiles : number of files
      * @param urls : The urls of the files
      * @param metadata : Staging metadata array
      * @param pintime : Time the file should stay in the cache
      * @param timeout : Operation timeout
      * @param token Where to put the retrieved token.
      * @param tsize The size of the buffer pointed by token.
      * @param async If true (!= 0), the call will not block. The caller will need
      *              to use bring_online_poll later.
      * @param err:  GError error support
      * @return      -1 on error (and err is set). 0 on success. 1 if the file has been staged.
      */
     int (*bring_online_list_v2)(plugin_handle plugin_data, int nbfiles,
                         const char* const* urls, const char* const* metadata,
                         time_t pintime, time_t timeout,
                         char* token, size_t tsize,
                         int async,
                         GError** err);

     /**
      * OPTIONAL: Polling the bring_online request (mandatory if bring online is supported)
      * @param nbfiles : number of files
      * @param urls   The same URLs as were passed to bring_online_async
      * @param token The token as returned by bring_online_async
      * @return      -1 on error (and err is set). 0 on success. 1 if the file has been staged.
      */
     int (*bring_online_poll_list)(plugin_handle plugin_data, int nbfiles, const char* const* urls,
                              const char* token, GError** err);

     /**
      * OPTIONAL: Releases a previously staged file (mandatory if bring online is supported)
      * @param plugin_data : internal plugin context
      * @param nbfiles : number of files in the list
      * @param url :  The urls of the files
      * @param token: The request token. If NULL,
      * @param err:   GError error support
      */
     int (*release_file_list)(plugin_handle plugin_data, int nbfiles, const char* const* urls,
                         const char* token,
                         GError** err);

     /**
      * OPTIONAL: Bulk deletion
      */
     int (*unlink_listG)(plugin_handle plugin_data, int nbfiles, const char* const* uris, GError ** errors);

     /**
      * OPTIONAL: allows clients to abort selective file requests from the asynchronous requests of any type
      * @param plugin_data : internal plugin context
      * @param nbfiles : number of files in the list
      * @param url :  The urls of the files
      * @param token: The request token
      * @param err:   GError error support
      */
     int(*abort_files)(plugin_handle handle, int nbfiles, const char* const* uris, const char* token, GError ** err);

     /**
      * OPTIONAL: copy nbfiles files
      * @param plugin_data : internal plugin context
      * @param context : gfal2 context
      * @param params: gfal2 transfer parameters
      * @param nbfiles: how many files are to be transferred
      * @param srcs: array of nbfiles sources
      * @param dsts: array of nbfiles destinations
      * @param checkums: array of nbfiles checksums. it can be NULL
      * @param op_error:   Operation error
      * @param file_errors: Per file error
      */
     int (*copy_bulk)(plugin_handle plugin_data, gfal2_context_t context, gfalt_params_t params,
            size_t nbfiles, const char* const* srcs, const char* const* dsts, const char* const* checksums,
            GError** op_error, GError*** file_errors);

     /**
      * OPTIONAL: executed by the core before entering a copy, so a plugin can install its own
      *           event listeners.
      */
     int (*copy_enter_hook)(plugin_handle plugin_data, gfal2_context_t context, gfalt_params_t params, GError** error);

    // QoS API

  /**
   *  OPTIONAL: Check what kind of QoS classes exist on the CDMI-enabled URL provided
   *
   *  @param plugin_data : internal plugin data
   *  @param url : CDMI-enabled URL to check for the protocol compatibility
   *  @param type : CDMI-enabld type of QoS class
   *  @param buff : buffer for the QoS classes content
   *  @param s_buff : maximum buffer size
   *  @param err : error handle, should be used ONLY in case of major failure.
   *  @return number of bytes in buff in case of success or -1 if error occurs
   *
   *  @note Result is provided as a comma-separated list of available QoS classes
   */
  ssize_t (*check_qos_classes)(plugin_handle plugin_data, const char* url, const char* type,
                               char* buff, size_t s_buff, GError** err);

  /**
   *  OPTIONAL: Check the QoS of a file with the CDMI-enabled url provided
   *
   *  @param plugin_data : internal plugin data
   *  @param url : CDMI-enabled URL to check for the protocol compatibility
   *  @param buff : buffer for the QoS class content
   *  @param s_buff : maximum buffer size
   *  @param err : error handle, should be used ONLY in case of major failure.
   *  @return number of bytes in buff in case of success or -1 if error occurs
   */
  ssize_t (*check_file_qos)(plugin_handle plugin_data, const char* url, char* buff, size_t s_buff, GError** err);

  /**
   *  OPTIONAL: Check the available transitions of a QoS class to other QoS classes
   *
   *  @param plugin_data : internal plugin data
   *  @param qos_class_url : CDMI-enabled URL of QoS class
   *  @param buff : buffer for the QoS class transitions content
   *  @param s_buff : maximum buffer size
   *  @param err : error handle, should be used ONLY in case of major failure.
   *  @return number of bytes in buff in case of success or -1 if error occurs
   *
   *  @note Result is provided as comma-separated list of available QoS class transitions
   */
  ssize_t (*check_qos_available_transitions)(plugin_handle plugin_data, const char* qos_class_url,
                                             char* buff, size_t s_buff, GError** err);

  /**
   *  OPTIONAL: Check the target QoS of an entity
   *
   *  @param plugin_data : internal plugin data
   *  @param url : CDMI-enabled URL of an entity
   *  @param buff : buffer for the target QoS class content
   *  @param s_buff : maximum buffer size
   *  @param err : error handle, should be used ONLY in case of major failure.
   *  @return number of bytes in buff in case of success or -1 if error occurs
   */
  ssize_t (*check_target_qos)(plugin_handle plugin_data, const char* url, char* buff, size_t s_buff, GError** err);

  /**
   *  OPTIONAL: Check the QoS of a file with the CDMI-enabled url provided
   *
   *  @param plugin_data : internal plugin data
   *  @param url : CDMI-enabled URL of an entity
   *  @param target_qos: the requested target QoS class
   *  @param err : error handle, should be used ONLY in case of major failure.
   *  @return 0 or -1 if error occurs
   */
  int (*change_object_qos)(plugin_handle plugin_data, const char* url, const char* target_qos, GError** err);

    // ARCHIVE API

  /**
   * OPTIONAL: Polling the archive request (mandatory if archiving is supported)
   *
   * @param plugin_data: internal plugin data
   * @param url: the URL for which to check archive status
   * @param err: error handle
   * @return -1 on error (and err is set). 0 on success. 1 if the file has been archived.
   */
  int (*archive_poll)(plugin_handle plugin_data, const char* url, GError** err);

  /**
   * OPTIONAL: Polling the archive request (mandatory if archiving is supported)
   *
   * @param plugin_data: internal plugin data
   * @param nbfiles: number of files
   * @param urls: the URLs for which to check archive status
   * @param err: error handle
   * @return -1 on error (and err is set). 0 on success. 1 if the files have been archived.
   *          2 if some files have bene archived, others encountered errors
   */
  int (*archive_poll_list)(plugin_handle plugin_data, int nbfiles,
                           const char* const* urls, GError** err);

    // TOKEN API

  /**
   * OPTIONAL: Retrieve a token from Storage Element for a given resource
   *
   * @param plugin_data: internal plugin data
   * @param url: the URL of the resource
   * @param issuer: the token issuer endpoint (optional)
   * @param write_access: write access flag
   * @param validity: token validity time in minutes
   * @param activities: array of activities for access request
   * @param buff : buffer for the token content
   * @param s_buff : maximum buffer size
   * @param err : error handle
   * @return number of bytes in buff in case of success or -1 if error occurs
   */
  ssize_t (*token_retrieve)(plugin_handle plugin_data, const char* url, const char* issuer,
                            gboolean write_access, unsigned validity, const char* const* activities,
                            char* buff, size_t s_buff, GError** err);

      // reserved for future usage
	 //! @cond
     void* future[4];
	 //! @endcond
};

/**
 * Trigger plugin instantiation from the client code
 * The passed interface is copied, so you can, if needed, free it after the call
 */
int gfal2_register_plugin(gfal2_context_t handle, const gfal_plugin_interface* ifce,
        GError** error);


// internal API for inter plugin communication
//! @cond
int gfal_plugins_accessG(gfal2_context_t handle, const char* path, int mode, GError** err);
int gfal_plugin_rmdirG(gfal2_context_t handle, const char* path, GError** err);
ssize_t gfal_plugin_readlinkG(gfal2_context_t handle, const char* path, char* buff, size_t buffsiz, GError** err);


int gfal_plugin_chmodG(gfal2_context_t handle, const char* path, mode_t mode, GError** err);
int gfal_plugin_statG(gfal2_context_t handle,const char* path, struct stat* st, GError** err);
int gfal_plugin_renameG(gfal2_context_t handle, const char* oldpath, const char* newpath, GError** err);
int gfal_plugin_symlinkG(gfal2_context_t handle, const char* oldpath, const char* newpath, GError** err);
int gfal_plugin_lstatG(gfal2_context_t handle,const char* path, struct stat* st, GError** err);
int gfal_plugin_mkdirp(gfal2_context_t handle, const char* path, mode_t mode, gboolean pflag,  GError** err);


gfal_file_handle gfal_plugin_opendirG(gfal2_context_t handle, const char* name, GError** err);
struct dirent* gfal_plugin_readdirppG(gfal2_context_t handle, gfal_file_handle fh, struct stat* st, GError** err);
int gfal_plugin_closedirG(gfal2_context_t handle, gfal_file_handle fh, GError** err);
struct dirent* gfal_plugin_readdirG(gfal2_context_t handle, gfal_file_handle fh, GError** err);


gfal_file_handle gfal_plugin_openG(gfal2_context_t handle, const char * path, int flag, mode_t mode, GError ** err);
int gfal_plugin_closeG(gfal2_context_t handle, gfal_file_handle fh, GError** err);
int gfal_plugin_writeG(gfal2_context_t handle, gfal_file_handle fh, void* buff, size_t s_buff, GError** err);
int gfal_plugin_lseekG(gfal2_context_t handle, gfal_file_handle fh, off_t offset, int whence, GError** err);
int gfal_plugin_readG(gfal2_context_t handle, gfal_file_handle fh, void* buff, size_t s_buff, GError** err);

ssize_t gfal_plugin_preadG(gfal2_context_t handle, gfal_file_handle fh, void* buff, size_t s_buff, off_t offset, GError** err);
ssize_t gfal_plugin_pwriteG(gfal2_context_t handle, gfal_file_handle fh, void* buff, size_t s_buff, off_t offset, GError** err);


int gfal_plugin_unlinkG(gfal2_context_t handle, const char* path, GError** err);


ssize_t gfal_plugin_getxattrG(gfal2_context_t, const char*, const char*, void* buff, size_t s_buff, GError** err);
ssize_t gfal_plugin_listxattrG(gfal2_context_t, const char*, char* list, size_t s_list, GError** err);
int gfal_plugin_setxattrG(gfal2_context_t, const char*, const char*, const void*, size_t, int, GError**);

int gfal_plugin_bring_onlineG(gfal2_context_t handle, const char* uri,
                              time_t pintime, time_t timeout,
                              char* token, size_t tsize,
                              int async,
                              GError ** err);

int gfal_plugin_bring_online_v2G(gfal2_context_t handle,
                                      const char* uri, const char* metadata,
                                      time_t pintime, time_t timeout,
                                      char* token, size_t tsize,
                                      int async,
                                      GError ** err);

int gfal_plugin_bring_online_pollG(gfal2_context_t handle, const char* uri,
                                   const char* token, GError ** err);

int gfal_plugin_release_fileG(gfal2_context_t handle, const char* uri,
                              const char* token, GError ** err);

int gfal_plugin_bring_online_listG(gfal2_context_t handle, int nbfiles, const char* const* uris,
                              time_t pintime, time_t timeout,
                              char* token, size_t tsize,
                              int async,
                              GError ** err);

int gfal_plugin_bring_online_list_v2G(gfal2_context_t handle, int nbfiles,
                                      const char* const* uris, const char* const* metadata,
                                      time_t pintime, time_t timeout,
                                      char* token, size_t tsize,
                                      int async,
                                      GError ** err);

int gfal_plugin_bring_online_poll_listG(gfal2_context_t handle, int nbfiles, const char* const* uris,
                                   const char* token, GError ** err);

int gfal_plugin_release_file_listG(gfal2_context_t handle, int nbfiles, const char* const* uris,
                              const char* token, GError ** err);

int gfal_plugin_unlink_listG(gfal2_context_t handle, int nbfiles, const char* const* uris, GError ** errors);

int gfal_plugin_abort_filesG(gfal2_context_t handle, int nbfiles, const char* const* uris, const char* token, GError ** err);

ssize_t gfal_plugin_qos_check_classes(gfal2_context_t handle, const char* url, const char* type,
                                      char* buff, size_t s_buff, GError** err);
ssize_t gfal_plugin_check_file_qos(gfal2_context_t handle, const char* url, char* buff, size_t s_buff, GError** err);
ssize_t gfal_plugin_check_qos_available_transitions(gfal2_context_t handle, const char* qos_class_url,
                                                    char* buff, size_t s_buff, GError** err);
ssize_t gfal_plugin_check_target_qos(gfal2_context_t handle, const char* url, char* buff, size_t s_buff, GError** err);
int gfal_plugin_change_object_qos(gfal2_context_t handle, const char* url, const char* target_qos, GError** err);

int gfal_plugin_archive_pollG(gfal2_context_t handle, const char* uri, GError ** err);
int gfal_plugin_archive_poll_listG(gfal2_context_t handle, int nbfiles, const char* const* uris,
                                   GError ** err);

ssize_t gfal_plugin_token_retrieveG(gfal2_context_t handle, const char* url, const char* issuer,
                                    gboolean write_access, unsigned validity, const char* const* activities,
                                    char* buff, size_t s_buff, GError** err);

//! @endcond

#ifdef __cplusplus
}
#endif


#endif /* GFAL_COMMON_PLUGIN_INTERFACE_H_ */
