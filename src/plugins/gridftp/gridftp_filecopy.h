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
#ifndef GRIFTP_IFCE_FILECOPY_H
#define GRIFTP_IFCE_FILECOPY_H

#include "gridftpmodule.h"
#include "gridftpwrapper.h"

extern const GQuark GFAL_GRIDFTP_DOMAIN_GSIFTP;


extern "C" int gridftp_plugin_filecopy(plugin_handle handle,
        gfal2_context_t context, gfalt_params_t params, const char* src,
        const char* dst, GError ** err);

extern "C" int gridftp_bulk_copy(plugin_handle plugin_data, gfal2_context_t context, gfalt_params_t params,
        size_t nbfiles, const char* const * srcs, const char* const * dsts,
        const char* const * checksums, GError** op_error, GError*** file_errors);

int gridftp_filecopy_delete_existing(GridFTPModule* module,
        gfalt_params_t params, const char * url);

void gridftp_create_parent_copy(GridFTPModule* module, gfalt_params_t params,
        const char * gridftp_url);

/**
 * Return the ip of the given host
 * @param host          Hostname
 * @param ipv6_enabled  If true, this function will look for an ipv6 preferably
 * @param got_ipv6      Will be set to true if ipv6_enabled is true, and the function finds an ipv6 for the host.
 *                      If NULL, it will be ignored.
 * @return An IP associated with host.
 */
std::string lookup_host(const char *host, bool ipv6_enabled, bool *got_ipv6);

std::string return_host_and_port(const std::string &uri, gboolean use_ipv6);

#endif /* GRIFTP_IFCE_FILECOPY_H */
