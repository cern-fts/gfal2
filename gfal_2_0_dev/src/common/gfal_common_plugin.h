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

/**
 * @file gfal_common_plugin.h
 * @brief the header file of the common lib for the plugin management
 * @author Devresse Adrien
 * @version 0.0.1
 * @date 8/04/2011
 * */
 
 
 
#include <stdarg.h>
#include <glib.h>
#include <errno.h>
#include <string.h> 
#include <dirent.h>
#include <sys/stat.h>
 // protos
#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_common_plugin_interface.h>

#include <common/gfal_common_parameter.h>

 
#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

typedef struct _plugin_pointer_handle{
	void* dlhandle;
	void* plugin_data;
	char plugin_name[GFAL_URL_MAX_LEN];
	char plugin_lib[GFAL_URL_MAX_LEN];
} *plugin_pointer_handle;

gfal_plugin_interface* gfal_plugin_interface_new();


extern char* gfal_get_cat_type(GError**);

/**
 * @brief plugins walker
 * provide a list of plugin handlers, each plugin handler is a reference to an usable plugin
 * @return give a NULL-ended table of plugin_pointer_handle, return NULL if error
 * */
plugin_pointer_handle gfal_plugins_list_handler(gfal_handle, GError** err);

inline int gfal_plugins_instance(gfal_handle, GError** err);
char** gfal_plugins_get_list(gfal_handle, GError** err);
int gfal_plugins_delete(gfal_handle, GError** err);




int gfal_plugins_accessG(gfal_handle handle, const char* path, int mode, GError** err);
int gfal_plugin_rmdirG(gfal_handle handle, const char* path, GError** err);
ssize_t gfal_plugin_readlinkG(gfal_handle handle, const char* path, char* buff, size_t buffsiz, GError** err);




int gfal_plugin_chmodG(gfal_handle handle, const char* path, mode_t mode, GError** err);
int gfal_plugin_statG(gfal_handle handle,const char* path, struct stat* st, GError** err);
int gfal_plugin_renameG(gfal_handle handle, const char* oldpath, const char* newpath, GError** err);
int gfal_plugin_symlinkG(gfal_handle handle, const char* oldpath, const char* newpath, GError** err);
int gfal_plugin_lstatG(gfal_handle handle,const char* path, struct stat* st, GError** err);
int gfal_plugin_mkdirp(gfal_handle handle, const char* path, mode_t mode, gboolean pflag,  GError** err);


gfal_file_handle gfal_plugin_opendirG(gfal_handle handle, const char* name, GError** err);
int gfal_plugin_closedirG(gfal_handle handle, gfal_file_handle fh, GError** err);
struct dirent* gfal_plugin_readdirG(gfal_handle handle, gfal_file_handle fh, GError** err);
 	

gfal_file_handle gfal_plugin_openG(gfal_handle handle, const char * path, int flag, mode_t mode, GError ** err);
int gfal_plugin_closeG(gfal_handle handle, gfal_file_handle fh, GError** err);
int gfal_plugin_writeG(gfal_handle handle, gfal_file_handle fh, void* buff, size_t s_buff, GError** err);
int gfal_plugin_lseekG(gfal_handle handle, gfal_file_handle fh, off_t offset, int whence, GError** err);
int gfal_plugin_readG(gfal_handle handle, gfal_file_handle fh, void* buff, size_t s_buff, GError** err);

ssize_t gfal_plugin_preadG(gfal_handle handle, gfal_file_handle fh, void* buff, size_t s_buff, off_t offset, GError** err);


int gfal_plugin_unlinkG(gfal_handle handle, const char* path, GError** err);


ssize_t gfal_plugin_getxattrG(gfal_handle, const char*, const char*, void* buff, size_t s_buff, GError** err);
ssize_t gfal_plugin_listxattrG(gfal_handle, const char*, char* list, size_t s_list, GError** err);
int gfal_plugin_setxattrG(gfal_handle, const char*, const char*, const void*, size_t, int, GError**);






int gfal_plugins_has_parameter(gfal_handle handle, const char* nmespace, const char* key, GError** err);

int gfal_plugins_notify_all(gfal_handle handle, const char* nmespace, const char* key, GError** err);

#ifdef __cplusplus
}
#endif // __cplusplus
