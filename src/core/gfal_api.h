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
#pragma once
#ifndef _GFAL2_API_
#define _GFAL2_API_

#define __GFAL2_H_INSIDE__

// gfal2 uses 64 bits offset size  by default
#ifndef _FILE_OFFSET_BITS
#  define _FILE_OFFSET_BITS 64
#endif

// global context operations
#include <global/gfal_global.h>

// parameter and configuration api
#include <config/gfal_config.h>

// log  api
#include <logger/gfal_logger.h>

// main gfal2 API for file operations
#include <file/gfal_file_api.h>

// operation control api
#include <cancel/gfal_cancel.h>

// gfal 1.0 compatibility layer
#include <posix/gfal_posix_api.h>

// transfers
#include <transfer/gfal_transfer.h>

// error helpers
#include <common/gfal_common_err_helpers.h>

#undef __GFAL2_H_INSIDE__

#endif  //_GFAL2_API_
