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
 * @file gfal_common_filedescriptor.h
 * @brief  header file for the file descriptor management
 * @author Devresse Adrien
 * @version 2.0
 * @date 22/05/2011
 * */
 
#include <glib.h>
#include <stdlib.h>
#include <pthread.h>
#include "gfal_prototypes.h"
#include "gfal_constants.h"

struct _gfal_file_descriptor_container{
	GHashTable* container;
	pthread_mutex_t m_container;	
};

struct _gfal_file_handle_{
	char module_name[GFAL_MODULE_NAME_SIZE]; // This MUST be the Name of the plugin associated with this handle !
	GMutex* lock;
	off_t offset;
	gpointer ext_data;
	gpointer fdesc;	
};

 // low level funcs
gfal_fdesc_container_handle gfal_file_descriptor_handle_create(GDestroyNotify destroyer);

int gfal_add_new_file_desc(gfal_fdesc_container_handle fhandle, gpointer pfile, GError** err);

gboolean gfal_remove_file_desc(gfal_fdesc_container_handle fhandle, int key, GError** err);


gpointer gfal_get_file_desc(gfal_fdesc_container_handle fhandle, int key, GError** err);


// high level funcs

gfal_file_handle gfal_file_handle_bind(gfal_fdesc_container_handle h, int file_desc, GError** err);

// convenience funcs

gfal_file_handle gfal_file_handle_new(const char* module_name, gpointer fdesc);

gfal_file_handle gfal_file_handle_ext_new(const char* module_name, gpointer fdesc, gpointer ext_data);


void gfal_file_handle_delete(gfal_file_handle fh);

void gfal_file_handle_lock(gfal_file_handle fh);

void gfal_file_handle_unlock(gfal_file_handle fh);
