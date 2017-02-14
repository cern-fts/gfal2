/*
 * Copyright (c) CERN 2013-2015
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
#ifndef GFAL_PROTOTYPES_H_
#define GFAL_PROTOTYPES_H_

#if !defined(__GFAL2_H_INSIDE__) && !defined(__GFAL2_BUILD__)
#   warning "Direct inclusion of gfal2 headers is deprecated. Please, include only gfal_api.h or gfal_plugins_api.h"
#endif

#include <glib.h>
#include "gfal_deprecated.h"


#ifdef __cplusplus
extern "C"
{
#endif

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
    GFAL_PLUGIN_BRING_ONLINE
} plugin_mode;

typedef enum _gfal_url2_check {
	GFAL_FILE_COPY,
	GFAL_BULK_COPY
} gfal_url2_check;


 /* plugin */
typedef struct _plugin_opts plugin_opts;
typedef struct _gfal_plugin_interface gfal_plugin_interface;
typedef gpointer plugin_handle;


/* file descriptor */
typedef struct _gfal_file_descriptor_container *gfal_fdesc_container_handle;

/* dir part file descriptor*/
typedef struct _gfal_file_handle_* gfal_file_handle;

typedef struct _gfal_descriptors_container gfal_descriptors_container;
typedef struct _gfal_conf_container gfal_conf_container;

#ifdef __cplusplus
}
#endif

#endif /* GFAL_PROTOTYPES_H_ */
