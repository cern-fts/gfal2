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
#ifndef _GFAL_COMMON_INTERNAL_H_
#define _GFAL_COMMON_INTERNAL_H_

#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>

#ifdef __cplusplus
extern "C"
{
#endif


/// init function
gfal2_context_t gfal_initG(GError** err);

/// free gfal handle
void gfal_handle_freeG(gfal2_context_t handle);

GQuark gfal_cancel_quark();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _GFAL_COMMON_INTERNAL_H_
