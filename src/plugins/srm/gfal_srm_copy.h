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
#ifndef _GFAL2_COMMON_SRM_COPY_
#define _GFAL2_COMMON_SRM_COPY_


/**
 * initialize a file copy from the given source to the given dest with the parameters params
 *  srm implementation of the plugin filecopy
 *  @param handle : srm plugin handle
 *  @param params :tranfer parameters
 *  @param status : transfer status handle
 *  @param src : source url
 *  @param dst : destination url
 *  @param err : error report system
 */
int srm_plugin_filecopy(plugin_handle handle, gfal2_context_t context,
                    gfalt_params_t params,
                    const char* src, const char* dst, GError ** err);

#endif
