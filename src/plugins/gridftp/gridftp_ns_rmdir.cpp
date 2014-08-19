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

static Glib::Quark gfal_gridftp_scope_rmdir(){
    return Glib::Quark("GridftpModule::rmdir");
}


void GridftpModule::rmdir(const char* path)
{
	if(path== NULL )
        throw Glib::Error(gfal_gridftp_scope_rmdir(), EINVAL, "Invalid arguments path");
	gfal_log(GFAL_VERBOSE_TRACE," -> [GridftpModule::rmdir] ");
	
	try{
		GridFTPRequestState req(_handle_factory->gfal_globus_ftp_take_handle(gridftp_hostname_from_url(path))); // get connexion session
		
        req.start();
		globus_result_t res = globus_ftp_client_rmdir(
					req.sess->get_ftp_handle(),
					path,
					req.sess->get_op_attr_ftp(),
					globus_basic_client_callback,
					&req);
        gfal_globus_check_result(gfal_gridftp_scope_rmdir(), res);
		// wait for answer
        req.wait_callback(gfal_gridftp_scope_rmdir());
	}catch(Glib::Error & e){
		if(e.code() == EEXIST) // false ENOTEMPTY errno, do conversion
			throw Glib::Error(e.domain(), ENOTEMPTY, e.what());
		throw e;
	}	

	gfal_log(GFAL_VERBOSE_TRACE," <- [GridftpModule::rmdir] ");	
	
}


extern "C" int gfal_gridftp_rmdirG(plugin_handle handle, const char* url , GError** err){
	g_return_val_err_if_fail( handle != NULL && url != NULL
			, -1, err, "[gfal_gridftp_rmdir][gridftp] einval params");

	GError * tmp_err=NULL;
	int ret = -1;
	gfal_log(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_rmdir]");
	CPP_GERROR_TRY
		(static_cast<GridftpModule*>(handle))->rmdir(url);
		ret = 0;
	CPP_GERROR_CATCH(&tmp_err);
	gfal_log(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_rmdir]<-");
	G_RETURN_ERR(ret, tmp_err, err);	
}


