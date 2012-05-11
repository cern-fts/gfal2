#ifndef GRIDFTPWRAPPER_H
#define GRIDFTPWRAPPER_H
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

#include "gridftpmodule.h"

struct GridFTP_Request_state{
	GridFTP_Request_state(){
		status=1;
	}
	virtual ~GridFTP_Request_state(){};
	
	int status;
    int errcode;
    std::string	error;	
};

struct GridFTP_stream_state : public GridFTP_Request_state{
	GridFTP_stream_state(GridFTP_session * s) : sess(s)	{
		offset =0;
		eof = false;
	}
	off_t offset; // file offset in the stream
	bool eof;     // end of file reached
	std::auto_ptr<GridFTP_session> sess;    
	Glib::Mutex lock;

};

void globus_basic_client_callback (void * user_arg, 
				globus_ftp_client_handle_t *		handle,
				globus_object_t *				error);
	
				
// wait for the end of the globus asynchronous call
//throw Glib::Error in case of failure				
void gridftp_wait_for_callback(const Glib::Quark & scope, GridFTP_Request_state* state);	// throw GLib::Error	

void gridftp_callback_err_report(const Glib::Quark & scope, GridFTP_Request_state* state);

void gridftp_callback_err_report(const Glib::Quark & scope, GridFTP_Request_state* state);

void gridftp_poll_callback(const Glib::Quark & scope, GridFTP_Request_state* state);

// do atomic read operation from globus async call
ssize_t gridftp_read_stream(const Glib::Quark & scope, GridFTP_stream_state* stream,
				void* buffer, size_t s_read);	

class GridFTPFactory : public GridFTPFactoryInterface
{
	public:
		GridFTPFactory(gfal_handle handle );
		virtual ~GridFTPFactory();

		/**
		 * take ownership and allocate a globus gass handle
		 * */
		virtual gfal_globus_copy_handle_t take_globus_gass_handle();
		
		 /**
		 * release ownership and free memory of  a globus gass handle
		 * */	
		virtual void release_globus_gass_handle(gfal_globus_copy_handle_t*);
		/**
		 * provide session handle for most of the operations
		 * 
		 * */
		virtual  GridFTP_session* gfal_globus_ftp_take_handle();
		/**
		 * destruct an existing session handle, close the connection and desallocate memory
		 * 
		 * */		
		virtual void gfal_globus_ftp_release_handle(GridFTP_session* h) ;
		
		virtual gfal_globus_copy_attr_t* take_globus_gass_attr();
		virtual void release_globus_gass_attr(gfal_globus_copy_attr_t * h);	
		

	protected:
		gfal_handle _handle;
		virtual gfal_handle get_handle();
		
		void gfal_globus_ftp_release_handle_internal(GridFTP_session* sess);

		
	friend struct GridFTP_session_implem;
};

/**
 * throw Glib::Error if error associated with this result
 * 
 **/
void gfal_globus_check_result(const Glib::Quark & scope, gfal_globus_result_t res);

/**
 * throw Glib::Error if error is present
 * */
void gfal_globus_check_error(const Glib::Quark & scope,  globus_object_t *	error);

#endif /* GRIDFTPWRAPPER_H */ 
