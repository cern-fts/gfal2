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
#include <exceptions/cpp_to_gerror.hpp>


static Glib::Quark GFAL_GRIDFTP_SCOPE_MKDIR("GridFTPModule::mkdir");


void GridFTPModule::mkdir(const char* path, mode_t mode)
{
    if (path == NULL)
        throw Glib::Error(GFAL_GRIDFTP_SCOPE_MKDIR, EINVAL,
                "Invalid arguments path or mode ");
    gfal_log(GFAL_VERBOSE_TRACE, " -> [GridFTPModule::mkdir] ");

    GridFTPSessionHandler handler(_handle_factory, path);
    GridFTPRequestState req(&handler);

    globus_result_t res = globus_ftp_client_mkdir(req.handler->get_ftp_client_handle(),
            path, req.handler->get_ftp_client_operationattr(), globus_ftp_client_done_callback,
            &req);
    gfal_globus_check_result("GridFTPModule::mkdir", res);
    // wait for answer
    req.wait(GFAL_GRIDFTP_SCOPE_MKDIR);

    gfal_log(GFAL_VERBOSE_TRACE, " <- [GridFTPModule::mkdir] ");

}


extern "C" int gfal_gridftp_mkdirG(plugin_handle handle, const char* path,
        mode_t mode, gboolean pflag, GError** err)
{
    g_return_val_err_if_fail(handle != NULL && path != NULL, -1, err,
            "[gfal_gridftp_mkdirG][gridftp] Invalid parameters");

    GError * tmp_err = NULL;
    int ret = -1;
    gfal_log(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_mkdirG]");
    CPP_GERROR_TRY
                (static_cast<GridFTPModule*>(handle))->mkdir(path, mode);
                ret = 0;
            CPP_GERROR_CATCH(&tmp_err);
    gfal_log(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_mkdirG]<-");
    G_RETURN_ERR(ret, tmp_err, err);
}
