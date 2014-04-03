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

#include "gridftpmodule.h"
#include "gridftp_namespace.h"

static Glib::Quark gfal_gridftp_scope_unlink(){
    return Glib::Quark("GridftpModule::unlink");
}

void gridftp_unlink_internal(gfal2_context_t context, GridFTP_session* sess, const char * path, bool own_session){

	gfal_log(GFAL_VERBOSE_TRACE," -> [GridftpModule::unlink] ");	
	std::auto_ptr<GridFTP_Request_state> req( new GridFTP_Request_state(sess, own_session)); // get connexion session
    GridFTPOperationCanceler canceler(context, req.get());

    req->start();
	globus_result_t res = globus_ftp_client_delete(
				req->sess->get_ftp_handle(),
				path,
				req->sess->get_op_attr_ftp(),
				globus_basic_client_callback,
				req.get());
    gfal_globus_check_result(gfal_gridftp_scope_unlink(), res);
	// wait for answer
    req->wait_callback(gfal_gridftp_scope_unlink());
	gfal_log(GFAL_VERBOSE_TRACE," <- [GridftpModule::unlink] ");		
}


void GridftpModule::unlink(const char* path)
{
	if(path== NULL )
        throw Glib::Error(gfal_gridftp_scope_unlink(), EINVAL, "Invalid arguments path");
	
    gridftp_unlink_internal(_handle_factory->get_handle(), this->_handle_factory->gfal_globus_ftp_take_handle(gridftp_hostname_from_url(path)),
							path,
							true);
	
}


extern "C" int gfal_gridftp_unlinkG(plugin_handle handle, const char* url , GError** err){
	g_return_val_err_if_fail( handle != NULL && url != NULL
			, -1, err, "[gfal_gridftp_unlinkG][gridftp] einval params");

	GError * tmp_err=NULL;
	int ret = -1;
	gfal_log(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_unlinkG]");
	CPP_GERROR_TRY
		(static_cast<GridftpModule*>(handle))->unlink(url);
		ret = 0;
	CPP_GERROR_CATCH(&tmp_err);
	gfal_log(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_unlinkG] <-");
	G_RETURN_ERR(ret, tmp_err, err);	
}


