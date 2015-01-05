/*
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gridftp_namespace.h"
#include "gridftpwrapper.h"
#include <exceptions/cpp_to_gerror.hpp>

static const GQuark GFAL_GRIDFTP_SCOPE_RENAME = g_quark_from_static_string("GridFTPModule::rmdir");


void GridFTPModule::rename(const char* src, const char* dst)
{
    if (src == NULL || dst == NULL) {
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_RENAME, EINVAL,
                "Invalid source and/or destination");
    }

    gfal_log(GFAL_VERBOSE_TRACE, " -> [GridFTPModule::rename] ");

    GridFTPSessionHandler handler(_handle_factory, src);
    GridFTPRequestState req(&handler);

    globus_result_t res = globus_ftp_client_move(req.handler->get_ftp_client_handle(),
            src, dst, req.handler->get_ftp_client_operationattr(), globus_ftp_client_done_callback,
            &req);
    gfal_globus_check_result(GFAL_GRIDFTP_SCOPE_RENAME, res);
    // wait for answer
    req.wait(GFAL_GRIDFTP_SCOPE_RENAME);

    gfal_log(GFAL_VERBOSE_TRACE, " <- [GridFTPModule::rename] ");

}


int gfal_gridftp_renameG(plugin_handle handle, const char * oldurl,
        const char * newurl, GError** err)
{
    g_return_val_err_if_fail(handle != NULL && oldurl != NULL && newurl != NULL,
            -1, err, "[gfal_gridftp_rename][gridftp] Invalid parameters");

    GError * tmp_err = NULL;
    int ret = -1;
    gfal_log(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_rename]");
    CPP_GERROR_TRY
                (static_cast<GridFTPModule*>(handle))->rename(oldurl, newurl);
                ret = 0;
    CPP_GERROR_CATCH(&tmp_err);
    gfal_log(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_rename]<-");
    G_RETURN_ERR(ret, tmp_err, err);
}
