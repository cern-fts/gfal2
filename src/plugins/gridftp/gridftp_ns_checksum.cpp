/* 
* Copyright @ Members of the EMI Collaboration, 2010.
* See www.eu-emi.eu for details on the copyright holders.
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
#include <config/gfal_config.h>
#include <exceptions/cpp_to_gerror.hpp>

static Glib::Quark gfal_gridftp_scope_checksum(){
    return Glib::Quark("Gridftp_checksum_module::checksum");
}



const char * gridftp_checksum_calc_timeout= "CHECKSUM_CALC_TIMEOUT";

extern "C" int gfal_gridftp_checksumG(plugin_handle handle, const char* url, const char* check_type,
                       char * checksum_buffer, size_t buffer_length,
                       off_t start_offset, size_t data_length,
                       GError ** err){
    g_return_val_err_if_fail( handle != NULL && url != NULL
            , -1, err, "[gfal_gridftp_checksumG][gridftp] einval params");

    GError * tmp_err=NULL;
    int ret = -1;
    gfal_log(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_checksumG]");
    CPP_GERROR_TRY
        (static_cast<GridftpModule*>(handle))->checksum(url, check_type,
                                                        checksum_buffer, buffer_length,
                                                        start_offset,  data_length);
        ret = 0;
    CPP_GERROR_CATCH(&tmp_err);
    gfal_log(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_checksumG] <-");
    G_RETURN_ERR(ret, tmp_err, err);
}



void GridftpModule::checksum(const char* url, const char* check_type,
                       char * checksum_buffer, size_t buffer_length,
                       off_t start_offset, size_t data_length)
{
    gfal_log(GFAL_VERBOSE_TRACE," -> [GridftpModule::checksum] ");
    gfal_log(GFAL_VERBOSE_DEBUG," Checksum calculation %s for url %s", check_type, url);
    std::auto_ptr<GridFTP_Request_state> req( new GridFTP_Request_state(_handle_factory->gfal_globus_ftp_take_handle(gridftp_hostname_from_url(url)),
                                                                        GRIDFTP_REQUEST_FTP));

    if(buffer_length < 16)
        throw Gfal::CoreException(gfal_gridftp_scope_checksum(),"buffer length for checksum calculation is not enought",ENOBUFS);

    req->start();
    GridFTPOperationCanceler canceler(_handle_factory->get_handle(), req.get());
    globus_result_t res = globus_ftp_client_cksm(
                req->sess->get_ftp_handle(),
                url,
                req->sess->get_op_attr_ftp(),
                checksum_buffer,
                start_offset,
                ((data_length)?(data_length):(-1)),
                check_type,
                globus_basic_client_callback,
                req.get());
    gfal_globus_check_result(gfal_gridftp_scope_checksum(), res);
    // wait for answer with a timeout
    const time_t timeout = gfal2_get_opt_integer_with_default(_handle_factory->get_handle(),
                                                              GRIDFTP_CONFIG_GROUP,
                                                              gridftp_checksum_calc_timeout, 1800);
    req->wait_callback(gfal_gridftp_scope_checksum(), timeout);
    gfal_log(GFAL_VERBOSE_TRACE," <- [GridftpModule::checksum] ");
}


