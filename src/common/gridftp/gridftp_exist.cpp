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

#include "gridftp_exist.h"

static Glib::Quark gfal_gridftp_scope_exist(){
    return Glib::Quark ("GridftpModule::file_exist");
}

bool gridftp_module_file_exist(gfal2_context_t context, GridFTP_session* sess, const char * url){
	
	gfal_log(GFAL_VERBOSE_TRACE,"   -> [gridftp_module_file_exist]");
	

	std::auto_ptr<GridFTP_Request_state> req(new GridFTP_Request_state(sess, false));
    GridFTPOperationCanceler canceler(context, req.get());

    req->start();
	globus_result_t res = globus_ftp_client_exists(
				req->sess->get_ftp_handle(),
				url,
                req->sess->get_op_attr_ftp(),
				globus_basic_client_callback,
				req.get());
    gfal_globus_check_result(gfal_gridftp_scope_exist(), res);
    req->poll_callback(gfal_gridftp_scope_exist());

	gfal_log(GFAL_VERBOSE_TRACE,"   <- [gridftp_module_file_exist]");	
	switch(req->get_error_code()){
		case 0:
			return true;
		case ENOENT:
			return false;
		default:
            req->err_report(gfal_gridftp_scope_exist());
		
	}
	return false;	
	
}
