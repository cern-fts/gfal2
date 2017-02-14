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
#ifndef GFAL_COMMON_FILEDESCRIPTOR_H_
#define GFAL_COMMON_FILEDESCRIPTOR_H_

#include <glib.h>
#include <stdlib.h>
#include <pthread.h>
#include <common/gfal_prototypes.h>
#include <common/gfal_constants.h>
#include <common/gfal_file_handle.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct _gfal_file_handle_container {
	GHashTable* container;
	pthread_mutex_t m_container;
};

struct _gfal_file_handle {
	char module_name[GFAL_MODULE_NAME_SIZE]; // This MUST be the Name of the plugin associated with this handle!
	GMutex* lock;
	off_t offset;
	gpointer ext_data;
	gpointer fdesc;
    gchar* path;
};

 // low level funcs
gfal_file_handle_container gfal_file_descriptor_handle_create(GDestroyNotify destroyer);

void gfal_file_descriptor_handle_destroy(gfal_file_handle_container fhandle);

int gfal_add_new_file_desc(gfal_file_handle_container fhandle, gpointer pfile, GError** err);

gboolean gfal_remove_file_desc(gfal_file_handle_container fhandle, int key, GError** err);


gpointer gfal_get_file_desc(gfal_file_handle_container fhandle, int key, GError** err);

gfal_file_handle gfal_file_handle_bind(gfal_file_handle_container h, int file_desc, GError** err);

#ifdef __cplusplus
}
#endif

#endif /* GFAL_COMMON_FILEDESCRIPTOR_H_ */
