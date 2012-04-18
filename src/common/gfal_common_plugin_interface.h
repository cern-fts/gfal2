#pragma once
#ifndef _GFAL_PLUGIN_INTERFACE_
#define _GFAL_PLUGIN_INTERFACE_
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
 * @file header file for the plugins implementation
 * @author Devresse Adrien
 * @version 1.0
 * @date 02/10/2011
 * 
 * */

#include <glib.h>
#include <common/gfal_types.h>
#include <common/gfal_constants.h>

#ifdef __cplusplus
extern "C"
{
#endif 



/**
 * @struct _gfal_plugin_interface 
 * 
 *  main Interface that MUST be returned the entry point function "gfal_plugin_init" of each GFAL 2.0 plugin.
 *  the minimum calls are : getName, plugin_delete, check_plugin_url
 *  all the unused function pointers must be set to NULL
 */
struct _gfal_plugin_interface{
	// internal datas*
	void * gfal_data;
	
	// handle
	plugin_handle handle;
	// to have name/id of the plugin MUST be IMPLEMENTED
	const char* (*getName)();
	/**
	 * Called before the destruction of the plugin interface
	 */
	void (*plugin_delete)(plugin_handle); 
	/**
	 *  Check the url syntax and the operation, return TRUE if the operation is compatible with the plugin, else FALSE.
	 *  THIS function MUST be implemented in EACH plugin.
	 *  @warning This function is a key function of GFAL 2.0, It MUST be as fast as possible.
	 */
	gboolean (*check_plugin_url)(plugin_handle, const char* url,  plugin_mode mode, GError** err);
	 
	/**
	 *  access function for the access for the normal associated url
	 * */
	int (*accessG)(plugin_handle, const char* path, int mode, GError** err);
	/**
	 *  function pointer for the access using the guid url
	 * */
	int	(*chmodG)(plugin_handle, const char *, mode_t, GError** err);
	int	(*renameG)(plugin_handle, const char *, const char *, GError** err);
	int (*symlinkG)(plugin_handle, const char*, const char*, GError** err);
	int (*statG)(plugin_handle, const char*, struct stat *buf, GError** err);
	int (*lstatG)(plugin_handle, const char*, struct stat *buf, GError** err);
	ssize_t (*readlinkG)(plugin_handle, const char*, char* buff, size_t buffsiz, GError** );

	// directory management
	 gfal_file_handle (*opendirG)(plugin_handle, const char*, GError**); 
	 int (*closedirG)(plugin_handle, gfal_file_handle, GError**);
	 struct dirent* (*readdirG)(plugin_handle, gfal_file_handle, GError**);
	int (*mkdirpG)(plugin_handle, const char*, mode_t, gboolean pflag, GError**);  //function pointer for the mkdir call, if pflag is set, the call is considered like a recursive call for a full path creation
	int (*rmdirG )(plugin_handle, const char*, GError**);	 	 

	 // basic file operation, 
	 gfal_file_handle (*openG)(plugin_handle, const char* path, int flag, mode_t mode, GError**);
	 int (*readG)(plugin_handle, gfal_file_handle fd, void* buff, size_t count, GError**);
	 int (*writeG)(plugin_handle, gfal_file_handle fd, void* buff, size_t count, GError**);
	 int (*closeG)(plugin_handle, gfal_file_handle fd, GError **);
	 int (*lseekG)(plugin_handle, gfal_file_handle fd, off_t offset, int whence, GError** err);
	 
	 // vector operations
	 /**
	  *  Vector operation, similar to the POSIX call pread
	  *  The vector operation needs to be set to NULL if not implemented
	  *  Vector operation permits an improvement of the performances.  
	  * * */
	 ssize_t (*preadG)(plugin_handle, gfal_file_handle fd, void* buff, size_t count, off_t offset, GError**);	 
	 ssize_t (*pwriteG)(plugin_handle, gfal_file_handle fd, const void* buff, size_t count, off_t offset, GError**);
	 
	 	 
	 // remove operations
	/**
	  * unlink implementation, follow the \ref unlink behavior but with GError report system.
	  * */	
	 int (*unlinkG)(plugin_handle, const char* path, GError**);
	 
	 // advanced attributes management
	 /**
	  * getxattr implementation, follow the \ref getxattr behavior but with GError report system.
	  * */	
	 ssize_t (*getxattrG)(plugin_handle, const char*, const char*, void* buff, size_t s_buff, GError** err);
	 /**
	  * listxattr implementation, follow the \ref listxattr behavior but with GError report system.
	  * */
	 ssize_t (*listxattrG)(plugin_handle, const char*, char* list, size_t s_list, GError** err);
	 /**
	  * setxattr implementation, follow the \ref setxattr behavior but with GError report system.
	  * */
	 int (*setxattrG)(plugin_handle, const char*, const char*, const void*, size_t , int, GError** );


	 /**
	  *  this function is called to inform plugin of a new parameter
	  *  must return 1 if this parameter is used by the plugin else return 0
	  *  @warning must be a NULL pointer if not used
	  * 
	  * */
	 int (*is_used_parameter)(plugin_handle, const char* namespacep, const char* key);

	 /**
	  *  called when a change occures on a parameter
	  *  push the information to the plugins
	  *  to check the new value associated with the key, call gfal_get_*_parameter functions
	  *  @warning must be a NULL pointer if not used
	  *  @return 0 if parameter changed or if no parameter triggered, -1 if major error with the new parameter
	  **/
	 void (*notify_change_parameter)(plugin_handle, const char* namespacep, const char* key);
};

/**
 * container for the plugin abstraction couch parameters
 * */
struct _plugin_opts{
	gfal_plugin_interface plugin_list[MAX_PLUGIN_LIST];
	int plugin_number;
};


#ifdef __cplusplus
}
#endif 


#endif


