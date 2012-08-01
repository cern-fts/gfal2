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
#include <globus_gsi_proxy.h>
#include <globus_ftp_client_restart_marker_plugin.h>
#include <globus_ftp_client_restart_plugin.h>
#include <globus_ftp_client_debug_plugin.h>

#include <common/gfal_constants.h>
#include <common/gfal_types.h>


typedef globus_gass_copy_handle_t gfal_globus_copy_handle_t;
typedef globus_gass_copy_attr_t gfal_globus_copy_attr_t;
typedef globus_result_t gfal_globus_result_t;


struct Gass_attr_handler{
    virtual ~Gass_attr_handler(){};
    globus_gass_copy_attr_t attr_gass;
    globus_ftp_client_operationattr_t operation_attr_ftp_for_gass;
};


struct GridFTP_session{
	
	virtual ~GridFTP_session(){}
	

	virtual globus_ftp_client_handle_t* get_ftp_handle()=0;
	virtual globus_ftp_client_operationattr_t* get_op_attr_ftp()=0;
	virtual globus_gass_copy_handle_t* get_gass_handle()=0;
    virtual globus_gass_copy_handleattr_t* get_gass_handle_attr()=0;

    virtual Gass_attr_handler* generate_gass_copy_attr()=0;

    virtual void set_nb_stream(const unsigned int nbstream)=0;

};


class GridFTPFactoryInterface
{
	public:
		GridFTPFactoryInterface();
		virtual ~GridFTPFactoryInterface();

		virtual gfal_handle get_handle()=0;	
		
		
		virtual  GridFTP_session* gfal_globus_ftp_take_handle(const std::string & hostname)=0;
		virtual void gfal_globus_ftp_release_handle(GridFTP_session* h) =0;
						
	protected:

};

#endif /* GRIDFTPINTERFACE_H */ 
