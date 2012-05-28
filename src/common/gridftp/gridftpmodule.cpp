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
#include "gridftpmodule.h"


//GOnce my_once = G_ONCE_INIT;

const Glib::Quark scope_globus_init("GridftpModule::init_globus");



// gfunction prototype : gonce 
static void* init_globus(gpointer data){
    globus_result_t result = GLOBUS_SUCCESS;
     if( (  result = globus_module_activate(GLOBUS_GASS_COPY_MODULE) ) != GLOBUS_SUCCESS)
    	throw Gfal::CoreException(scope_globus_init, "Error globus init, globus gass", result);
     if( ( result = globus_module_activate(GLOBUS_GSI_GSSAPI_MODULE) ) != GLOBUS_SUCCESS)
		throw Gfal::CoreException(scope_globus_init, "Error globus init, globus gssapi", result);
	 if( ( result = globus_module_activate(GLOBUS_FTP_CLIENT_MODULE)) != GLOBUS_SUCCESS)
		throw Gfal::CoreException(scope_globus_init, "Error globus init, globus ftp", result);
     if( (   result = globus_module_activate(GLOBUS_FTP_CLIENT_RESTART_PLUGIN_MODULE) ) != GLOBUS_SUCCESS)
    	throw Gfal::CoreException(scope_globus_init, "Error globus init, glopus ftp restart plugin", result);
     if( (   result = globus_module_activate(GLOBUS_FTP_CLIENT_RESTART_MARKER_PLUGIN_MODULE) ) != GLOBUS_SUCCESS)
    	throw Gfal::CoreException(scope_globus_init, "Error globus init, globus ftp restart marker", result);
	return NULL;
}

static void* deinit_globus(gpointer data){
    globus_result_t result = GLOBUS_SUCCESS;  
     if( (  result = globus_module_deactivate(GLOBUS_GASS_COPY_MODULE) ) != GLOBUS_SUCCESS)
    	throw Gfal::CoreException(scope_globus_init, "Error globus deinit, globus gass", result);
     if( ( result = globus_module_deactivate(GLOBUS_GSI_GSSAPI_MODULE) ) != GLOBUS_SUCCESS)
		throw Gfal::CoreException(scope_globus_init, "Error globus deinit, globus gssapi", result);
	 if( ( result = globus_module_deactivate(GLOBUS_FTP_CLIENT_MODULE)) != GLOBUS_SUCCESS)
		throw Gfal::CoreException(scope_globus_init, "Error globus deinit, globus ftp", result);
     if( (   result = globus_module_deactivate(GLOBUS_FTP_CLIENT_RESTART_PLUGIN_MODULE) ) != GLOBUS_SUCCESS)
    	throw Gfal::CoreException(scope_globus_init, "Error globus deinit, glopus ftp restart plugin", result);
     if( (   result = globus_module_deactivate(GLOBUS_FTP_CLIENT_RESTART_MARKER_PLUGIN_MODULE) ) != GLOBUS_SUCCESS)
    	throw Gfal::CoreException(scope_globus_init, "Error globus deinit, globus ftp restart marker", result);
	return NULL;
}

GridftpModule::GridftpModule(GridFTPFactoryInterface* factory) 
{
	init_globus(NULL);
	_handle_factory = factory;
}


GridftpModule::~GridftpModule()
{
	deinit_globus(NULL);	
}

void gridftp_module_file_exist(GridFTP_session* sess, const char * url){
	
	
}

