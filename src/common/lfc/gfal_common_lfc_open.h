#pragma once
/* 
* Copyright @ Members of the EMI Collaboration, 2010.
* See www.eu-emi.eu for details on the copyright holders.
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

#define _GNU_SOURCE
 
/*
  * 
   file gfal_common_lfc_open.c
   brief header for lfc implementation for open/read/write/close
   author Adrien Devresse
   date 06/07/2011
 */

#include <regex.h>
#include <glib.h>
#include "gfal_common_lfc.h"
#include "lfc_ifce_ng.h"



gfal_file_handle lfc_openG(plugin_handle ch, const char* path, int flag, mode_t mode, GError** err);

