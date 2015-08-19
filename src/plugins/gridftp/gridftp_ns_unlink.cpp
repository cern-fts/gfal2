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

#include "gridftpmodule.h"
#include "gridftp_namespace.h"
#include <exceptions/cpp_to_gerror.hpp>

static const GQuark GFAL_GRIDFTP_SCOPE_UNLINK = g_quark_from_static_string("GridFTPModule::unlink");


void gridftp_unlink_internal(gfal2_context_t context, GridFTPSessionHandler* sess, const char * path)
{
    gfal2_log(G_LOG_LEVEL_DEBUG, " -> [GridFTPModule::unlink] ");
    GridFTPRequestState req(sess); // get connection session

    globus_result_t res = globus_ftp_client_delete(req.handler->get_ftp_client_handle(),
            path, req.handler->get_ftp_client_operationattr(),
            globus_ftp_client_done_callback, &req);
    gfal_globus_check_result(GFAL_GRIDFTP_SCOPE_UNLINK, res);
    req.wait(GFAL_GRIDFTP_SCOPE_UNLINK);
    gfal2_log(G_LOG_LEVEL_DEBUG, " <- [GridFTPModule::unlink] ");
}


void GridFTPModule::unlink(const char* path)
{
    if (path == NULL) {
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_UNLINK, EINVAL,
                "Invalid arguments path");
    }

    GridFTPSessionHandler handler(_handle_factory, path);
    gridftp_unlink_internal(_handle_factory->get_gfal2_context(), &handler, path);

}


extern "C" int gfal_gridftp_unlinkG(plugin_handle handle, const char* url,
        GError** err)
{
    g_return_val_err_if_fail(handle != NULL && url != NULL, -1, err,
            "[gfal_gridftp_unlinkG][gridftp] Invalid parameters");

    GError * tmp_err = NULL;
    int ret = -1;
    gfal2_log(G_LOG_LEVEL_DEBUG, "  -> [gfal_gridftp_unlinkG]");
    CPP_GERROR_TRY
                (static_cast<GridFTPModule*>(handle))->unlink(url);
                ret = 0;
            CPP_GERROR_CATCH(&tmp_err);
    gfal2_log(G_LOG_LEVEL_DEBUG, "  [gfal_gridftp_unlinkG] <-");
    G_RETURN_ERR(ret, tmp_err, err);
}
