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

/***
 * @brief  gfal_types.h
 * @author  Adrien Devresse
 * types declaration for gfal  
 * */
#ifndef _GFAL_TYPES_H
#define _GFAL_TYPES_H



#include <unistd.h>
#include <glib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <attr/xattr.h>

#include <common/gfal_prototypes.h>
#include <common/gfal_constants.h>
#include <common/gfal_common_plugin_interface.h>
#include <g_config_manager/g_config_manager.h>


/* enforce proper calling convention */
#ifdef __cplusplus
extern "C"
{
#endif


/**
 * @enum list the type of the check associated with the url
 *  check_plugin_url send this mode to the plugin to know is this type of operation on it
 * */






struct _gfal_descriptors_container{
	gfal_fdesc_container_handle dir_container;
	gfal_fdesc_container_handle file_container;
	
};

typedef struct _gfal_conf_elem{
	GKeyFile * key_file;
} *gfal_conf_elem_t;

typedef struct _gfal_conf{
	GMutex* mux;
    GKeyFile* running_config;
    GConfigManager_t manager;
} *gfal_conf_t;


 
struct gfal_handle_ {		// define the protocole version of SRM choosen by default
	gboolean initiated; 					// 1 if initiated, else error
	// struct of the plugin opts
	struct _plugin_opts plugin_opt;
	//struct for the file descriptors
	gfal_descriptors_container fdescs;
	int no_bdii_check;
	gfal_conf_t conf;
};





#ifdef __cplusplus
}
#endif

#endif
