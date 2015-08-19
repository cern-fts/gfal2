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
#ifndef GFAL_COMMON_FILE_HANDLE_H_
#define GFAL_COMMON_FILE_HANDLE_H_

#include "gfal_constants.h" 
#include "gfal_prototypes.h"

gfal_fdesc_container_handle gfal_file_handle_container_instance(gfal_descriptors_container* fdescs, GError** err);

void gfal_file_handle_container_delete(gfal_descriptors_container* fdescs);

#endif /* GFAL_COMMON_FILE_HANDLE_H_ */
