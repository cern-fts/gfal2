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



#include "gridftp_ifce_filecopy.h"
#include <transfer/gfal_transfer_types_internal.h>


int GridftpModule::filecopy(gfalt_params_handle params, const char* src, const char* dst){
	using namespace Gfal::Transfer;
	GError * tmp_err=NULL;

	const unsigned long timeout = gfalt_get_timeout(params, &tmp_err);
	Gfal::gerror_to_cpp(&tmp_err);
	gfal_globus_copy_handle_t h = _handle_factory->take_globus_gass_handle();

	gfal_print_verbose(GFAL_VERBOSE_TRACE, "   [GridFTPFileCopyModule::filecopy] start gridftp transfer %s -> %s", src, dst);
	gfal_globus_result_t res = globus_gass_copy_url_to_url 	(&h,
		(char*)src,
		GLOBUS_NULL,
		(char*)dst,
		GLOBUS_NULL 
		);
	_handle_factory->release_globus_gass_handle(&h);
	gfal_globus_check_result("GridFTPFileCopyModule::filecopy", res);
	return 0;			
}

extern "C"{

/**
 * initiaize a file copy from the given source to the given dest with the parameters params
 */
int plugin_filecopy(plugin_handle handle, gfal_context_t context, gfalt_params_t params, const char* src, const char* dst, GError ** err){
	g_return_val_err_if_fail( handle != NULL && src != NULL
			&& dst != NULL , -1, err, "[plugin_filecopy][gridftp] einval params");

	GError * tmp_err=NULL;
	int ret = -1;
	gfal_print_verbose(GFAL_VERBOSE_TRACE, "  -> [gridftp_plugin_filecopy]");
	CPP_GERROR_TRY
		( static_cast<GridftpModule*>(handle))->filecopy(params, src, dst);
		ret = 0;
	CPP_GERROR_CATCH(&tmp_err);
	gfal_print_verbose(GFAL_VERBOSE_TRACE, "  [gridftp_plugin_filecopy]<-");
	G_RETURN_ERR(ret, tmp_err, err);
}


}
