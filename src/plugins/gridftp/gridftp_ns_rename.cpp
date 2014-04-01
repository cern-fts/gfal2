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

static const Glib::Quark gfal_gridftp_scope_rename("GridftpModule::rmdir");



void GridftpModule::rename(const char* src, const char* dst)
{
    if (src == NULL || dst == NULL)
        throw Glib::Error(gfal_gridftp_scope_rename, EINVAL, "Invalid source and/or destination");

    gfal_log(GFAL_VERBOSE_TRACE," -> [GridftpModule::rename] ");

    std::auto_ptr<GridFTP_Request_state> req(
            new GridFTP_Request_state(
                    _handle_factory->gfal_globus_ftp_take_handle(
                            gridftp_hostname_from_url(src)))); // get connexion session

    req->start();
    globus_result_t res = globus_ftp_client_move(
                req->sess->get_ftp_handle(),
                src, dst,
                req->sess->get_op_attr_ftp(),
                globus_basic_client_callback,
                req.get());
    gfal_globus_check_result(gfal_gridftp_scope_rename, res);
    // wait for answer
    req->wait_callback(gfal_gridftp_scope_rename);

    gfal_log(GFAL_VERBOSE_TRACE," <- [GridftpModule::rename] ");

}


int gfal_gridftp_renameG(plugin_handle handle, const char * oldurl,
                         const char * newurl, GError** err)
{
    g_return_val_err_if_fail(handle != NULL && oldurl != NULL && newurl != NULL,
                             -1, err, "[gfal_gridftp_rename][gridftp] einval params");

    GError * tmp_err=NULL;
    int ret = -1;
    gfal_log(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_rename]");
    CPP_GERROR_TRY
        (static_cast<GridftpModule*>(handle))->rename(oldurl, newurl);
        ret = 0;
    CPP_GERROR_CATCH(&tmp_err);
    gfal_log(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_rename]<-");
    G_RETURN_ERR(ret, tmp_err, err);
}
