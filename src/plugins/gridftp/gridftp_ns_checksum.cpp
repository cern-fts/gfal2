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

#include <exceptions/cpp_to_gerror.hpp>

#include "gridftp_namespace.h"
#include "gridftp_plugin.h"
#include <iostream>
#include <algorithm>


static const GQuark GFAL_GRIDFTP_SCOPE_CHECKSUM = g_quark_from_static_string("GridFTPModule::checksum");


const char * gridftp_checksum_calc_timeout= "CHECKSUM_CALC_TIMEOUT";


extern "C" int gfal_gridftp_checksumG(plugin_handle handle, const char* url,
        const char* check_type, char * checksum_buffer, size_t buffer_length,
        off_t start_offset, size_t data_length, GError ** err)
{
    g_return_val_err_if_fail(handle != NULL && url != NULL, -1, err,
            "[gfal_gridftp_checksumG][gridftp] Invalid parameeters");

    GError * tmp_err = NULL;
    int ret = -1;
    gfal2_log(G_LOG_LEVEL_DEBUG, "  -> [gfal_gridftp_checksumG]");
    CPP_GERROR_TRY
        (static_cast<GridFTPModule*>(handle))->checksum(url, check_type,
                checksum_buffer, buffer_length, start_offset,
                data_length);
        ret = 0;
    CPP_GERROR_CATCH(&tmp_err);
    gfal2_log(G_LOG_LEVEL_DEBUG, "  [gfal_gridftp_checksumG] <-");
    G_RETURN_ERR(ret, tmp_err, err);
}

bool string_is_valid(const std::string &str)
{
     for(int i=0; i < str.length(); i++){
        if(!isalnum(str[i])){
        return 0;
        }
        return 1;    
    }

}
void GridFTPModule::checksum(const char* url, const char* check_type,
        char * checksum_buffer, size_t buffer_length, off_t start_offset,
        size_t data_length)
{
    gfal2_log(G_LOG_LEVEL_DEBUG, " -> [GridFTPModule::checksum] ");
    gfal2_log(G_LOG_LEVEL_DEBUG, " Checksum calculation %s for url %s",
            check_type, url);

    GridFTPSessionHandler handler(_handle_factory, url);
    GridFTPRequestState req(&handler, GRIDFTP_REQUEST_FTP);

    if (buffer_length < 16) {
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_CHECKSUM, ENOBUFS,
                "buffer length for checksum calculation is not enough");
    }

    globus_result_t res = globus_ftp_client_cksm(req.handler->get_ftp_client_handle(),
            url, req.handler->get_ftp_client_operationattr(),
            checksum_buffer, start_offset,
            ((data_length) ? (data_length) : (-1)), check_type,
            globus_ftp_client_done_callback, &req);
    gfal_globus_check_result(GFAL_GRIDFTP_SCOPE_CHECKSUM, res);
    // wait for answer with a timeout
    const time_t global_timeout = gfal2_get_opt_integer_with_default(
        _handle_factory->get_gfal2_context(),
        CORE_CONFIG_GROUP, CORE_CONFIG_CHECKSUM_TIMEOUT, 1800
    );

    const time_t timeout = gfal2_get_opt_integer_with_default(
            _handle_factory->get_gfal2_context(),
            GRIDFTP_CONFIG_GROUP, gridftp_checksum_calc_timeout, global_timeout);


    req.wait(GFAL_GRIDFTP_SCOPE_CHECKSUM, timeout);

    if (string_is_valid(checksum_buffer) != 1) {
        std::string s(16, '0');
	strncpy(checksum_buffer, s.c_str(), buffer_length);
    } 
	
    gfal2_log(G_LOG_LEVEL_DEBUG, " <- [GridFTPModule::checksum] ");
}
