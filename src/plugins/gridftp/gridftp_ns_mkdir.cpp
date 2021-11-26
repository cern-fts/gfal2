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

#include "gridftp_namespace.h"
#include <exceptions/cpp_to_gerror.hpp>


static const GQuark GFAL_GRIDFTP_SCOPE_MKDIR = g_quark_from_static_string("GridFTPModule::mkdir");
static const GQuark GFAL_GRIDFTP_SCOPE_REQ_STATE = g_quark_from_static_string("GridFTPModule::RequestState");


void GridFTPModule::mkdir(const char* path, mode_t mode)
{
    if (path == NULL) {
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_MKDIR, EINVAL,
                "Invalid arguments path or mode");
    }
    gfal2_log(G_LOG_LEVEL_DEBUG, " -> [GridFTPModule::mkdir] ");

    GridFTPSessionHandler handler(_handle_factory, path);
    GridFTPRequestState req(&handler);

    globus_result_t res = globus_ftp_client_mkdir(req.handler->get_ftp_client_handle(),
            path, req.handler->get_ftp_client_operationattr(), globus_ftp_client_done_callback,
            &req);
    gfal_globus_check_result(GFAL_GRIDFTP_SCOPE_MKDIR, res);
    // wait for answer
    req.wait(GFAL_GRIDFTP_SCOPE_MKDIR);

    gfal2_log(G_LOG_LEVEL_DEBUG, " <- [GridFTPModule::mkdir] ");

}


extern "C" int gfal_gridftp_mkdirG(plugin_handle handle, const char* path,
        mode_t mode, gboolean pflag, GError** err)
{
    g_return_val_err_if_fail(handle != NULL && path != NULL, -1, err,
            "[gfal_gridftp_mkdirG][gridftp] Invalid parameters");

    GError * tmp_err = NULL;
    int ret = -1;
    gfal2_log(G_LOG_LEVEL_DEBUG, "  -> [gfal_gridftp_mkdirG]");
    CPP_GERROR_TRY
            (static_cast<GridFTPModule*>(handle))->mkdir(path, mode);
            ret = 0;
    CPP_GERROR_CATCH(&tmp_err);
    gfal2_log(G_LOG_LEVEL_DEBUG, "  [gfal_gridftp_mkdirG]<-");

    // Distinguish between actual ENOENT or certificate loading error
    if (pflag && tmp_err && tmp_err->code == ENOENT && tmp_err->domain == GFAL_GRIDFTP_SCOPE_REQ_STATE) {
        // ENOENT error will be masked by parent flag
        tmp_err->code = EFAULT;
    }

    G_RETURN_ERR(ret, tmp_err, err);
}
