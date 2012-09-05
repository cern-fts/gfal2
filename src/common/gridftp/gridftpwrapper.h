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

#include <ctime>
#include <externals/utils/time_utils.h>
#include "gridftpmodule.h"


#define GRIDFTP_CONFIG_GROUP "GRIDFTP PLUGIN"

enum Gridftp_request_status{
	GRIDFTP_REQUEST_NOT_LAUNCHED,
	GRIDFTP_REQUEST_RUNNING,
	GRIDFTP_REQUEST_FINISHED,
};

enum GridFtp_request_type{
    GRIDFTP_REQUEST_GASS,
    GRIDFTP_REQUEST_FTP
};

struct GridFTP_Request_state{
    GridFTP_Request_state(GridFTP_session * s, bool own_session=true,  GridFtp_request_type request_type = GRIDFTP_REQUEST_FTP);
	virtual ~GridFTP_Request_state();

    // ftp session
	std::auto_ptr<GridFTP_session> sess;  
    // request type
    GridFtp_request_type request_type;
    // request status
    Gridftp_request_status req_status;
    int errcode;
    std::string	error;
    // params
    struct timespec end_time; // timeout trigger -> 0 if not enabled, usage of CLOCK_MONOTONIC is required.
    // enable/disable destroy when out of scope
    bool own_session;

    //
    inline void init_timeout(struct timespec * time_offset){
        if( timespec_isset(&end_time)){
            timespec_clear(&end_time);
        }else{
            clock_gettime(CLOCK_MONOTONIC, &end_time);
            timespec_add(&end_time, time_offset, &end_time);
        }
    }
};

struct GridFTP_stream_state : public GridFTP_Request_state{
	GridFTP_stream_state(GridFTP_session * s) : GridFTP_Request_state(s)	{
		offset =0;
		eof = false;
	}
	off_t offset; // file offset in the stream
	bool eof;     // end of file reached
    Gridftp_request_status stream_status;

	bool finished(){
        return (stream_status==GRIDFTP_REQUEST_FINISHED);
	}
	Glib::Mutex lock;

};

				

class GridFTPFactory : public GridFTPFactoryInterface
{
	public:
		GridFTPFactory(gfal_handle handle );
		virtual ~GridFTPFactory();

		/**
		 * provide session handle for most of the operations
		 * 
		 * */
		virtual  GridFTP_session* gfal_globus_ftp_take_handle(const std::string & hostname);
		/**
		 * destruct an existing session handle, close the connection and desallocate memory
		 * 
		 * */		
		virtual void gfal_globus_ftp_release_handle(GridFTP_session* h) ;
		
		

	private:
		gfal_handle _handle;
		virtual gfal_handle get_handle();
        // session re-use management
		bool session_reuse;
		unsigned int size_cache;
		// session cache
		std::multimap<std::string, GridFTP_session*> sess_cache;
		Glib::Mutex mux_cache;
		void recycle_session(GridFTP_session* sess);
		void clear_cache();
		GridFTP_session* get_recycled_handle(const std::string & hostname);
		GridFTP_session* get_new_handle(const std::string & hostname);
		
		
		
		void gfal_globus_ftp_release_handle_internal(GridFTP_session* sess);

		void configure_gridftp_handle_attr(globus_ftp_client_handleattr_t * attrs);
	
	
		
	friend struct GridFTP_session_implem;
};


void globus_gass_cancel_sync(const Glib::Quark & scope, GridFTP_Request_state* req);

void globus_basic_client_callback (void * user_arg, 
				globus_ftp_client_handle_t *		handle,
				globus_object_t *				error);

void globus_gass_basic_client_callback(
        void * callback_arg,
        globus_gass_copy_handle_t * handle,
        globus_object_t * error);

// wait for the end of the gass asynchronous call
//throw Glib::Error in case of failure
void gridftp_gass_wait_for_callback(const Glib::Quark & scope, GridFTP_Request_state* state);	// throw GLib::Error
	
				
// wait for the end of the globus asynchronous call
//throw Glib::Error in case of failure				
void gridftp_wait_for_callback(const Glib::Quark & scope, GridFTP_Request_state* state);	// throw GLib::Error	

void gridftp_callback_err_report(const Glib::Quark & scope, GridFTP_Request_state* state);

void gridftp_callback_err_report(const Glib::Quark & scope, GridFTP_Request_state* state);


// polling function for gass ops
void gridftp_gass_poll_callback(const Glib::Quark & scope, GridFTP_Request_state* state);

// polling function for gridftp ops
void gridftp_poll_callback(const Glib::Quark & scope, GridFTP_Request_state* state);

// do atomic read operation from globus async call
ssize_t gridftp_read_stream(const Glib::Quark & scope, GridFTP_stream_state* stream,
				void* buffer, size_t s_read);	
				
// do atomic write operation from globus async call
ssize_t gridftp_write_stream(const Glib::Quark & scope, GridFTP_stream_state* stream,
				const void* buffer, size_t s_write, bool eof);


// return 0 if no error, or return errno and set the error string properly
// error allocation is dynamic
int gfal_globus_error_convert(globus_object_t * error, char ** str_error);

// throw Glib::Error if error associated with this result
void gfal_globus_check_result(const Glib::Quark & scope, gfal_globus_result_t res);

// throw Glib::Error if error is present
void gfal_globus_check_error(const Glib::Quark & scope,  globus_object_t *	error);

std::string gridftp_hostname_from_url(const char * url);

#endif /* GRIDFTPWRAPPER_H */ 
