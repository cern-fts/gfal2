#pragma once
#ifndef GRIDFTP_EXIST_H
#define GRIDFTP_EXIST_H

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

bool gridftp_module_file_exist(gfal2_context_t context, GridFTP_session* sess, const char * url);




#endif /* GRIDFTP_EXIST_H */ 
