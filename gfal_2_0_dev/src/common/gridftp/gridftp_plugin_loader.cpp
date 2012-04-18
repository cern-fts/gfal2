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




#include <glib.h>


#include <common/gfal_constants.h>
#include <common/gfal_types.h>
#include <common/gfal_common_errverbose.h>
#include <libcpp/gerror_to_cpp.h>
#include <libcpp/cpp_to_gerror.hpp>

#include "gridftpmodule.h"

#include "gridftp_plugin_main.h"
#include "gridftp_plugin_loader.h"



extern "C"{




plugin_handle plugin_load(gfal_handle handle, GError ** err){
	GError * tmp_err=NULL;
	plugin_handle h = NULL;
	CPP_GERROR_TRY
		 h = static_cast<plugin_handle>(new GridFTPFileCopyModule( new GridftpModule( new GridFTPWrapper(handle) )));
	
	CPP_GERROR_CATCH(&tmp_err);
	
	G_RETURN_ERR(h, tmp_err, err);
	
}


void plugin_unload(plugin_handle handle){
	if(handle){
		try{
			delete (static_cast<GridFTPFileCopyModule*>(handle));
		}catch(...){
			gfal_print_verbose(GFAL_VERBOSE_NORMAL, " bug found plugin gridFTP throws error while loading");
		}
	}
}

/**
 * simply return the name of the plugin
 */
const char * plugin_name(){
	return "plugin_gridftp";
	
}

/**
 * Map function for the gridftp interface
 * this function provide the generic PLUGIN interface for the gridftp plugin.
 * lfc_initG do : liblfc shared library load, sym resolve, endpoint check, and plugin function map.
 * 
 * */
gfal_plugin_interface gfal_plugin_init(gfal_handle handle, GError** err){
	GError* tmp_err=NULL;
	
	gfal_plugin_interface ret;
	memset(&ret, 0, sizeof(gfal_plugin_interface));
	plugin_handle r = plugin_load(handle, &tmp_err);

	ret.handle = r;
	ret.check_plugin_url = &plugin_url_check_with_gerror;
	ret.plugin_delete = &plugin_unload;
	ret.getName = &plugin_name;
	G_RETURN_ERR(ret, tmp_err, err);
}




}
