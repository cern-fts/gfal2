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
#include <glib.h>
#include <cancel/gfal_cancel.h>
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
 protected:
   Glib::Mutex internal_lock;
   
   int         errcode;
   std::string error;
   
   Gridftp_request_status req_status;
   
   inline void init_timeout(struct timespec * time_offset){
       if( time_offset  && timespec_isset(time_offset)){
           end_time.assign_current_time();
           end_time.add_seconds(time_offset->tv_sec);
           end_time.add_microseconds(time_offset->tv_nsec/1000);
       }else{
         end_time = Glib::TimeVal(0,0);
       }
   }

   inline void set_end_time(struct timespec * my_end_time){
       if(my_end_time && timespec_isset(my_end_time)){
           end_time = Glib::TimeVal(my_end_time->tv_sec, my_end_time->tv_nsec/1000);
       }else{
         end_time = Glib::TimeVal(0,0);
       }
   }

 public:
    GridFTP_Request_state(GridFTP_session * s, bool own_session=true,  GridFtp_request_type request_type = GRIDFTP_REQUEST_FTP);
	virtual ~GridFTP_Request_state();

    // cancel token
    gfal_cancel_token_t cancel_token;
    // ftp session
	std::auto_ptr<GridFTP_session> sess;  
    // request type
    GridFtp_request_type request_type;   
    // params
    Glib::TimeVal end_time; // timeout trigger -> 0 if not enabled, usage of CLOCK_MONOTONIC is required.
    // enable/disable destroy when out of scope
    bool own_session;
    // bool canceling
    bool canceling;
    // mutex for state checking
    Glib::RWLock mux_req_state;
    Glib::Mutex mux_callback_lock;
    Glib::Cond signal_callback_main;

    inline void start(){
        this->req_status = GRIDFTP_REQUEST_RUNNING;
    }
    
    inline int get_error_code(void) {
      Glib::Mutex::Lock locker(internal_lock);
      return errcode;
    }
    
    inline void set_error_code(int code) {
      Glib::Mutex::Lock locker(internal_lock);
      errcode = code;
    }
    
    inline std::string get_error(void) {
      Glib::Mutex::Lock locker(internal_lock);
      return error;
    }
    
    inline void set_error(const std::string & errstr) {
      Glib::Mutex::Lock locker(internal_lock);
      error = errstr;
    }
    
    inline Gridftp_request_status get_req_status(void) {
      Glib::Mutex::Lock locker(internal_lock);
      return  req_status;
    }
    
    inline void set_req_status(const Gridftp_request_status & st) {
      Glib::Mutex::Lock locker(internal_lock);
      req_status = st;
    }

    void poll_callback(const Glib::Quark & scope);
    void wait_callback(const Glib::Quark & scope, time_t timeout = 300);
    void err_report(const Glib::Quark &scope);

    void cancel_operation(const Glib::Quark &scope, const std::string & msg = "");

    int cancel_operation_async(const Glib::Quark &scope, const std::string & msg = "");
};

struct GridFTP_stream_state : public GridFTP_Request_state{
 protected:
    off_t offset; // file offset in the stream
	bool eof;     // end of file reached
    Gridftp_request_status stream_status;
    Glib::Mutex mux_stream_callback;
    Glib::Cond  cond_stream_callback;
    
 public:
   
   // ownership lock
   Glib::Mutex lock;
   
	GridFTP_stream_state(GridFTP_session * s) : GridFTP_Request_state(s)	{
		offset =0;
		eof = false;
        stream_status = GRIDFTP_REQUEST_NOT_LAUNCHED;
	}

    virtual ~GridFTP_stream_state();
	
	bool finished(){
        Glib::Mutex::Lock locker(internal_lock);
        return (stream_status==GRIDFTP_REQUEST_FINISHED);
	}
    
    inline bool is_eof(void) {
        Glib::Mutex::Lock locker(internal_lock);
        return eof;
    }
    
    inline void set_eof(bool end) {
        Glib::Mutex::Lock locker(internal_lock);
        eof = end;
    }
    
    inline off_t get_offset(void) {
        Glib::Mutex::Lock locker(internal_lock);
        return offset;
    }
       
    void set_offset(off_t off) {
        Glib::Mutex::Lock locker(internal_lock);
        offset = off;
    }
    
    void increase_offset(off_t diff) {
        Glib::Mutex::Lock locker(internal_lock);
        offset += diff;
    }
    
    Gridftp_request_status get_stream_status(void) {
        Glib::Mutex::Lock locker(internal_lock);
        return stream_status;
    }
    
    void set_stream_status(const Gridftp_request_status & st) {
        Glib::Mutex::Lock locker(internal_lock);
        stream_status = st;
    }

    void poll_callback_stream(const Glib::Quark & scope);
    void wait_callback_stream(const Glib::Quark & scope);

    friend void gfal_stream_callback_prototype(void *user_arg, globus_ftp_client_handle_t *handle, globus_object_t *error, globus_byte_t *buffer,
                                                globus_size_t length, globus_off_t offset, globus_bool_t eof, const char* err_msg_offset);
};

class GridFTPOperationCanceler{
public:
    GridFTPOperationCanceler(gfal2_context_t context, GridFTP_Request_state* state);
    ~GridFTPOperationCanceler();

private:
    GridFTP_Request_state* _state;
    gfal_cancel_token_t _cancel_token;
    gfal2_context_t _context;

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

void globus_basic_client_callback (void * user_arg, 
				globus_ftp_client_handle_t *		handle,
				globus_object_t *				error);


void globus_gass_basic_client_callback(
        void * callback_arg,
        globus_gass_copy_handle_t * handle,
        globus_object_t * error);

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
