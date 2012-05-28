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


#include "gridftp_mkdir_module.h"


const Glib::Quark scope_mkdir("GridftpModule::mkdir");

void GridftpModule::mkdir(const char* path, mode_t mode)
{
	if(path== NULL )
		throw Glib::Error(scope_mkdir, EINVAL, "Invalid arguments path or mode ");
	gfal_print_verbose(GFAL_VERBOSE_TRACE," -> [GridftpModule::mkdir] ");
	
	std::auto_ptr<GridFTP_Request_state> req( new GridFTP_Request_state(_handle_factory->gfal_globus_ftp_take_handle(gridftp_hostname_from_url(path)))); // get connexion session
	
	
	globus_result_t res = globus_ftp_client_mkdir(
				req->sess->get_ftp_handle(),
				path,
				NULL,
				globus_basic_client_callback,
    			req.get());
	gfal_globus_check_result("GridftpModule::mkdir", res);
	// wait for answer
	gridftp_wait_for_callback(scope_mkdir, req.get());		

	gfal_print_verbose(GFAL_VERBOSE_TRACE," <- [GridftpModule::mkdir] ");	
	
}


extern "C" int gfal_gridftp_mkdirG(plugin_handle handle , const char* path , mode_t mode , gboolean pflag, GError** err){
	g_return_val_err_if_fail( handle != NULL && path != NULL
			, -1, err, "[gfal_gridftp_mkdirG][gridftp] einval params");

	GError * tmp_err=NULL;
	int ret = -1;
	gfal_print_verbose(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_mkdirG]");
	CPP_GERROR_TRY
		(static_cast<GridftpModule*>(handle))->mkdir(path, mode);
		ret = 0;
	CPP_GERROR_CATCH(&tmp_err);
	gfal_print_verbose(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_mkdirG]<-");
	G_RETURN_ERR(ret, tmp_err, err);	
}


