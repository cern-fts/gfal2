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
#ifndef GRIDFTP_PLUGIN_H
#define GRIDFTP_PLUGIN_H

#include <gfal_plugins_api.h>

// Configuration options
#define GRIDFTP_CONFIG_GROUP          "GRIDFTP PLUGIN"
#define GRIDFTP_CONFIG_IPV6           "IPV6"
#define GRIDFTP_CONFIG_V2             "GRIDFTP_V2"
#define GRIDFTP_CONFIG_SESSION_REUSE  "SESSION_REUSE"
#define GRIDFTP_CONFIG_OP_TIMEOUT     "OPERATION_TIMEOUT"
#define GRIDFTP_CONFIG_DCAU           "DCAU"
#define GRIDFTP_CONFIG_DELAY_PASSV    "DELAY_PASSV"
#define GRIDFTP_CONFIG_ENABLE_PASV_PLUGIN "ENABLE_PASV_PLUGIN"

#define GRIDFTP_CONFIG_TRANSFER_CHECKSUM       "COPY_CHECKSUM_TYPE"
#define GRIDFTP_CONFIG_TRANSFER_PERF_TIMEOUT   "PERF_MARKER_TIMEOUT"
#define GRIDFTP_CONFIG_TRANSFER_SKIP_CHECKSUM  "SKIP_SOURCE_CHECKSUM"
#define GRIDFTP_CONFIG_TRANSFER_UDT            "ENABLE_UDT"


#ifdef __cplusplus
extern "C" {
#endif

const char *gridftp_plugin_name();

plugin_handle gridftp_plugin_load(gfal2_context_t handle, GError ** err);

void gridftp_plugin_unload(plugin_handle handle);

gboolean gridftp_check_url_transfer(plugin_handle handle,
        gfal2_context_t context, const char* src, const char* dst,
        gfal_url2_check type);

#ifdef __cplusplus
}
#endif

#endif /* GRIDFTP_PLUGIN_H */
