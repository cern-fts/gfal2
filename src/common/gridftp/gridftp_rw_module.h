#pragma once
#ifndef GRIDFTP_RW_MODULE_H
#define GRIDFTP_RW_MODULE_H
/*
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

#include "gridftpmodule.h"


extern "C" gfal_file_handle gfal_gridftp_openG(plugin_handle ch, const char* url, int flag, mode_t mode, GError** err);


#endif /* GRIDFTP_RW_MODULE_H */ 
