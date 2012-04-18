#pragma once
#ifndef GRIDFTPINTERFACE_H
#define GRIDFTPINTERFACE_H
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

#include <string>

#include <globus_gass_copy.h>
#include <globus_ftp_client.h>
#include <globus_ftp_client_restart_marker_plugin.h>
#include <globus_ftp_client_restart_plugin.h>
#include <globus_ftp_client_debug_plugin.h>

#include <common/gfal_constants.h>
#include <common/gfal_types.h>


typedef globus_gass_copy_handle_t gfal_globus_copy_handle_t;
typedef globus_result_t gfal_globus_result_t;

class GridFTPInterface
{
	public:
		GridFTPInterface();
		virtual ~GridFTPInterface();

		virtual gfal_handle get_handle()=0;	
		
		virtual gfal_globus_copy_handle_t take_globus_handle() = 0;
		virtual void release_globus_handle(gfal_globus_copy_handle_t*) = 0;
		virtual void globus_check_result(const std::string & nmspace, gfal_globus_result_t res) = 0;
		
	protected:

};

#endif /* GRIDFTPINTERFACE_H */ 
