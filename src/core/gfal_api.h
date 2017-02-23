/*
 * Copyright (c) CERN 2013-2017
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
#ifndef GFAL2_API_H_
#define GFAL2_API_H_

#define __GFAL2_H_INSIDE__

/* gfal2 uses 64 bits offset size  by default */
#ifndef _FILE_OFFSET_BITS
#  define _FILE_OFFSET_BITS 64
#endif

/* global context operations */
#include <common/gfal_common.h>
#include <common/gfal_cred_mapping.h>

/* parameter and configuration API */
#include <common/gfal_config.h>

/* log  API */
#include <logger/gfal_logger.h>

/* main gfal2 API for file operations */
#include <file/gfal_file_api.h>

/* operation control API */
#include <common/gfal_cancel.h>

/* posix compatibility layer */
#include <posix/gfal_posix_api.h>

/* transfers*/
#include <transfer/gfal_transfer.h>

/* error helpers*/
#include <common/gfal_error.h>

#undef __GFAL2_H_INSIDE__

#endif  /* GFAL2_API_H_ */
