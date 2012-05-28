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



#include "gridftpmodule.h"

#include "gridftp_plugin_main.h"
#include "gridftp_stat_module.h"




extern "C"{
	
plugin_handle plugin_load(gfal_handle handle, GError ** err){
	GError * tmp_err=NULL;
	plugin_handle h = NULL;
	CPP_GERROR_TRY
		gfal_log(GFAL_VERBOSE_TRACE, " -> [gridftp_plugin] try to load ..");	
		 h = static_cast<plugin_handle>(
					new GridftpModule( new GridFTPFactory(handle) )
			);
		gfal_log(GFAL_VERBOSE_TRACE, " -> [gridftp_plugin] loaded ..");	
	CPP_GERROR_CATCH(&tmp_err);
	
	G_RETURN_ERR(h, tmp_err, err);
	
}


void plugin_unload(plugin_handle handle){
	if(handle){
		try{
			delete (static_cast<GridftpModule*>(handle));
		}catch(...){
			gfal_log(GFAL_VERBOSE_NORMAL, " bug found plugin gridFTP throws error while loading");
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

	ret.plugin_data = r;
	ret.check_plugin_url = &plugin_url_check_with_gerror;
	ret.plugin_delete = &plugin_unload;
	ret.getName = &plugin_name;
	ret.accessG = & gfal_gridftp_accessG;
	ret.statG = & gfal_gridftp_statG;
	ret.lstatG = &gfal_gridftp_statG;
	ret.unlinkG = &gfal_gridftp_unlinkG;
	ret.mkdirpG = &gfal_gridftp_mkdirG;
	ret.chmodG = &gfal_gridftp_chmodG;
	ret.rmdirG = &gfal_gridftp_rmdirG;
	ret.opendirG = &gfal_gridftp_opendirG;
	ret.readdirG = &gfal_gridftp_readdirG;
	ret.closedirG = &gfal_gridftp_closedirG;
	ret.openG = &gfal_gridftp_openG;
	ret.closeG = &gfal_gridftp_closeG;
	ret.readG = &gfal_gridftp_readG;
	ret.writeG = &gfal_gridftp_writeG;
	ret.lseekG = &gfal_gridftp_lseekG;
	
	G_RETURN_ERR(ret, tmp_err, err);
}

/*  --> desactivated temporary -> responsible of segfault with globus 5.2 thread problem
__attribute__((destructor)) 
static void gridftp_destructor(){ // try to unload globus
	globus_module_deactivate_all();
}*/

}
