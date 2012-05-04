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

/*
 * @file gfal_common_srm_getxattr.h
 * @brief header for the getxattr function on the srm url type
 * @author Devresse Adrien
 * @date 02/08/2011
 * */
 
#include <sys/types.h>
#include <attr/xattr.h> 
#include <stdio.h>

#include "gfal_common_srm.h"



ssize_t gfal_srm_getxattrG(plugin_handle handle, const char* path, const char* name , void* buff, size_t s_buff, GError** err);

ssize_t gfal_srm_status_getxattrG(plugin_handle handle, const char* path, const char* name , void* buff, size_t s_buff, GError** err);

ssize_t gfal_srm_listxattrG(plugin_handle handle, const char* path, char* list, size_t size, GError** err);
