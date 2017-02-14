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

#ifndef GFAL_FILE_HANDLE_H
#define GFAL_FILE_HANDLE_H
#pragma once


#if !defined(__GFAL2_H_INSIDE__) && !defined(__GFAL2_BUILD__)
#   warning "Direct inclusion of gfal2 headers is deprecated. Please, include only gfal_api.h or gfal_plugins_api.h"
#endif

#include <glib.h>
#include <stdlib.h>
#include <common/gfal_prototypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
* @brief create a gfal file handle
* @param module_name : module name must be the plugin_name of the plugin creating the gfal_file_handle, \ref _gfal_plugin_interface
* @param gpointer : internal file descriptor of the plugin to store*
*/
gfal_file_handle gfal_file_handle_new(const char* module_name, gpointer fdesc);
/**
* same than \ref gfal_file_handle_new but allows to store user data in the gfal file descriptor
* allow the usage of the readdirpp operation in case of directory file handle
*/
gfal_file_handle gfal_file_handle_new2(const char *module_name, gpointer fdesc, gpointer user_data, const char *file_path);

/**
 * return the file descriptor of this gfal file handle
 */
gpointer gfal_file_handle_get_fdesc(gfal_file_handle fh);

/**
 * set the file descriptor
 */
void gfal_file_handle_set_fdesc(gfal_file_handle fh, gpointer fdesc);

/**
* return the user data of this gfal file descriptor
*/
gpointer gfal_file_handle_get_user_data(gfal_file_handle fh);

/**
 * return the file path associated to the file handle
 */
const gchar* gfal_file_handle_get_path(gfal_file_handle fh);

/**
* delete an existing gfal file descriptor
* a file descriptor must be deleted by the plugin in the "close" functions
*/
void gfal_file_handle_delete(gfal_file_handle fh);

#ifdef __cplusplus
}
#endif

#endif /* GFAL_FILE_HANDLE_H */
