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


#include "grid_ftp_ifce_include.h"
#include "gridftpmodule.h"
#include <glib.h>

GOnce my_once = G_ONCE_INIT;



static void* init_globus(gpointer data){
    globus_result_t result = GLOBUS_SUCCESS;
     if( (  result = globus_module_activate(GLOBUS_GASS_COPY_MODULE) ) != GLOBUS_SUCCESS)
    	throw Gfal::CoreException("GridftpModule::init_globus", "Error globus init, globus gass", result);
     if( ( result = globus_module_activate(GLOBUS_GSI_GSSAPI_MODULE) ) != GLOBUS_SUCCESS)
		throw Gfal::CoreException("GridftpModule::init_globus", "Error globus init, globus gssapi", result);

     if( (   result = globus_module_activate(GLOBUS_FTP_CLIENT_RESTART_PLUGIN_MODULE) ) != GLOBUS_SUCCESS)
    	throw Gfal::CoreException("GridftpModule::init_globus", "Error globus init, glopus ftp restart plugin", result);
     if( (   result = globus_module_activate(GLOBUS_FTP_CLIENT_RESTART_MARKER_PLUGIN_MODULE) ) != GLOBUS_SUCCESS)
    	throw Gfal::CoreException("GridftpModule::init_globus", "Error globus init, globus ftp restart marker", result);
	return NULL;
}

GridftpModule::GridftpModule(GridFTPInterface* wrap) : GridFTPDecorator(wrap)
{
	g_once(&my_once, init_globus, NULL);
}


GridftpModule::~GridftpModule()
{
	
}

