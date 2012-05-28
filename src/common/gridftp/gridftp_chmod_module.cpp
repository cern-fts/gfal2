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


#include "gridftp_chmod_module.h"

const Glib::Quark scope_chmod("GridftpModule::chmod");


void GridftpModule::chmod(const char* path, mode_t mode)
{
	if(path== NULL )
		throw Glib::Error(scope_chmod, EINVAL, "Invalid arguments path or mode ");
	gfal_print_verbose(GFAL_VERBOSE_TRACE," -> [GridftpModule::chmod] ");
	
	std::auto_ptr<GridFTP_Request_state> req( new GridFTP_Request_state(_handle_factory->gfal_globus_ftp_take_handle())); // get connexion session
	
	globus_result_t res = globus_ftp_client_chmod(
				req->sess->get_ftp_handle(),
				path,
				mode,
				NULL,
				globus_basic_client_callback,
    			req.get());
	gfal_globus_check_result(scope_chmod, res);
	// wait for answer
	gridftp_wait_for_callback(scope_chmod, req.get());		

	gfal_print_verbose(GFAL_VERBOSE_TRACE," <- [GridftpModule::chmod] ");	
	
}


extern "C" int gfal_gridftp_chmodG(plugin_handle handle, const char* path , mode_t mode, GError** err){
	g_return_val_err_if_fail( handle != NULL && path != NULL
			, -1, err, "[gfal_gridftp_chmodG][gridftp] einval params");

	GError * tmp_err=NULL;
	int ret = -1;
	gfal_print_verbose(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_chmod]");
	CPP_GERROR_TRY
		(static_cast<GridftpModule*>(handle))->chmod(path, mode);
		ret = 0;
	CPP_GERROR_CATCH(&tmp_err);
	gfal_print_verbose(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_chmod]<-");
	G_RETURN_ERR(ret, tmp_err, err);	
}


