/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
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


#include "gridftp_exist.h"
#include "gridftp_ifce_filecopy.h"
#include <transfer/gfal_transfer_types_internal.h>

const Glib::Quark scope_filecopy("GridFTP::Filecopy");

void gridftp_filecopy_delete_existing(GridFTP_session * sess, gfalt_params_t params, const char * url){
	const bool replace = gfalt_get_replace_existing_file(params,NULL);
	bool exist = gridftp_module_file_exist(sess, url);	
	if(exist){

		if(replace){
			gfal_log(GFAL_VERBOSE_TRACE, " File %s already exist, delete it for override ....",url); 
			gridftp_unlink_internal(sess, url, false);
			gfal_log(GFAL_VERBOSE_TRACE, " File %s deleted with success, proceed to copy ....",url); 									
		}else{
			char err_buff[GFAL_ERRMSG_LEN];
			snprintf(err_buff, GFAL_ERRMSG_LEN, " Destination already exist %s, Cancel", url);
			throw Gfal::CoreException(scope_filecopy, err_buff, EEXIST);
		}
	}
	
}


int GridftpModule::filecopy(gfalt_params_t params, const char* src, const char* dst){
	using namespace Gfal::Transfer;
	GError * tmp_err=NULL;

	const unsigned long timeout = gfalt_get_timeout(params, &tmp_err);
	Gfal::gerror_to_cpp(&tmp_err);
	std::auto_ptr<GridFTP_session> sess(_handle_factory->gfal_globus_ftp_take_handle(gridftp_hostname_from_url(src)));
	
	gridftp_filecopy_delete_existing(sess.get(), params, dst);

	gfal_log(GFAL_VERBOSE_TRACE, "   [GridFTPFileCopyModule::filecopy] start gridftp transfer %s -> %s", src, dst);
	gfal_globus_result_t res = globus_gass_copy_url_to_url 	(sess->get_gass_handle(),
		(char*)src,
		GLOBUS_NULL,
		(char*)dst,
		GLOBUS_NULL 
		);
	gfal_globus_check_result("GridFTPFileCopyModule::filecopy", res);
	return 0;			
}

extern "C"{

/**
 * initiaize a file copy from the given source to the given dest with the parameters params
 */
int plugin_filecopy(plugin_handle handle, gfal2_context_t context, gfalt_params_t params, const char* src, const char* dst, GError ** err){
	g_return_val_err_if_fail( handle != NULL && src != NULL
			&& dst != NULL , -1, err, "[plugin_filecopy][gridftp] einval params");

	GError * tmp_err=NULL;
	int ret = -1;
	gfal_log(GFAL_VERBOSE_TRACE, "  -> [gridftp_plugin_filecopy]");
	CPP_GERROR_TRY
		( static_cast<GridftpModule*>(handle))->filecopy(params, src, dst);
		ret = 0;
	CPP_GERROR_CATCH(&tmp_err);
	gfal_log(GFAL_VERBOSE_TRACE, "  [gridftp_plugin_filecopy]<-");
	G_RETURN_ERR(ret, tmp_err, err);
}


}
