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
#ifndef GRIDFTP_RW_MODULE_H
#define GRIDFTP_RW_MODULE_H

#include "gridftpmodule.h"

extern "C" gfal_file_handle gfal_gridftp_openG(plugin_handle ch,
        const char* url, int flag, mode_t mode, GError** err);

extern "C" ssize_t gfal_gridftp_readG(plugin_handle ch, gfal_file_handle fd,
        void* buff, size_t s_buff, GError** err);

extern "C" ssize_t gfal_gridftp_writeG(plugin_handle ch, gfal_file_handle fd,
        const void* buff, size_t s_buff, GError** err);

extern "C" off_t gfal_gridftp_lseekG(plugin_handle ch, gfal_file_handle fd,
        off_t offset, int whence, GError** err);

extern "C" int gfal_gridftp_closeG(plugin_handle ch, gfal_file_handle fd,
        GError** err);


#endif /* GRIDFTP_RW_MODULE_H */ 
